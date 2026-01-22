/**
 * @file MCPToolNotificationHandler.cpp
 * @brief MCP tool notification handler implementation
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */
#include <MCPLog.h>
#include <MCPServer.h>
#include <MCPToolNotificationHandler.h>
#include <MCPToolService.h>

MCPToolNotificationHandler::MCPToolNotificationHandler(MCPServer *pServer, QObject *pParent)
    : MCPNotificationHandlerBase(pServer, pParent)
{}

MCPToolNotificationHandler::~MCPToolNotificationHandler() {}

void MCPToolNotificationHandler::onToolsListChanged()
{
    //MCP_CORE_LOG_INFO() << "MCPToolNotificationHandler: Tools list changed, sending notification to all clients";

    // Get the latest tools list
    QJsonArray arrTools = m_pServer->getToolService()->list();

    // Build notification parameters
    QJsonObject params;
    params["tools"] = arrTools;

    // Broadcast notification
    broadcastNotification("notifications/tools/list_changed", params);
}