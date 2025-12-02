/**
 * @file MCPHandlerResolver.h
 * @brief MCP Handler Resolver
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <QObject>
#include <QString>
#include <QMap>

/**
 * @brief MCP Handler Resolver
 * 
 * Responsibilities:
 * - Find Handler objects from the application
 * - Build a mapping table from Handler name to object
 * - Support multiple Handler identification methods
 * 
 * Usage example:
 * @code
 * QMap<QString, QObject*> handlers = MCPHandlerResolver::resolveHandlers();
 * QObject* pHandler = handlers.value("MyHandler");
 * @endcode
 * 
 * Coding conventions:
 * - Static utility class
 * - String types add str prefix
 * - Pointer types add p prefix
 * - { and } should be on separate lines
 */
class MCPHandlerResolver
{
public:
    /**
     * @brief Find all Handler objects from the application
     * @param pSearchRoot Search root object, default is qApp
     * @return Mapping table from Handler name to object
     * 
     * Supported identification methods:
     * 1. Object's objectName
     * 2. Object's "MPCToolHandlerName" property (for tool Handlers)
     * 3. Object's "MCPResourceHandlerName" property (for resource Handlers)
     */
    static QMap<QString, QObject*> resolveHandlers(QObject* pSearchRoot = nullptr);
    
    /**
     * @brief Find all resource Handler objects from the application
     * @param pSearchRoot Search root object, default is qApp
     * @return Mapping table from resource Handler name to object
     * 
     * Supported identification methods:
     * 1. Object's objectName
     * 2. Object's "MCPResourceHandlerName" property
     */
    static QMap<QString, QObject*> resolveResourceHandlers(QObject* pSearchRoot = nullptr);
    
    /**
     * @brief Find Handler with specified name
     * @param strHandlerName Handler name
     * @param pSearchRoot Search root object, default is qApp
     * @return Handler object pointer, return nullptr if not found
     */
    static QObject* findHandler(const QString& strHandlerName, QObject* pSearchRoot = nullptr);
    
    /**
     * @brief Find all default Handler objects from the application
     * @return Mapping table from Handler name to object
     * 
     * Scope for traversal:
     * 1. All child objects of qApp
     * 2. All QWidget objects
     * 
     * Supported identification methods:
     * 1. Object's objectName
     * 2. Object's "MPCToolHandlerName" property (for tool Handlers)
     * 3. Object's "MCPResourceHandlerName" property (for resource Handlers)
     */
    static QMap<QString, QObject*> resolveDefaultHandlers();
};
