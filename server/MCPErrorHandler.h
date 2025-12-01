#pragma once

/**
 * @file MCPErrorHandler.h
 * @brief MCP error handling utility class
 * @author zhangheng
 * @date 2025-01-08
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <QObject>
#include <QJsonObject>
#include <MCPError.h>

/**
 * @brief MCP error handling utility class
 *
 * Provides simple error response generation and logging functionality.
 */
class MCPErrorHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     */
    explicit MCPErrorHandler(QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~MCPErrorHandler();

public:
    /**
     * @brief Create success response
     * @param result Result data
     * @param requestId Request ID
     * @return JSON-RPC 2.0 format success response
     */
    static QJsonObject createSuccessResponse(const QJsonValue& result, const QJsonValue& requestId = QJsonValue::Null);

    /**
     * @brief Log error to log
     * @param error Error object
     * @param context Error context information
     */
    static void logError(const MCPError& error, const QString& context = QString());
};

// Global error handler instance
MCPErrorHandler* getGlobalErrorHandler();