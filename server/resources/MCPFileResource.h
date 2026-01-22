/**
 * @file MCPFileResource.h
 * @brief MCP file resource class (inherits from MCPContentResource)
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include "MCPContentResource.h"

/**
 * @brief MCP file resource class
 * 
 * Responsibilities:
 * - Implement resource content reading based on file path
 * - Support text and binary files
 * - Automatically infer MIME type
 * 
 * Note:
 * - Inherits from MCPContentResource, reuses fluent interface (withName, withDescription, withMimeType)
 * - Implements file reading logic through contentProvider
 * 
 * Coding standards:
 * - Class members add m_ prefix
 * - String types add str prefix
 * - { and } should be on separate lines
 */
class MCPFileResource : public MCPContentResource
{
    Q_OBJECT
    
public:
    /**
     * @brief Constructor
     * @param strUri Resource URI
     * @param strFilePath File path
     * @param strName Resource name (optional, if not provided use filename)
     * @param pParent Parent object
     */
    explicit MCPFileResource(const QString& strUri,
                             const QString& strFilePath,
                             const QString& strName = QString(),
                             QObject* pParent = nullptr);
    
    virtual ~MCPFileResource();
    
    /**
     * @brief Get file path
     * @return File path
     */
    QString getFilePath() const;
    
private:
    /**
     * @brief Infer MIME type based on file extension
     */
    void inferMimeType();
    
    /**
     * @brief Create file content provider function
     * @return Content provider function
     */
    std::function<QString()> createFileContentProvider() const;
    
private:
    QString m_strFilePath;  // File path
};