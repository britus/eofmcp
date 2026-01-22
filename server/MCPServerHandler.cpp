/**
 * @file MCPServerHandler.cpp
 * @brief Implementation of the MCP server business handler class
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */
#include <IMCPTransport.h>
#include <MCPClientMessage.h>
#include <MCPContext.h>
#include <MCPHttpReplyMessage.h>
#include <MCPLog.h>
#include <MCPMessage.h>
#include <MCPMessageSender.h>
#include <MCPMessageType.h>
#include <MCPPendingNotification.h>
#include <MCPPromptNotificationHandler.h>
#include <MCPPromptService.h>
#include <MCPRequestDispatcher.h>
#include <MCPResource.h>
#include <MCPResourceNotificationHandler.h>
#include <MCPResourceService.h>
#include <MCPServer.h>
#include <MCPServerHandler.h>
#include <MCPServerMessage.h>
#include <MCPSession.h>
#include <MCPSessionService.h>
#include <MCPToolNotificationHandler.h>
#include <MCPToolService.h>
#include <QJsonArray>

MCPServerHandler::MCPServerHandler(MCPServer *pServer, QObject *pParent)
    : QObject(pParent)
    , m_pServer(pServer)
    , m_pRequestDispatcher(nullptr)
    , m_pMessageSender(nullptr)
    , m_pResourceNotificationHandler(nullptr)
    , m_pToolNotificationHandler(nullptr)
    , m_pPromptNotificationHandler(nullptr)
{
    // Create message sender (unified handling of message sending logic)
    m_pMessageSender = new MCPMessageSender(pServer->getTransport(), this);

    // Create request dispatcher (responsible for routing and dispatching client requests)
    m_pRequestDispatcher = new MCPRequestDispatcher(pServer, this);

    // Connect request dispatcher's server message signal to this Handler
    QObject::connect(m_pRequestDispatcher, &MCPRequestDispatcher::serverMessageReceived, this, &MCPServerHandler::onServerMessageReceived);

    // Create various notification handlers
    m_pResourceNotificationHandler = new MCPResourceNotificationHandler(pServer, this);
    m_pToolNotificationHandler = new MCPToolNotificationHandler(pServer, this);
    m_pPromptNotificationHandler = new MCPPromptNotificationHandler(pServer, this);

    // Connect various notification handler's notification request signals to this Handler
    QObject::connect(m_pResourceNotificationHandler, &MCPNotificationHandlerBase::notificationRequested, this, &MCPServerHandler::onNotificationRequested);
    QObject::connect(m_pToolNotificationHandler, &MCPNotificationHandlerBase::notificationRequested, this, &MCPServerHandler::onNotificationRequested);
    QObject::connect(m_pPromptNotificationHandler, &MCPNotificationHandlerBase::notificationRequested, this, &MCPServerHandler::onNotificationRequested);
}

MCPServerHandler::~MCPServerHandler() {}

void MCPServerHandler::onClientMessageReceived(quint64 nConnectionId, const QSharedPointer<MCPMessage> &pMessage)
{
    if (auto pClientMessage = pMessage.dynamicCast<MCPClientMessage>()) {
        if (auto pSession = m_pServer->getSessionService()->getSession(nConnectionId, pClientMessage)) {
            auto strMethodName = pClientMessage->getMethodName();
            auto pContext = QSharedPointer<MCPContext>::create(nConnectionId, pSession, pClientMessage);
            if (auto pResponse = m_pRequestDispatcher->handleClientMessage(pContext)) {
                onServerMessageReceived(pResponse);
            }
        }
    }
}

void MCPServerHandler::onServerMessageReceived(const QSharedPointer<MCPMessage> &pMessage)
{
    if (auto pServerMessage = pMessage.dynamicCast<MCPServerMessage>()) {
        // For Streamable transport responses, pending notifications need to be sent first
        auto enMessageType = pServerMessage->getType();
        if ((enMessageType & MCPMessageType::StreamableTransport) && (enMessageType & MCPMessageType::Response)) {
            sendStreamableTransportPendingNotifications(pServerMessage);
        }

        // Use message sender to send messages uniformly
        m_pMessageSender->sendMessage(pServerMessage);
    }
}

void MCPServerHandler::onConnectionClosed(quint64 nConnectionId)
{
    // First get session to unsubscribe
    auto pSession = m_pServer->getSessionService()->getSessionByConnectionId(nConnectionId);
    if (pSession) {
        QString strSessionId = pSession->getSessionId();
        // Unsubscribe all subscriptions for this session (using sessionId)
        m_pServer->getResourceService()->unsubscribeAll(strSessionId);
    }

    // Remove session
    m_pServer->getSessionService()->removeSessionBySSEConnectId(nConnectionId);
}

// Note: onSseTransportServerMessageReceived and onStreamableTransportServerMessageReceived
// methods have been removed, message sending logic is unified by MCPMessageSender

void MCPServerHandler::sendStreamableTransportPendingNotifications(const QSharedPointer<MCPServerMessage> &pServerMessage)
{
    auto pContext = pServerMessage->getContext();
    if (pContext == nullptr) {
        return;
    }

    auto pSession = pContext->getSession();
    if (pSession == nullptr) {
        return;
    }

    // Check if there are pending notifications to send
    if (!pSession->hasPendingNotifications()) {
        return;
    }

    // Generate and send notifications based on notification objects
    auto lstPendingNotifications = pSession->takePendingNotifications();
    for (const MCPPendingNotification &notification : lstPendingNotifications) {
        QJsonObject notificationObj;
        if (notification.isResourceChanged()) {
            notificationObj = generateResourceChangedNotification(notification);
        } else if (notification.isResourcesListChanged()) {
            // Resource list change notification: generated by method name
            notificationObj = generateNotificationByMethod(notification.getMethod());
        } else if (notification.isToolsListChanged()) {
            // Tool list change notification: generated by method name
            notificationObj = generateNotificationByMethod(notification.getMethod());
        } else if (notification.isPromptsListChanged()) {
            // Prompt list change notification: generated by method name
            notificationObj = generateNotificationByMethod(notification.getMethod());
        } else {
            // Unknown notification type
            MCP_CORE_LOG_WARNING() << "MCPServerHandler: sendStreamableTransportPendingNotifications invalid type:" << static_cast<int>(notification.getType());
            continue;
        }

        if (notificationObj.isEmpty()) {
            MCP_CORE_LOG_WARNING() << "MCPServerHandler: sendStreamableTransportPendingNotifications no object for:" << notification.getMethod();
            continue;
        }

        // Create notification message
        auto pNotificationMessage = QSharedPointer<MCPServerMessage>::create(pContext, notificationObj, MCPMessageType::StreamableTransport | MCPMessageType::RequestNotification);

        // Use message sender to send notification
        m_pMessageSender->sendMessage(pNotificationMessage);

        //MCP_CORE_LOG_DEBUG() << "MCPServerHandler: StreamableTransport:" << notification.getMethod();
    }
}

void MCPServerHandler::onSubscriptionNotification(const QString &strSessionId, const QJsonObject &objNotification)
{
    // Get session by sessionId
    auto pSession = m_pServer->getSessionService()->getSessionBySessionId(strSessionId);

    if (pSession == nullptr) {
        MCP_CORE_LOG_WARNING() << "MCPServerHandler: invalid session:" << strSessionId;
        return;
    }

    // Determine handling method based on transport type
    QString strMethod = objNotification.value("method").toString();
    if (pSession->isStreamableTransport()) {
        // StreamableTransport: cache notification flags
        if (!strMethod.isEmpty()) {
            // Determine notification type by method name
            if (strMethod == "notifications/resources/updated") {
                QJsonObject params = objNotification.value("params").toObject();
                QString strUri = params.value("uri").toString();
                pSession->addResourceChangedNotification(strUri);
                MCP_CORE_LOG_DEBUG() << "MCPServerHandler: resources StreamableTransport added for uri:" << strUri;
            } else if (strMethod == "notifications/resources/list_changed") {
                pSession->addResourcesListChangedNotification();
                MCP_CORE_LOG_DEBUG() << "MCPServerHandler: ResourcesList StreamableTransport added.";
            } else if (strMethod == "notifications/tools/list_changed") {
                pSession->addToolsListChangedNotification();
                MCP_CORE_LOG_DEBUG() << "MCPServerHandler: ToolsList StreamableTransport added.";
            } else if (strMethod == "notifications/prompts/list_changed") {
                pSession->addPromptsListChangedNotification();
                MCP_CORE_LOG_DEBUG() << "MCPServerHandler: PromptsList StreamableTransport added.";
            } else {
                // Unknown notification type
                MCP_CORE_LOG_WARNING() << "MCPServerHandler::onSubscriptionNotification unknown method:" << strMethod;
            }
        }
    } else {
        // SSE transport: send notification immediately
        // Get SSE connection ID
        quint64 nSseConnectionId = pSession->getSseConnectionId();
        if (nSseConnectionId == 0) {
            MCP_CORE_LOG_WARNING() << "MCPServerHandler: invalid sessionId" << strSessionId;
            return;
        }

        auto pClientMessage = QSharedPointer<MCPClientMessage>::create(MCPMessageType::SseTransport | MCPMessageType::Notification);

        // Create Context
        auto pContext = QSharedPointer<MCPContext>::create(nSseConnectionId, pSession, pClientMessage);

        // Create notification message (notification messages don't need id field)
        auto pNotificationMessage = QSharedPointer<MCPServerMessage>::create(pContext, objNotification, MCPMessageType::SseTransport | MCPMessageType::RequestNotification);

        // Use message sender to send notification
        m_pMessageSender->sendMessage(pNotificationMessage);
    }
}

void MCPServerHandler::onNotificationRequested(const QString &strSessionId, const QJsonObject &objNotification)
{
    // Directly call onSubscriptionNotification to handle notification
    onSubscriptionNotification(strSessionId, objNotification);
}

void MCPServerHandler::onResourceContentChanged(const QString &strUri)
{
    // Forward to resource notification handler
    m_pResourceNotificationHandler->onResourceContentChanged(strUri);
}

void MCPServerHandler::onResourceDeleted(const QString &strUri)
{
    // Forward to resource notification handler
    m_pResourceNotificationHandler->onResourceDeleted(strUri);
}

void MCPServerHandler::onResourcesListChanged()
{
    // Forward to resource notification handler
    m_pResourceNotificationHandler->onResourcesListChanged();
}

void MCPServerHandler::onToolsListChanged()
{
    // Forward to tool notification handler
    m_pToolNotificationHandler->onToolsListChanged();
}

void MCPServerHandler::onPromptsListChanged()
{
    // Forward to prompt notification handler
    m_pPromptNotificationHandler->onPromptsListChanged();
}

MCPResourceNotificationHandler *MCPServerHandler::getResourceNotificationHandler() const
{
    return m_pResourceNotificationHandler;
}

MCPToolNotificationHandler *MCPServerHandler::getToolNotificationHandler() const
{
    return m_pToolNotificationHandler;
}

MCPPromptNotificationHandler *MCPServerHandler::getPromptNotificationHandler() const
{
    return m_pPromptNotificationHandler;
}

QJsonObject MCPServerHandler::generateNotificationByMethod(const QString &strMethod)
{
    QJsonObject notification;
    notification["method"] = strMethod;

    if (strMethod == "notifications/resources/list_changed") {
        // Resource list change notification: includes current resource list
        QJsonArray resources = m_pServer->getResourceService()->list();
        QJsonObject params;
        params["resources"] = resources;
        notification["params"] = params;
    } else if (strMethod == "notifications/tools/list_changed") {
        // Tool list change notification: includes current tool list
        QJsonArray tools = m_pServer->getToolService()->list();
        QJsonObject params;
        params["tools"] = tools;
        notification["params"] = params;
    } else if (strMethod == "notifications/prompts/list_changed") {
        // Prompt list change notification: includes current prompt list
        QJsonArray prompts = m_pServer->getPromptService()->list();
        QJsonObject params;
        params["prompts"] = prompts;
        notification["params"] = params;
    } else {
        // Unknown notification method, return empty object
        MCP_CORE_LOG_WARNING() << "MCPServerHandler: unknown method:" << strMethod;
        return QJsonObject();
    }

    return notification;
}

QJsonObject MCPServerHandler::generateResourceChangedNotification(const MCPPendingNotification &notification)
{
    // Resource change notification: re-read resource data and metadata
    QString strUri = notification.getUri();
    if (strUri.isEmpty()) {
        MCP_CORE_LOG_WARNING() << "MCPServerHandler: generateResourceChangedNotification URI is empty";
        return QJsonObject();
    }

    auto pResourceService = m_pServer->getResourceService();
    QJsonObject resourceData;

    if (pResourceService->has(strUri)) {
        // Get resource content (includes contents array)
        QJsonObject resourceInfo = pResourceService->readResource(strUri);
        // Get resource metadata (name, description, mimeType)
        if (auto pResource = pResourceService->getResource(strUri)) {
            QJsonObject metadata = pResource->getMetadata();
            // Merge metadata into resource information
            resourceInfo["name"] = metadata["name"];
            resourceInfo["description"] = metadata["description"];
            resourceInfo["mimeType"] = metadata["mimeType"];
        }
        resourceData["resource"] = QJsonValue(resourceInfo);
    } else {
        resourceData["deleted"] = true;
    }

    QJsonObject notificationObj;
    notificationObj["method"] = notification.getMethod();
    QJsonObject params;
    params["uri"] = strUri;
    params["data"] = QJsonValue(resourceData);
    notificationObj["params"] = QJsonValue(params);
    return notificationObj;
}