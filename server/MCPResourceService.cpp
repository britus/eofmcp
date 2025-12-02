/**
 * @file MCPResourceService.cpp
 * @brief MCP resource service implementation
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPResourceService.h"
#include "MCPContentResource.h"
#include "MCPFileResource.h"
#include "MCPHandlerResolver.h"
#include "MCPInvokeHelper.h"
#include "MCPLog.h"
#include "MCPResource.h"
#include "MCPResourceContentGenerator.h"
#include "MCPResourceWrapper.h"
#include "MCPResourcesConfig.h"
#include <QSet>

MCPResourceService::MCPResourceService(QObject *pParent)
    : IMCPResourceService(pParent)
{}

MCPResourceService::~MCPResourceService()
{
    // Clean up all resources
    foreach (MCPResource *pResource, m_dictResources) {
        if (pResource) {
            pResource->deleteLater();
        }
    }
    m_dictResources.clear();
    m_subscriptions.clear();
    m_sessionSubscriptions.clear();
}

bool MCPResourceService::add(const QString &strUri, const QString &strName, const QString &strDescription, const QString &strMimeType, std::function<QString()> contentProvider)
{
    return MCPInvokeHelper::syncInvokeReturn(this, [this, strUri, strName, strDescription, strMimeType, contentProvider]() {
        return doAddImpl(strUri, strName, strDescription, strMimeType, contentProvider) != nullptr;
    });
}

bool MCPResourceService::add(const QString &strUri, const QString &strName, const QString &strDescription, const QString &strFilePath, const QString &strMimeType)
{
    return MCPInvokeHelper::syncInvokeReturn(this, [this, strUri, strName, strDescription, strFilePath, strMimeType]() {
        return doAddImpl(strUri, strName, strDescription, strFilePath, strMimeType) != nullptr;
    });
}

bool MCPResourceService::registerResource(const QString &strUri, MCPResource *pResource)
{
    // If already exists, remove old one (overwrite), don't emit signal because new object will emit later
    if (m_dictResources.contains(strUri)) {
        MCP_CORE_LOG_INFO() << "MCPResourceService: Resource already exists, overwriting old resource:" << strUri;
        doRemoveImpl(strUri, false);
    }

    m_dictResources[strUri] = pResource;
    MCP_CORE_LOG_INFO() << "MCPResourceService: Resource registered:" << strUri;

    // Connect resource's changed signal to resourceContentChanged(QString) signal
    // This notifies subscribers when resource metadata (name, description, mimeType) or content changes
    QObject::connect(pResource, &MCPResource::changed, this, [this, strUri](const QString &, const QString &, const QString &) { emit resourceContentChanged(strUri); });

    // Connect resource's invalidated signal to resourceDeleted(QString) signal
    // This notifies subscribers when resource is invalidated (deleted)
    QObject::connect(pResource, &MCPResource::invalidated, this, [this, strUri]() {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Resource invalidated:" << strUri;
        emit resourceDeleted(strUri); // Notify subscribers that resource is deleted
    });

    emit resourceContentChanged(strUri); // Notify subscribers (subscription mechanism) - Resource registration
    emit resourcesListChanged();         // Notify all clients (broadcast notification)
    return true;
}

bool MCPResourceService::remove(const QString &strUri)
{
    return MCPInvokeHelper::syncInvokeReturn(this, [this, strUri]() { return doRemoveImpl(strUri); });
}

bool MCPResourceService::has(const QString &strUri) const
{
    return MCPInvokeHelper::syncInvokeReturn(const_cast<MCPResourceService *>(this), [this, strUri]() { return doHasImpl(strUri); });
}

QJsonArray MCPResourceService::list(const QString &strUriPrefix) const
{
    QJsonArray arrResult;
    MCPInvokeHelper::syncInvoke(const_cast<MCPResourceService *>(this), [this, &arrResult, strUriPrefix]() { arrResult = doListImpl(strUriPrefix); });
    return arrResult;
}

QJsonObject MCPResourceService::readResource(const QString &strUri)
{
    QJsonObject objResult;
    MCPInvokeHelper::syncInvoke(this, [this, &objResult, strUri]() { objResult = doReadResourceImpl(strUri); });
    return objResult;
}

bool MCPResourceService::addFromJson(const QJsonObject &jsonResource, QObject *pSearchRoot)
{
    return MCPInvokeHelper::syncInvokeReturn(this, [this, jsonResource, pSearchRoot]() {
        // Convert JSON object to MCPResourceConfig
        MCPResourceConfig resourceConfig = MCPResourceConfig::fromJson(jsonResource);

        return addFromConfig(resourceConfig, MCPHandlerResolver::resolveResourceHandlers(pSearchRoot));
    });
}

bool MCPResourceService::addFromConfig(const MCPResourceConfig &resourceConfig, const QMap<QString, QObject *> &dictHandlers)
{
    // Call corresponding handler based on resource type
    if (resourceConfig.strType == "file") {
        return addFileResourceFromConfig(resourceConfig);
    } else if (resourceConfig.strType == "wrapper") {
        return addWrapperResourceFromConfig(resourceConfig, dictHandlers);
    }
    return addContentResourceFromConfig(resourceConfig);
}

bool MCPResourceService::addFileResourceFromConfig(const MCPResourceConfig &resourceConfig)
{
    // Validate file path
    if (resourceConfig.strFilePath.isEmpty()) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: File resource configuration invalid (missing filePath):" << resourceConfig.strUri;
        return false;
    }

    // Use file path method (supports automatic MIME type inference and binary files)
    MCPResource *pResource = doAddImpl( //
        resourceConfig.strUri,
        resourceConfig.strName,
        resourceConfig.strDescription,
        resourceConfig.strFilePath,
        resourceConfig.strMimeType.isEmpty() ? QString() : resourceConfig.strMimeType);

    // Apply annotations (if any)
    if (pResource != nullptr) {
        applyAnnotationsIfNeeded(pResource, resourceConfig.annotations);
    }

    return pResource != nullptr;
}

bool MCPResourceService::addWrapperResourceFromConfig(const MCPResourceConfig &resourceConfig, const QMap<QString, QObject *> &dictHandlers)
{
    // Validate Handler name
    if (resourceConfig.strHandlerName.isEmpty()) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Wrapper resource configuration invalid (missing handlerName):" << resourceConfig.strUri;
        return false;
    }

    // Find Handler from dictHandlers mapping table, if dictHandlers is empty search from qApp
    QObject *pHandler = dictHandlers.value(resourceConfig.strHandlerName, nullptr);
    if (pHandler == nullptr) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Resource Handler not found:" << resourceConfig.strHandlerName << ", Resource URI:" << resourceConfig.strUri;
        return false;
    }

    // Create MCPResourceWrapper
    MCPResourceWrapper *pWrapper = MCPResourceWrapper::create(resourceConfig.strUri, pHandler, this);

    if (pWrapper == nullptr) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Failed to create resource wrapper:" << resourceConfig.strUri << ", Handler:" << resourceConfig.strHandlerName;
        return false;
    }

    // Set resource metadata
    pWrapper->setName(resourceConfig.strName);
    pWrapper->setDescription(resourceConfig.strDescription);
    if (!resourceConfig.strMimeType.isEmpty()) {
        pWrapper->setMimeType(resourceConfig.strMimeType);
    }

    // Register resource
    bool bSuccess = registerResource(resourceConfig.strUri, pWrapper);

    // Apply annotations (if any)
    if (bSuccess) {
        applyAnnotationsIfNeeded(pWrapper, resourceConfig.annotations);
    }

    return bSuccess;
}

bool MCPResourceService::addContentResourceFromConfig(const MCPResourceConfig &resourceConfig)
{
    // Validate content
    if (resourceConfig.strContent.isEmpty()) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Content resource configuration invalid (missing content):" << resourceConfig.strUri;
        return false;
    }

    // Use static content method
    QString strContent = resourceConfig.strContent;
    std::function<QString()> contentProvider = [strContent]() {
        return strContent;
    };

    MCPResource *pResource = doAddImpl( //
        resourceConfig.strUri,
        resourceConfig.strName,
        resourceConfig.strDescription,
        resourceConfig.strMimeType.isEmpty() ? "text/plain" : resourceConfig.strMimeType,
        contentProvider);

    // Apply annotations (if any)
    if (pResource != nullptr) {
        applyAnnotationsIfNeeded(pResource, resourceConfig.annotations);
    }

    return pResource != nullptr;
}

bool MCPResourceService::applyAnnotationsIfNeeded(MCPResource *pResource, const QJsonObject &annotations)
{
    if (annotations.isEmpty()) {
        return true; // No annotations, return success directly
    }

    if (pResource == nullptr) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Cannot apply annotations, resource object is null";
        return false;
    }

    pResource->setAnnotations(annotations);
    return true;
}

MCPResource *MCPResourceService::doAddImpl(const QString &strUri, const QString &strName, const QString &strDescription, const QString &strMimeType, std::function<QString()> contentProvider)
{
    // Create resource object (set parent to this)
    MCPContentResource *pResource = new MCPContentResource(strUri, this);
    pResource->withName(strName)->withDescription(strDescription)->withMimeType(strMimeType)->withContentProvider(contentProvider);

    // Register to service
    if (!registerResource(strUri, pResource)) {
        // If registration fails, delete Resource object to avoid memory leak
        pResource->deleteLater();
        return nullptr;
    }

    return pResource;
}

MCPResource *MCPResourceService::doAddImpl(const QString &strUri, const QString &strName, const QString &strDescription, const QString &strFilePath, const QString &strMimeType)
{
    // Create file resource object (set parent to this)
    MCPFileResource *pResource = new MCPFileResource(strUri, strFilePath, strName, this);
    pResource->withDescription(strDescription);

    // If MIME type provided, set it (otherwise MCPFileResource will infer automatically)
    if (!strMimeType.isEmpty()) {
        pResource->withMimeType(strMimeType);
    }

    // Register to service
    if (!registerResource(strUri, pResource)) {
        // If registration fails, delete Resource object to avoid memory leak
        pResource->deleteLater();
        return nullptr;
    }

    return pResource;
}

bool MCPResourceService::doRemoveImpl(const QString &strUri, bool bEmitSignal)
{
    if (!m_dictResources.contains(strUri)) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Resource does not exist:" << strUri;
        return false;
    }

    MCPResource *pResource = m_dictResources.take(strUri);
    if (pResource) {
        pResource->deleteLater();
    }

    MCP_CORE_LOG_INFO() << "MCPResourceService: Resource unregistered:" << strUri;
    if (bEmitSignal) {
        emit resourceDeleted(strUri); // Notify subscribers that resource is deleted (subscription mechanism)
        emit resourcesListChanged();  // Notify all clients (broadcast notification)
    }
    return true;
}

bool MCPResourceService::doHasImpl(const QString &strUri) const
{
    return m_dictResources.contains(strUri);
}

QJsonArray MCPResourceService::doListImpl(const QString &strUriPrefix) const
{
    QJsonArray arrResources;

    for (auto it = m_dictResources.constBegin(); it != m_dictResources.constEnd(); ++it) {
        const QString &strUri = it.key();
        MCPResource *pResource = it.value();

        // If prefix specified, filter
        if (!strUriPrefix.isEmpty() && !strUri.startsWith(strUriPrefix)) {
            continue;
        }

        // Build resource metadata (uses getMetadata() method, automatically includes annotations)
        QJsonObject metadata = pResource->getMetadata();
        metadata["uri"] = strUri; // URI needs to be added separately, getMetadata() doesn't include URI

        arrResources.append(metadata);
    }

    return arrResources;
}

QJsonObject MCPResourceService::doReadResourceImpl(const QString &strUri)
{
    if (!m_dictResources.contains(strUri)) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Attempting to read non-existent resource:" << strUri;
        return QJsonObject();
    }

    MCPResource *pResource = m_dictResources[strUri];
    QString strContent = pResource->readContent();
    QString strMimeType = pResource->getMimeType();

    // According to MCP protocol specification, resource response format:
    // {
    //   "contents": [
    //     {
    //       "uri": "...",
    //       "mimeType": "...",
    //       "text": "..." or "blob": "..."
    //     }
    //   ]
    // }
    QJsonObject result;
    QJsonArray contents;
    QJsonObject contentObj;

    contentObj["uri"] = strUri;

    // If MIME type is not empty, add to content object
    if (!strMimeType.isEmpty()) {
        contentObj["mimeType"] = strMimeType;
    }

    // Determine whether to use text or blob field based on MIME type
    if (MCPResourceContentGenerator::isTextMimeType(strMimeType)) {
        // Text type, use text field
        contentObj["text"] = strContent;
    } else {
        // Binary type, use blob field (Base64 encoded)
        // Note: readContent() for binary resources already returns Base64 encoded string
        contentObj["blob"] = strContent;
    }

    contents.append(contentObj);
    result["contents"] = contents;

    return result;
}

bool MCPResourceService::subscribe(const QString &strUri, const QString &strSessionId)
{
    if (strUri.isEmpty()) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Subscription failed, URI is empty";
        return false;
    }

    if (strSessionId.isEmpty()) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Subscription failed, session ID is empty";
        return false;
    }

    // Check if already subscribed
    if (m_subscriptions.contains(strUri)) {
        QSet<QString> &sessionIds = m_subscriptions[strUri];
        if (sessionIds.contains(strSessionId)) {
            MCP_CORE_LOG_DEBUG() << "MCPResourceService: Session" << strSessionId << "already subscribed to URI:" << strUri;
            return true; // Already subscribed, return success
        }
    }

    // Add to subscription mapping
    m_subscriptions[strUri].insert(strSessionId);
    m_sessionSubscriptions[strSessionId].insert(strUri);

    MCP_CORE_LOG_INFO() << "MCPResourceService: Session" << strSessionId << "subscribed to URI:" << strUri;
    return true;
}

bool MCPResourceService::unsubscribe(const QString &strUri, const QString &strSessionId)
{
    if (strUri.isEmpty()) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Unsubscription failed, URI is empty";
        return false;
    }

    if (strSessionId.isEmpty()) {
        MCP_CORE_LOG_WARNING() << "MCPResourceService: Unsubscription failed, session ID is empty";
        return false;
    }

    // Remove from URI subscription mapping
    if (m_subscriptions.contains(strUri)) {
        QSet<QString> &sessionIds = m_subscriptions[strUri];
        if (sessionIds.contains(strSessionId)) {
            sessionIds.remove(strSessionId);
            if (sessionIds.isEmpty()) {
                // If no more subscribers, remove the URI mapping
                m_subscriptions.remove(strUri);
            }
        } else {
            MCP_CORE_LOG_DEBUG() << "MCPResourceService: Session" << strSessionId << "not subscribed to URI:" << strUri;
            return false;
        }
    } else {
        MCP_CORE_LOG_DEBUG() << "MCPResourceService: URI has no subscription:" << strUri;
        return false;
    }

    // Remove from session subscription mapping
    if (m_sessionSubscriptions.contains(strSessionId)) {
        QSet<QString> &uris = m_sessionSubscriptions[strSessionId];
        uris.remove(strUri);
        if (uris.isEmpty()) {
            // If session has no subscriptions, remove the session mapping
            m_sessionSubscriptions.remove(strSessionId);
        }
    }

    MCP_CORE_LOG_INFO() << "MCPResourceService: Session" << strSessionId << "unsubscribed from URI:" << strUri;
    return true;
}

void MCPResourceService::unsubscribeAll(const QString &strSessionId)
{
    if (strSessionId.isEmpty()) {
        MCP_CORE_LOG_DEBUG() << "MCPResourceService: Session ID is empty";
        return;
    }

    if (!m_sessionSubscriptions.contains(strSessionId)) {
        MCP_CORE_LOG_DEBUG() << "MCPResourceService: Session" << strSessionId << "has no subscription records";
        return;
    }

    // Get all subscribed URIs for this session
    QSet<QString> uris = m_sessionSubscriptions[strSessionId];

    // Iterate through all URIs, remove this session from subscription mapping
    for (const QString &strUri : uris) {
        if (m_subscriptions.contains(strUri)) {
            QSet<QString> &sessionIds = m_subscriptions[strUri];
            sessionIds.remove(strSessionId);

            if (sessionIds.isEmpty()) {
                // If no more subscribers, remove the URI mapping
                m_subscriptions.remove(strUri);
            }
        }
    }

    // Remove session's subscription mapping
    m_sessionSubscriptions.remove(strSessionId);

    MCP_CORE_LOG_INFO() << "MCPResourceService: All subscriptions for session" << strSessionId << "canceled, total of" << uris.size() << "subscriptions";
}

QSet<QString> MCPResourceService::getSubscribedSessionIds(const QString &strUri) const
{
    if (strUri.isEmpty()) {
        return QSet<QString>();
    }

    if (!m_subscriptions.contains(strUri)) {
        return QSet<QString>();
    }

    return m_subscriptions[strUri];
}

MCPResource *MCPResourceService::getResource(const QString &strUri) const
{
    if (strUri.isEmpty()) {
        return nullptr;
    }

    if (!m_dictResources.contains(strUri)) {
        return nullptr;
    }

    return m_dictResources[strUri];
}
