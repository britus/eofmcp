/**
 * @file IMCPResourceService.h
 * @brief MCP resource service interface
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include "MCPCore_global.h.h"
#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

/**
 * @brief MCP resource service interface
 *
 * Responsibilities:
 * - Define public interface of resource service
 * - Provide resource registration and management functions
 * - Hide specific implementation details
 *
 * Coding standards:
 * - Use pure virtual functions for interface methods
 * - { and } should be on separate lines
 *
 * @note Deadlock risk explanation: In service runtime, when calling addFromJson and other methods to add wrapper type resources,
 *       if the caller and wrapper object are in the same thread, it may cause a deadlock. This issue is currently not handled,
 *       and it's recommended to complete resource addition operations during service initialization.
 */
class MCPCORE_EXPORT IMCPResourceService : public QObject
{
    Q_OBJECT

public:
    explicit IMCPResourceService(QObject* pParent = nullptr) : QObject(pParent) {}

protected:
    /**
     * @brief Destructor (protected, Service objects are managed by Server, don't delete directly)
     */
    virtual ~IMCPResourceService() {}

public:
    /**
     * @brief Register resource
     * @param strUri Resource URI
     * @param strName Resource name
     * @param strDescription Resource description
     * @param strMimeType MIME type
     * @param contentProvider Content provider function
     * @return true if registration successful, false if failed
     */
    virtual bool add(const QString& strUri,
                     const QString& strName,
                     const QString& strDescription,
                     const QString& strMimeType,
                     std::function<QString()> contentProvider) = 0;

    /**
     * @brief Register resource (from file path)
     * @param strUri Resource URI
     * @param strName Resource name
     * @param strDescription Resource description
     * @param strFilePath File path
     * @param strMimeType MIME type (optional, if not provided it will be inferred from file extension)
     * @return true if registration successful, false if failed
     *
     * Usage example:
     * @code
     * auto pResourceService = pServer->getResourceService();
     * // Method 1: Auto-infer MIME type
     * pResourceService->add("file:///path/to/file.txt", "My File", "A text file", "/path/to/file.txt");
     *
     * // Method 2: Specify MIME type
     * pResourceService->add("file:///path/to/image.png", "My Image", "An image", "/path/to/image.png", "image/png");
     * @endcode
     */
    virtual bool add(const QString& strUri,
                     const QString& strName,
                     const QString& strDescription,
                     const QString& strFilePath,
                     const QString& strMimeType = QString()) = 0;

    /**
     * @brief Unregister resource
     * @param strUri Resource URI
     * @return true if unregistration successful, false if failed
     */
    virtual bool remove(const QString& strUri) = 0;

    /**
     * @brief Check if resource exists
     * @param strUri Resource URI
     * @return true if exists, false if not exists
     */
    virtual bool has(const QString& strUri) const = 0;

    /**
     * @brief Get resource list
     * @param strUriPrefix URI prefix (optional, for filtering)
     * @return Resource list (JSON array format)
     */
    virtual QJsonArray list(const QString& strUriPrefix = QString()) const = 0;

    /**
     * @brief Read resource content
     * @param strUri Resource URI
     * @return Resource content (JSON object format)
     */
    virtual QJsonObject readResource(const QString& strUri) = 0;

    /**
     * @brief Add resource from JSON object
     * @param jsonResource JSON object containing configuration information of the resource
     * @param pSearchRoot Search root object for Handler (defaults to nullptr, use qApp)
     * @return true if registration successful, false if failed
     *
     * JSON object format:
     * {
     *   "uri": "Resource URI",
     *   "name": "Resource name",
     *   "description": "Resource description",
     *   "mimeType": "MIME type (optional, default text/plain)",
     *   "type": "Resource type: file/wrapper/content (optional, default content)",
     *   "filePath": "File path (required for file type)",
     *   "content": "Static content (required for content type)",
     *   "handlerName": "Handler name (required for wrapper type)",
     *   "annotations": { ... } (optional)
     * }
     *
     * Usage example:
     * @code
     * QJsonObject json;
     * json["uri"] = "file:///path/to/file";
     * json["name"] = "My File";
     * json["type"] = "file";
     * json["filePath"] = "/path/to/file.txt";
     * pResourceService->addFromJson(json);
     * @endcode
     */
    virtual bool addFromJson(const QJsonObject& jsonResource, QObject* pSearchRoot = nullptr) = 0;

signals:
    /**
     * @brief Resources list changed signal
     * Emitted when resources are registered or unregistered
     */
    void resourcesListChanged();
};

//#define IMCPResourceService_iid "org.eof.IMCPResourceService"
//Q_DECLARE_INTERFACE(IMCPResourceService, IMCPResourceService_iid)
