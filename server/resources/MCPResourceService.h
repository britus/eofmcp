/**
 * @file MCPResourceService.h
 * @brief MCP resource service (internal implementation)
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include "IMCPResourceService.h"

class MCPResource;
struct MCPResourceConfig;

/**
 * @brief MCP resource service implementation class
 * 
 * Responsibilities:
 * - Resource registration and management
 * - Resource reading operations
 * - Resource list provision
 * - Resource subscription management (only resources that support subscription)
 */
class MCPResourceService : public IMCPResourceService
{
    Q_OBJECT

public:
    explicit MCPResourceService(QObject* pParent = nullptr);
    virtual ~MCPResourceService();

    // Implementation of IMCPResourceService interface
    bool add(const QString& strUri,
             const QString& strName,
             const QString& strDescription,
             const QString& strMimeType,
             std::function<QString()> contentProvider) override;
    
    bool add(const QString& strUri,
             const QString& strName,
             const QString& strDescription,
             const QString& strFilePath,
             const QString& strMimeType = QString()) override;
    
    bool remove(const QString& strUri) override;
    bool has(const QString& strUri) const override;
    QJsonArray list(const QString& strUriPrefix = QString()) const override;
    QJsonObject readResource(const QString& strUri) override;
    
    bool addFromJson(const QJsonObject& jsonResource, QObject* pSearchRoot = nullptr) override;
    
public:
    // Internal methods (for internal use)
    bool registerResource(const QString& strUri, MCPResource* pResource);
    
    /**
     * @brief Subscribe to resource changes
     * @param strUri Resource URI
     * @param strSessionId Session ID
     * @return Whether subscription was successful
     */
    bool subscribe(const QString& strUri, const QString& strSessionId);
    
    /**
     * @brief Unsubscribe from resource changes
     * @param strUri Resource URI
     * @param strSessionId Session ID
     * @return Whether unsubscription was successful
     */
    bool unsubscribe(const QString& strUri, const QString& strSessionId);
    
    /**
     * @brief Unsubscribe all subscriptions for a session
     * @param strSessionId Session ID
     */
    void unsubscribeAll(const QString& strSessionId);
    
    /**
     * @brief Get all session IDs subscribed to a specific URI
     * @param strUri Resource URI
     * @return Set of session IDs subscribed to this URI
     */
    QSet<QString> getSubscribedSessionIds(const QString& strUri) const;
    
    /**
     * @brief Get resource object (internal method, for internal use)
     * @param strUri Resource URI
     * @return Pointer to resource object, returns nullptr if not found
     */
    MCPResource* getResource(const QString& strUri) const;

signals:
    /**
     * @brief Resource content changed signal (for subscription mechanism)
     * This signal is emitted when the following occurs:
     * - Resource registration
     * - Resource metadata changes (name, description, mimeType)
     * - Resource content changes (through notifyChanged())
     * 
     * Handled by ServerHandler to send notifications to clients subscribed to this URI
     */
    void resourceContentChanged(const QString& strUri);
    
    /**
     * @brief Resource deleted signal (for subscription mechanism)
     * This signal is emitted when the following occurs:
     * - Resource unregistration
     * - Resource invalidation (through notifyInvalidated())
     * 
     * Handled by ServerHandler to send deletion notifications to clients subscribed to this URI
     */
    void resourceDeleted(const QString& strUri);
    
    /**
     * @brief Resource list changed signal (for broadcast notifications)
     * This signal is emitted when resources are registered or unregistered, broadcasting to all clients
     * 
     * Note: Metadata changes do not trigger this signal, only resource additions and deletions trigger it
     */
    void resourcesListChanged();

private:
    /**
     * @brief Internal method: actually perform the add resource operation
     * @return Returns pointer to resource object on success, nullptr on failure
     */
    MCPResource* doAddImpl(const QString& strUri,
                           const QString& strName,
                           const QString& strDescription,
                           const QString& strMimeType,
                           std::function<QString()> contentProvider);
    
    /**
     * @brief Internal method: actually perform the add resource operation (from file path)
     * @return Returns pointer to resource object on success, nullptr on failure
     */
    MCPResource* doAddImpl(const QString& strUri,
                           const QString& strName,
                           const QString& strDescription,
                           const QString& strFilePath,
                           const QString& strMimeType);
    
    /**
     * @brief Internal method: actually perform the remove resource operation
     * @param strUri Resource URI
     * @param bEmitSignal Whether to emit signal, default is true
     */
    bool doRemoveImpl(const QString& strUri, bool bEmitSignal = true);
    
    /**
     * @brief Internal method: actually perform the check if resource exists operation
     */
    bool doHasImpl(const QString& strUri) const;
    
    /**
     * @brief Internal method: actually perform the get resource list operation
     */
    QJsonArray doListImpl(const QString& strUriPrefix) const;
    
    /**
     * @brief Internal method: actually perform the read resource content operation
     */
    QJsonObject doReadResourceImpl(const QString& strUri);
    
    /**
     * @brief Add file resource from configuration
     * @param resourceConfig Resource configuration object
     * @return true if registration successful, false if failed
     */
    bool addFileResourceFromConfig(const MCPResourceConfig& resourceConfig);
    
    /**
     * @brief Add wrapper resource from configuration
     * @param resourceConfig Resource configuration object
     * @param dictHandlers Mapping of handler names to objects, if empty search from qApp
     * @return true if registration successful, false if failed
     */
    bool addWrapperResourceFromConfig(const MCPResourceConfig& resourceConfig, const QMap<QString, QObject*>& dictHandlers = QMap<QString, QObject*>());
    
    /**
     * @brief Add content resource from configuration
     * @param resourceConfig Resource configuration object
     * @return true if registration successful, false if failed
     */
    bool addContentResourceFromConfig(const MCPResourceConfig& resourceConfig);
    
    /**
     * @brief Apply annotations to resource if configuration contains annotations
     * @param pResource Pointer to resource object
     * @param annotations Annotation object
     * @return true if successful, false if failed
     */
    bool applyAnnotationsIfNeeded(MCPResource* pResource, const QJsonObject& annotations);
    
    /**
     * @brief Add resource from configuration object (internal method, for MCPServer use)
     * @param resourceConfig Resource configuration object
     * @param dictHandlers Mapping of handler names to objects, if empty search from qApp
     * @return true if registration successful, false if failed
     * 
     * @warning Deadlock risk: When calling this method to add resources during service operation, if the added resource is of wrapper type,
     *          and the caller and wrapped object are in the same thread, deadlock may occur.
     *          This is because creating MCPResourceWrapper calls updatePropertiesFromWrappedObject() in the constructor,
     *          which in turn calls getMetadata() on the wrapped object. If the wrapped object is waiting for some operation to complete,
     *          a deadlock will occur. It's recommended to call this method during service initialization, avoiding dynamic addition of wrapper-type resources during runtime.
     */
    bool addFromConfig(const MCPResourceConfig& resourceConfig, const QMap<QString, QObject*>& dictHandlers = QMap<QString, QObject*>());

private:
    QMap<QString, MCPResource*> m_dictResources;
    
    // Subscription management (based on sessionId)
    QMap<QString, QSet<QString>> m_subscriptions;  // URI -> Set of session IDs
    QMap<QString, QSet<QString>> m_sessionSubscriptions;  // Session ID -> Set of URIs
private:
    friend class MCPServer;
};


