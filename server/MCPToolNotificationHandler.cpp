/**
 * @file MCPToolNotificationHandler.cpp
 * @brief MCP工具通知处理器实现
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
    //MCP_CORE_LOG_INFO() << "MCPToolNotificationHandler: onToolsListChanged()";

    // 获取最新的工具列表
    QJsonArray arrTools = m_pServer->getToolService()->list();

    // 构建通知参数
    QJsonObject params;
    params["tools"] = arrTools;

    // 广播通知
    broadcastNotification("notifications/tools/list_changed", params);

    //MCP_CORE_LOG_INFO() << "MCPToolNotificationHandler: onToolsListChanged done.";
}
