/**
 * @file MCPPromptNotificationHandler.cpp
 * @brief MCP提示词通知处理器实现
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <MCPLog.h>
#include <MCPPromptNotificationHandler.h>
#include <MCPPromptService.h>
#include <MCPServer.h>

MCPPromptNotificationHandler::MCPPromptNotificationHandler(MCPServer *pServer, QObject *pParent)
    : MCPNotificationHandlerBase(pServer, pParent)
{}

MCPPromptNotificationHandler::~MCPPromptNotificationHandler() {}

void MCPPromptNotificationHandler::onPromptsListChanged()
{
    MCP_CORE_LOG_INFO() << "MCPPromptNotificationHandler: onPromptsListChanged";

    // 获取最新的提示词列表
    QJsonArray arrPrompts = m_pServer->getPromptService()->list();

    // 构建通知参数
    QJsonObject params;
    params["prompts"] = arrPrompts;

    // 广播通知
    broadcastNotification("notifications/prompts/list_changed", params);

    MCP_CORE_LOG_INFO() << "MCPPromptNotificationHandler: onPromptsListChanged done.";
}
