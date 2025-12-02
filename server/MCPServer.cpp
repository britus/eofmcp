/**
 * @file MCPServer.cpp
 * @brief MCP X Server Implementation
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <IMCPPromptService.h>
#include <IMCPResourceService.h>
#include <IMCPToolService.h>
#include <IMCPTransport.h>
#include <MCPContext.h>
#include <MCPHandlerResolver.h>
#include <MCPHttpTransportAdapter.h>
#include <MCPInvokeHelper.h>
#include <MCPLog.h>
#include <MCPPrompt.h>
#include <MCPPromptService.h>
#include <MCPPromptsConfig.h>
#include <MCPResource.h>
#include <MCPResourceService.h>
#include <MCPResourceWrapper.h>
#include <MCPResourcesConfig.h>
#include <MCPServer.h>
#include <MCPServerConfig.h>
#include <MCPServerHandler.h>
#include <MCPServerMessage.h>
#include <MCPSessionService.h>
#include <MCPTool.h>
#include <MCPToolService.h>
#include <MCPToolsConfig.h>
#include <impl/MCPHttpReplyMessage.h>
#include <QDir>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMetaObject>
#include <QStandardPaths>
#include <QThread>

MCPServer::MCPServer(QObject *pParent)
    : IMCPServer(pParent)
    , m_pTransport(new MCPHttpTransportAdapter(this))
    , m_pSessionService(new MCPSessionService(this))
    , m_pToolService(new MCPToolService(this))
    , m_pResourceService(new MCPResourceService(this))
    , m_pPromptService(new MCPPromptService(this))
    , m_pConfig(new MCPServerConfig(this))
    , m_pHandler(nullptr)
    , m_pThread(new QThread(this))
{
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir dir(logDir);
    if (!dir.exists()) {
        QFile::Permissions permissions;
        permissions.setFlag(QFile::Permission::ReadOwner, true);
        permissions.setFlag(QFile::Permission::ReadGroup, true);
        permissions.setFlag(QFile::Permission::WriteOwner, true);
        permissions.setFlag(QFile::Permission::WriteGroup, true);
        permissions.setFlag(QFile::Permission::ExeOwner, true);
        permissions.setFlag(QFile::Permission::ExeGroup, true);
        dir.mkpath(logDir, permissions);
    }

    // Initialize module logging system
    QString logFile = dir.absoluteFilePath("mcpserver.log");
    MCPLog::instance()->initialize(logFile, LogLevel::Debug, true);

    // Create business handler, responsible for message processing and business logic (internally creates request dispatcher)
    m_pHandler = new MCPServerHandler(this, this);

    // Connect transport layer's message received signal to business handler
    QObject::connect(m_pTransport, &IMCPTransport::messageReceived, m_pHandler, &MCPServerHandler::onClientMessageReceived);

    // Connect resource service's signal to business handler (MCPServerHandler internally forwards to corresponding sub-Handler)
    QObject::connect(m_pResourceService, &MCPResourceService::resourceContentChanged, m_pHandler, &MCPServerHandler::onResourceContentChanged);
    QObject::connect(m_pResourceService, &MCPResourceService::resourcesListChanged, m_pHandler, &MCPServerHandler::onResourcesListChanged);
    QObject::connect(m_pResourceService, &MCPResourceService::resourceDeleted, m_pHandler, &MCPServerHandler::onResourceDeleted);

    // Connect tool service's signal to business handler (MCPServerHandler internally forwards to corresponding sub-Handler)
    QObject::connect(m_pToolService, &MCPToolService::toolsListChanged, m_pHandler, &MCPServerHandler::onToolsListChanged);

    // Connect prompt service's signal to business handler (MCPServerHandler internally forwards to corresponding sub-Handler)
    QObject::connect(m_pPromptService, &MCPPromptService::promptsListChanged, m_pHandler, &MCPServerHandler::onPromptsListChanged);

    // Connect configuration loaded signal to config application slot
    QObject::connect(m_pConfig, &MCPServerConfig::configLoaded, this, &MCPServer::onConfigLoaded);

    m_pThread->setObjectName("MCPServer-WorkerThread");
}

MCPServer::~MCPServer()
{
    if (m_pThread->isRunning()) {
        MCPInvokeHelper::syncInvoke(this, [this]() { doStop(); });
        m_pThread->quit();
        m_pThread->wait();
        setParent(nullptr);
    }
}

bool MCPServer::start()
{
    // If thread is already running, directly call doStart
    if (!m_pThread->isRunning()) {
        // Start worker thread
        moveToThread(m_pThread);
        m_pThread->start();
        // Start server in worker thread
        MCPInvokeHelper::asynInvoke(this, [this]() { doStart(); });
    }
    return true;
}

void MCPServer::stop()
{
    if (m_pThread->isRunning()) {
        MCPInvokeHelper::syncInvoke(this, [this]() { doStop(); });
        m_pThread->quit();
        m_pThread->wait();
        setParent(nullptr);
    }
}

bool MCPServer::isRunning()
{
    return m_pTransport->isRunning();
}

IMCPServerConfig *MCPServer::getConfig()
{
    return m_pConfig;
}

MCPToolService *MCPServer::getToolService()
{
    return m_pToolService;
}

MCPResourceService *MCPServer::getResourceService()
{
    return m_pResourceService;
}

MCPPromptService *MCPServer::getPromptService()
{
    return m_pPromptService;
}

IMCPTransport *MCPServer::getTransport() const
{
    return m_pTransport;
}

MCPSessionService *MCPServer::getSessionService() const
{
    return m_pSessionService;
}

bool MCPServer::doStart()
{
    // Start transport layer
    auto nPort = m_pConfig->getPort();
    if (!m_pTransport->start(nPort)) {
        MCP_CORE_LOG_WARNING() << "MCPServer: doStart error";
        return false;
    }

    MCP_CORE_LOG_INFO() << "MCPServer: started:" << nPort;
    return true;
}

bool MCPServer::doStop()
{
    MCP_CORE_LOG_INFO() << "MCPServer: doStop...";
    m_pTransport->stop();
    return true;
}

bool MCPServer::initServer(QSharedPointer<MCPToolsConfig> pToolsConfig, QSharedPointer<MCPResourcesConfig> pResourcesConfig, QSharedPointer<MCPPromptsConfig> pPromptsConfig)
{
    Q_UNUSED(pResourcesConfig);

    // Pre-resolve all Handlers (one-time resolution to avoid repeated traversal of object tree)
    QMap<QString, QObject *> dictHandlers = MCPHandlerResolver::resolveDefaultHandlers();

    // 1. Apply tool configuration
    if (pToolsConfig != nullptr) {
        for (const auto &config : pToolsConfig->getTools()) {
            m_pToolService->addFromConfig(config, dictHandlers);
        }
    }

    // 2. Apply resource configuration
    if (pResourcesConfig != nullptr) {
        for (const auto &config : pResourcesConfig->getResources()) {
            m_pResourceService->addFromConfig(config, dictHandlers);
        }
    }

    // 3. Apply prompt configuration
    if (pPromptsConfig != nullptr) {
        for (const auto &config : pPromptsConfig->getPrompts()) {
            m_pPromptService->addFromConfig(config);
        }
    }

    return true;
}

void MCPServer::onThreadReady()
{
    // Empty implementation, only used to ensure worker thread event loop is started
}

void MCPServer::onConfigLoaded(QSharedPointer<MCPToolsConfig> pToolsConfig, QSharedPointer<MCPResourcesConfig> pResourcesConfig, QSharedPointer<MCPPromptsConfig> pPromptsConfig)
{
    // Configuration loaded, apply configuration
    initServer(pToolsConfig, pResourcesConfig, pPromptsConfig);
}
