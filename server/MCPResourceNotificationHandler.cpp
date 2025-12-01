/**
 * @file MCPResourceNotificationHandler.cpp
 * @brief MCP resource notification handler implementation
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <MCPLog.h>
#include <MCPResource.h>
#include <MCPResourceNotificationHandler.h>
#include <MCPResourceService.h>
#include <MCPServer.h>

MCPResourceNotificationHandler::MCPResourceNotificationHandler(MCPServer *pServer, QObject *pParent)
    : MCPNotificationHandlerBase(pServer, pParent)
{}

MCPResourceNotificationHandler::~MCPResourceNotificationHandler() {}

void MCPResourceNotificationHandler::onResourceContentChanged(const QString &strUri)
{
    MCP_CORE_LOG_INFO() << "MCPResourceNotificationHandler: Resource content changed, notify subscribers:" << strUri;

    // Get all session IDs subscribed to this URI
    auto pResourceService = m_pServer->getResourceService();
    QSet<QString> lstSubscribedSessionIds = pResourceService->getSubscribedSessionIds(strUri);
    if (lstSubscribedSessionIds.isEmpty()) {
        MCP_CORE_LOG_DEBUG() << "MCPResourceNotificationHandler: URI has no subscribers:" << strUri;
        return;
    }

    // Read resource content and metadata
    QJsonObject resourceInfo = pResourceService->readResource(strUri);
    // Get resource metadata (name, description, mimeType)
    auto pResource = pResourceService->getResource(strUri);
    QJsonObject metadata = pResource->getMetadata();
    resourceInfo["name"] = metadata["name"];
    resourceInfo["description"] = metadata["description"];
    resourceInfo["mimeType"] = metadata["mimeType"];

    // Build notification parameters
    QJsonObject params;
    params["uri"] = strUri;
    QJsonObject resourceData;
    resourceData["resource"] = resourceInfo;
    params["data"] = resourceData;

    // Send notification to subscribers
    // According to MCP protocol specification, the method name for resource update notifications is "notifications/resources/updated"
    sendNotificationToSubscribers("notifications/resources/updated", params, lstSubscribedSessionIds);

    MCP_CORE_LOG_INFO() << "MCPResourceNotificationHandler: URI" << strUri << "'s content change notification has been processed, total" << lstSubscribedSessionIds.size() << "subscribers";
}

void MCPResourceNotificationHandler::onResourceDeleted(const QString &strUri)
{
    if (strUri.isEmpty()) {
        return;
    }

    auto pResourceService = m_pServer->getResourceService();

    MCP_CORE_LOG_INFO() << "MCPResourceNotificationHandler: Resource deleted, notify subscribers:" << strUri;

    // Get all session IDs subscribed to this URI
    QSet<QString> subscribedSessionIds = pResourceService->getSubscribedSessionIds(strUri);
    if (subscribedSessionIds.isEmpty()) {
        MCP_CORE_LOG_DEBUG() << "MCPResourceNotificationHandler: URI has no subscribers:" << strUri;
        return;
    }

    // Build deletion notification parameters
    QJsonObject params;
    params["uri"] = strUri;
    QJsonObject resourceData;
    resourceData["deleted"] = true;
    params["data"] = resourceData;

    // Send notification to subscribers
    // According to MCP protocol specification, the method name for resource update notifications is "notifications/resources/updated"
    sendNotificationToSubscribers("notifications/resources/updated", params, subscribedSessionIds);

    MCP_CORE_LOG_INFO() << "MCPResourceNotificationHandler: URI" << strUri << "'s deletion notification has been processed, total" << subscribedSessionIds.size() << "subscribers";
}

void MCPResourceNotificationHandler::onResourcesListChanged()
{
    auto pResourceService = m_pServer->getResourceService();

    MCP_CORE_LOG_INFO() << "MCPResourceNotificationHandler: Resource list changed, send notification to all clients";

    // Get the latest resource list
    QJsonArray arrResources = pResourceService->list();

    // Build notification parameters
    QJsonObject params;
    params["resources"] = arrResources;

    // Broadcast notification
    broadcastNotification("notifications/resources/list_changed", params);

    MCP_CORE_LOG_INFO() << "MCPResourceNotificationHandler: Resource list change notification processing completed";
}
