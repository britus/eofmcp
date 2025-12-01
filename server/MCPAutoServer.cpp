/**
 * @file MCPAutoServer.cpp
 * @brief MCP自动服务器实现
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */
#ifdef Q_OS_WINDOWS
#include <windows.h>
#endif

#include <IMCPServer.h>
#include <IMCPToolService.h>
#include <MCPAutoServer.h>
#include <MCPHandlerResolver.h>
#include <MCPInvokeHelper.h>
#include <MCPLog.h>
#include <MCPServer.h>
#include <MCPServerConfig.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QMap>
#include <QStandardPaths>

MCPAutoServer::MCPAutoServer(QObject *pParent)
    : QObject(pParent)
    , m_pServer(nullptr)
{}

MCPAutoServer::~MCPAutoServer() {}

void MCPAutoServer::performStart()
{
    MCP_CORE_LOG_INFO() << "MCPAutoServer: performStart...";
    m_pServer = IMCPServer::createServer();
}

void MCPAutoServer::performStop()
{
    MCP_CORE_LOG_INFO() << "MCPAutoServer: performStop...";
    if (m_pServer != nullptr) {
        IMCPServer::destroyServer(m_pServer);
        m_pServer = nullptr;
    }
}

void MCPAutoServer::loadTool(const QString &strToolConfigFile)
{
    if (m_pServer == nullptr) {
        return;
    }

    QFile file(strToolConfigFile);
    if (!file.open(QIODevice::ReadOnly)) {
        MCP_CORE_LOG_WARNING() << "MCPAutoServer: Unable to open file: " << strToolConfigFile;
        return;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError error;
    auto jsonTool = QJsonDocument::fromJson(jsonData, &error).object();
    if (error.error != QJsonParseError::NoError) {
        MCP_CORE_LOG_WARNING() << "MCPAutoServer: Failed to parse JSON file: " //
                               << strToolConfigFile << "\n"
                               << error.errorString();
        return;
    }

    MCP_CORE_LOG_DEBUG().noquote() << "MCPAutoServer: Register tool:" << strToolConfigFile;

    MCPInvokeHelper::asynInvoke(m_pServer, [this, jsonTool]() { //
        if (!m_pServer->getToolService()->addFromJson(jsonTool)) {
            MCP_CORE_LOG_WARNING() << "MCPAutoServer: Failed to add tool: " //
                                   << jsonTool;
        }
    });
}

#ifdef Q_OS_WINDOWS
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            // 增加模块引用计数
            //GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, (LPCTSTR)hModule, &hModule);
            break;
    }
    return TRUE;
}
#endif
