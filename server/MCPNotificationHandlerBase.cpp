/**
 * @file MCPNotificationHandlerBase.cpp
 * @brief MCP通知处理器基类实现
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <MCPLog.h>
#include <MCPNotificationHandlerBase.h>
#include <MCPPendingNotification.h>
#include <MCPServer.h>
#include <MCPSession.h>
#include <MCPSessionService.h>

MCPNotificationHandlerBase::MCPNotificationHandlerBase(MCPServer *pServer, QObject *pParent)
    : QObject(pParent)
    , m_pServer(pServer)
{}

MCPNotificationHandlerBase::~MCPNotificationHandlerBase() {}

void MCPNotificationHandlerBase::broadcastNotification(const QString &strMethod, const QJsonObject &objParams)
{
    auto pSessionService = m_pServer->getSessionService();

    // 获取所有会话
    QList<QSharedPointer<MCPSession>> allSessions = pSessionService->getAllSessions();

    // 构建通知消息
    QJsonObject notification;
    notification["method"] = strMethod;
    notification["params"] = objParams;

    // 遍历所有会话，根据传输类型决定处理方式
    for (const auto &pSession : allSessions) {
        if (pSession == nullptr) {
            continue;
        }

        QString strSessionId = pSession->getSessionId();

        if (pSession->isStreamableTransport()) {
            // StreamableTransport：缓存通知标记，等待下次请求时发送
            // 根据方法名判断通知类型
            if (strMethod == "notifications/resources/updated") {
                QString strUri = objParams.value("uri").toString();
                pSession->addResourceChangedNotification(strUri);
            } else if (strMethod == "notifications/resources/list_changed") {
                pSession->addResourcesListChangedNotification();
            } else if (strMethod == "notifications/tools/list_changed") {
                pSession->addToolsListChangedNotification();
            } else if (strMethod == "notifications/prompts/list_changed") {
                pSession->addPromptsListChangedNotification();
            } else {
                // 未知的通知类型，使用通用方法（向后兼容）
                MCP_CORE_LOG_WARNING() << "MCPNotificationHandlerBase: method:" << strMethod;
            }
            //MCP_CORE_LOG_DEBUG() << "MCPNotificationHandlerBase: StreamableTransport id:" << strSessionId << "method:" << strMethod;
        } else {
            // SSE传输：通过信号发送通知
            emit notificationRequested(strSessionId, notification);
            MCP_CORE_LOG_DEBUG() << "MCPNotificationHandlerBase id:" << strSessionId << "method:" << strMethod << "notification:" << notification;
        }
    }
}

void MCPNotificationHandlerBase::sendNotificationToSubscribers(const QString &strMethod, const QJsonObject &objParams, const QSet<QString> &setSubscribedSessionIds)
{
    if (setSubscribedSessionIds.isEmpty()) {
        MCP_CORE_LOG_DEBUG() << "MCPNotificationHandlerBase: sendNotificationToSubscribers:" << strMethod;
        return;
    }

    // 构建通知消息
    QJsonObject notification;
    notification["method"] = strMethod;
    notification["params"] = objParams;

    auto pSessionService = m_pServer->getSessionService();

    // 遍历每个订阅者，根据传输类型决定处理方式
    for (const QString &strSessionId : setSubscribedSessionIds) {
        auto pSession = pSessionService->getSessionBySessionId(strSessionId);
        if (pSession == nullptr) {
            continue;
        }
        if (pSession->isStreamableTransport()) {
            // StreamableTransport：缓存通知标记，等待下次请求时发送
            // 对于资源变化通知，需要包含URI信息
            if (strMethod == "notifications/resources/updated") {
                QString strUri = objParams.value("uri").toString();
                pSession->addResourceChangedNotification(strUri);
            } else if (strMethod == "notifications/resources/list_changed") {
                pSession->addResourcesListChangedNotification();
            } else if (strMethod == "notifications/tools/list_changed") {
                pSession->addToolsListChangedNotification();
            } else if (strMethod == "notifications/prompts/list_changed") {
                pSession->addPromptsListChangedNotification();
            } else {
                // 未知的通知类型，使用通用方法（向后兼容）
                MCP_CORE_LOG_WARNING() << "MCPNotificationHandlerBase: unknown:" << strMethod;
            }
            //MCP_CORE_LOG_DEBUG() << "MCPNotificationHandlerBase: StreamableTransport id:" << strSessionId << "method:" << strMethod;
        } else {
            // SSE传输：通过信号发送通知
            emit notificationRequested(strSessionId, notification);
            MCP_CORE_LOG_DEBUG() << "MCPNotificationHandlerBase: id:" << strSessionId << "method:" << strMethod;
        }
    }
}
