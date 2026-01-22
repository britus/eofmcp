/**
 * @file MCPContentResource.h
 * @brief MCP content resource class (inherits from MCPResource base class)
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include "MCPResource.h"
#include <functional>

/**
 * @brief MCP content resource class
 * 
 * Responsibilities:
 * - Implement resource content reading logic
 * - Support content provider function approach
 * - Support text and binary resources
 * 
 * Note:
 * - Use MCPFileResource class for file-based resources
 * 
 * Coding standards:
 * - Class members add m_ prefix
 * - String types add str prefix
 * - { and } should be on separate lines
 */
class MCPContentResource : public MCPResource
{
    Q_OBJECT
    
public:
    explicit MCPContentResource(const QString& strUri, 
                                QObject* pParent = nullptr);
    virtual ~MCPContentResource();
    
public:
    /**
     * @brief Set resource name (fluent interface)
     * @param strName Resource name
     * @return Returns this pointer, supports fluent interface
     */
    MCPContentResource* withName(const QString& strName);
    
    /**
     * @brief Set resource description (fluent interface)
     * @param strDescription Resource description
     * @return Returns this pointer, supports fluent interface
     */
    MCPContentResource* withDescription(const QString& strDescription);
    
    /**
     * @brief Set MIME type (fluent interface)
     * @param strMimeType MIME type
     * @return Returns this pointer, supports fluent interface
     */
    MCPContentResource* withMimeType(const QString& strMimeType);
    
    /**
     * @brief Set content provider function (fluent interface)
     * @param contentProvider Content provider function
     * @return Returns this pointer, supports fluent interface
     */
    MCPContentResource* withContentProvider(std::function<QString()> contentProvider);
    
public:
    /**
     * @brief Read resource content (implements pure virtual function from base class)
     * @return Resource content string
     */
    QString readContent() const override;
    
private:
    std::function<QString()> m_contentProvider;
};