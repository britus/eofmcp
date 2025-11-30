/**
 * @file MCPXServer.cpp
 * @brief MCP X服务器实现
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
    QFile::Permissions permissions;
    permissions.setFlag(QFile::Permission::ReadOwner, true);
    permissions.setFlag(QFile::Permission::ReadGroup, true);
    permissions.setFlag(QFile::Permission::WriteOwner, true);
    permissions.setFlag(QFile::Permission::WriteGroup, true);
    permissions.setFlag(QFile::Permission::ExeOwner, true);
    permissions.setFlag(QFile::Permission::ExeGroup, true);

    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(logDir, permissions);
    }

    // 初始化模块日志系统
    QString logFile = dir.absoluteFilePath("mcpserver.log");
    MCPLog::instance()->initialize(logFile, LogLevel::Debug, true);

    // 创建业务处理器，负责消息处理和业务逻辑（内部会创建请求调度器）
    m_pHandler = new MCPServerHandler(this, this);

    // 连接传输层的消息接收信号到业务处理器
    QObject::connect(m_pTransport, &IMCPTransport::messageReceived, m_pHandler, &MCPServerHandler::onClientMessageReceived);

    // 连接资源服务的信号到业务处理器（MCPServerHandler内部会转发到对应的子Handler）
    QObject::connect(m_pResourceService, &MCPResourceService::resourceContentChanged, m_pHandler, &MCPServerHandler::onResourceContentChanged);
    QObject::connect(m_pResourceService, &MCPResourceService::resourcesListChanged, m_pHandler, &MCPServerHandler::onResourcesListChanged);
    QObject::connect(m_pResourceService, &MCPResourceService::resourceDeleted, m_pHandler, &MCPServerHandler::onResourceDeleted);

    // 连接工具服务的信号到业务处理器（MCPServerHandler内部会转发到对应的子Handler）
    QObject::connect(m_pToolService, &MCPToolService::toolsListChanged, m_pHandler, &MCPServerHandler::onToolsListChanged);

    // 连接提示词服务的信号到业务处理器（MCPServerHandler内部会转发到对应的子Handler）
    QObject::connect(m_pPromptService, &MCPPromptService::promptsListChanged, m_pHandler, &MCPServerHandler::onPromptsListChanged);

    // 连接配置加载完成信号到配置应用槽（传递配置数据）
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
    // 如果线程已经在运行，直接调用 doStart
    if (!m_pThread->isRunning()) {
        // 启动工作线程
        moveToThread(m_pThread);
        m_pThread->start();
        // 在工作线程中启动服务器
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
    // 启动传输层
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

    // 预先解析所有Handlers（一次性解析，避免重复遍历对象树）
    QMap<QString, QObject *> dictHandlers = MCPHandlerResolver::resolveDefaultHandlers();

    // 1. 应用工具配置
    if (pToolsConfig != nullptr) {
        for (const auto &toolConfig : pToolsConfig->getTools()) {
            m_pToolService->addFromConfig(toolConfig, dictHandlers);
        }
    }

    // 3. 应用提示词配置
    if (pPromptsConfig != nullptr) {
        for (const auto &promptConfig : pPromptsConfig->getPrompts()) {
            m_pPromptService->addFromConfig(promptConfig);
        }
    }
    return true;
}

void MCPServer::onThreadReady()
{
    // 空实现，仅用于确保工作线程事件循环已启动
}

void MCPServer::onConfigLoaded(QSharedPointer<MCPToolsConfig> pToolsConfig, QSharedPointer<MCPResourcesConfig> pResourcesConfig, QSharedPointer<MCPPromptsConfig> pPromptsConfig)
{
    // 配置加载完成，应用配置
    initServer(pToolsConfig, pResourcesConfig, pPromptsConfig);
}
