/**
 * @file MCPSubscriptionHandler.cpp
 * @brief MCP subscription handler implementation
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */
#include "MCPSubscriptionHandler.h"
#include "MCPClientMessage.h"
#include "MCPContext.h"
#include "MCPLog.h"
#include "MCPResourceService.h"
#include "MCPServerMessage.h"

MCPSubscriptionHandler::MCPSubscriptionHandler(MCPResourceService *pResourceService, QObject *pParent)
    : QObject(pParent)
    , m_pResourceService(pResourceService)
{
    if (!m_pResourceService) {
        MCP_CORE_LOG_WARNING() << "MCPSubscriptionHandler: Resource service is null";
    }
}

MCPSubscriptionHandler::~MCPSubscriptionHandler() {}

QSharedPointer<MCPServerMessage> MCPSubscriptionHandler::handleSubscribe(const QSharedPointer<MCPContext> &pContext)
{
    if (!m_pResourceService) {
        // According to the MCP protocol specification, internal errors should return -32603, and error messages should be in English
        return QSharedPointer<MCPServerErrorResponse>::create(pContext, MCPError::internalError("Resource service not initialized"));
    }

    auto pClientMessage = pContext->getClientMessage().dynamicCast<MCPClientMessage>();
    auto jsonParams = pClientMessage->getParmams().toObject();
    QString strUri = jsonParams.value("uri").toString();

    if (strUri.isEmpty()) {
        // According to the JSON-RPC 2.0 and MCP protocol specifications, error messages should be in English
        return QSharedPointer<MCPServerErrorResponse>::create(pContext, MCPError::invalidParams("Missing required parameter: uri"));
    }

    // Validate if the resource exists (optional, according to MCP protocol specification)
    // If the resource does not exist, subscription is still allowed so that notifications can be received when the resource is created

    // Perform subscription (using sessionId instead of connectionId)
    auto pSession = pContext->getSession();
    if (!pSession) {
        // According to the JSON-RPC 2.0 and MCP protocol specifications, error messages should be in English
        return QSharedPointer<MCPServerErrorResponse>::create(pContext, MCPError::invalidRequest("Session not found"));
    }

    QString strSessionId = pSession->getSessionId();
    bool bSuccess = m_pResourceService->subscribe(strUri, strSessionId);

    if (!bSuccess) {
        // According to the MCP protocol specification, internal errors should return -32603, and error messages should be in English
        return QSharedPointer<MCPServerErrorResponse>::create(pContext, MCPError::internalError("Subscription failed"));
    }

    MCP_CORE_LOG_INFO() << "MCPSubscriptionHandler: Session" << strSessionId << "subscribed to URI:" << strUri;

    // Return success response (empty result indicates success)
    return QSharedPointer<MCPServerMessage>::create(pContext, QJsonObject());
}

QSharedPointer<MCPServerMessage> MCPSubscriptionHandler::handleUnsubscribe(const QSharedPointer<MCPContext> &pContext)
{
    if (!m_pResourceService) {
        // According to the MCP protocol specification, internal errors should return -32603, and error messages should be in English
        return QSharedPointer<MCPServerErrorResponse>::create(pContext, MCPError::internalError("Resource service not initialized"));
    }

    auto pClientMessage = pContext->getClientMessage().dynamicCast<MCPClientMessage>();
    auto jsonParams = pClientMessage->getParmams().toObject();
    QString strUri = jsonParams.value("uri").toString();

    if (strUri.isEmpty()) {
        // According to the JSON-RPC 2.0 and MCP protocol specifications, error messages should be in English
        return QSharedPointer<MCPServerErrorResponse>::create(pContext, MCPError::invalidParams("Missing required parameter: uri"));
    }

    // Perform unsubscription (using sessionId instead of connectionId)
    auto pSession = pContext->getSession();
    if (!pSession) {
        // According to the JSON-RPC 2.0 and MCP protocol specifications, error messages should be in English
        return QSharedPointer<MCPServerErrorResponse>::create(pContext, MCPError::invalidRequest("Session not found"));
    }

    QString strSessionId = pSession->getSessionId();
    bool bSuccess = m_pResourceService->unsubscribe(strUri, strSessionId);

    if (!bSuccess) {
        // If not subscribed, return an error (according to the MCP protocol specification, error messages should be in English)
        QJsonObject data;
        data["uri"] = strUri;
        return QSharedPointer<MCPServerErrorResponse>::create(pContext, MCPError(MCPErrorCode::INVALID_REQUEST, "Not subscribed to URI", data));
    }

    MCP_CORE_LOG_INFO() << "MCPSubscriptionHandler: Session" << strSessionId << "unsubscribed from URI:" << strUri;

    // Return success response (empty result indicates success)
    return QSharedPointer<MCPServerMessage>::create(pContext, QJsonObject());
}
