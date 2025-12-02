/**
 * @file MCPHelper.h
 * @brief MCP Unified Helper Class
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include "MCPCore_global.h.h"
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <functional>

/**
 * @brief MCP Unified Helper Class
 * 
 * Responsibilities:
 * - Provide thread-safe method invocation
 * - Provide reflection-based method invocation assistance
 * - Encapsulate internal implementation details
 * 
 * Coding conventions:
 * - Static methods, no instantiation required
 * - Parameter naming follows project conventions
 * - { and } should be on separate lines
 */
class MCPCORE_EXPORT MCPHelper
{
public:
    /**
     * @brief Synchronously invoke a method on the target object (thread-safe)
     * @param pTargetObj Target object
     * @param fun Function to execute
     * @return true on success, false on failure
     */
    static void syncInvoke(QObject* pTargetObj, const std::function<void()>& fun);
    
    /**
     * @brief Asynchronously invoke a method on the target object (thread-safe)
     * @param pTargetObj Target object
     * @param fun Function to execute
     * @return true on success, false on failure
     */
    static void asyncInvoke(QObject* pTargetObj, const std::function<void()>& fun);
    
    /**
     * @brief Synchronously invoke an object's method (via reflection)
     * @param pHandler Handler object
     * @param strMethodName Method name
     * @param lstArguments Argument list
     * @return Method return value
     */
    static QVariant callMethod(QObject* pHandler, const QString& strMethodName, const QVariantList& lstArguments);
    
    /**
     * @brief Synchronously invoke an object's method (via reflection, using Map parameters)
     * @param pHandler Handler object
     * @param strMethodName Method name
     * @param dictArguments Parameter dictionary
     * @return Method return value
     */
    static QVariant callMethod(QObject* pHandler, const QString& strMethodName, const QVariantMap& dictArguments);
    
    /**
     * @brief Set thread name
     * @param dwThreadID Thread ID
     * @param strThreadName Thread name
     * @return true on success, false on failure
     */
    static void setThreadName(unsigned long dwThreadID, const QString& strThreadName);
    
    /**
     * @brief Set current thread name
     * @param strThreadName Thread name
     * @return true on success, false on failure
     */
    static void setCurrentThreadName(const QString& strThreadName);
};
