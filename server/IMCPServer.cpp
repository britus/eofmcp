/**
 * @file MCPServer.cpp
 * @brief MCP server implementation
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <IMCPServer.h>
#include <MCPAutoServer.h>
#include <MCPInvokeHelper.h>
#include <MCPLog.h>
#include <MCPServer.h>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

IMCPServer::IMCPServer(QObject *pParent)
    : QObject(pParent)
{}

IMCPServer *IMCPServer::createServer()
{

    MCPServer *server = new MCPServer();

    auto pConfig = server->getConfig();

    QFile::Permissions permissions;
    permissions.setFlag(QFile::Permission::ReadOwner, true);
    permissions.setFlag(QFile::Permission::ReadGroup, true);
    permissions.setFlag(QFile::Permission::WriteOwner, true);
    permissions.setFlag(QFile::Permission::WriteGroup, true);
    permissions.setFlag(QFile::Permission::ExeOwner, true);
    permissions.setFlag(QFile::Permission::ExeGroup, true);

    QString cfgDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(cfgDir);
    if (!dir.exists()) {
        dir.mkdir(cfgDir, permissions);
    }

    QString strConfigDir = dir.absolutePath();

    if (!pConfig->loadFromDirectory(strConfigDir)) {
        MCP_CORE_LOG_WARNING() << "MCPAutoServer: loadFromDirectory failed";
        delete pConfig;
        return nullptr;
    }

    if (!server->start()) {
        MCP_CORE_LOG_WARNING() << "MCPAutoServer: start failed";
        IMCPServer::destroyServer(server);
        delete pConfig;
        return nullptr;
    }

    MCP_CORE_LOG_INFO() << "MCPAutoServer: Server port:" << pConfig->getPort();

    return server;
}

void IMCPServer::destroyServer(IMCPServer *pServer)
{

    if (pServer != nullptr) {
        pServer->stop();
        pServer->deleteLater();
    }
}

IMCPServer::~IMCPServer() {}

#if 0
//
static MCPAutoServer autoServer;
void StartAutoMCPServer()
{

    autoServer.performStart();
}
void FlushAutoMCPServerTool(const char *szToolConfigFile)
{

    auto strConfigFile = QString::fromUtf8(szToolConfigFile);
    autoServer.loadTool(strConfigFile);
}
void StopAutoMCPServer()
{

    autoServer.performStop();
}
#endif