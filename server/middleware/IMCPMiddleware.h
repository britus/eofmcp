/**
 * @file IMCPMiddleware.h
 * @brief MCP middleware interface
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <MCPServer_global.h>
#include <functional>
#include <QSharedPointer>

class MCPContext;
class MCPServerMessage;

/**
 * @brief MCP middleware interface
 *
 * Responsibilities:
 * - Define common behavior of middleware
 * - Support interception before and after request processing
 * - Implement cross-cutting concerns (logging, monitoring, validation, etc.)
 *
 * Design pattern:
 * - Chain of Responsibility pattern
 * - Middleware pattern
 *
 * Usage instructions:
 * - Call next() in process method to continue to the next middleware
 * - Add processing logic before and after next()
 * - Return nullptr to continue execution, return non-null to use this response
 *
 * Coding standards:
 * - Add p prefix to pointer types
 * - { and } should be on separate lines
 */
class IMCPMiddleware
{
public:
    virtual ~IMCPMiddleware() {}

    /**
     * @brief Process request
     * @param pContext Request context
     * @param next Next middleware/processor function
     * @return Response message. Return nullptr to continue execution, return non-null to use this response
     *
     * Usage example:
     * @code
     * QSharedPointer<MCPServerMessage> process(...)
     * {
     *     // Pre-processing
     *     qDebug() << "Before processing";
     *
     *     // Call next middleware
     *     auto pResponse = next();
     *
     *     // Post-processing
     *     qDebug() << "After processing";
     *
     *     return pResponse;
     * }
     * @endcode
     */
    virtual QSharedPointer<MCPServerMessage> process(const QSharedPointer<MCPContext> &pContext, std::function<QSharedPointer<MCPServerMessage>()> next) = 0;
};
