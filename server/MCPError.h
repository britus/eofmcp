#pragma once

/**
 * @file MCPError.h
 * @brief MCP error class definition
 * @author zhangheng
 * @date 2025-01-08
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QMetaType>
#include "MCPErrorCode.h"

/**
 * @brief MCP error class
 *
 * Encapsulate complete error information, including error code, message and additional data.
 * Supports JSON serialization and deserialization for network transmission.
 */
class MCPError
{
public:
    /**
     * @brief Default constructor
     */
    MCPError();

    /**
     * @brief Constructor
     * @param code Error code
     * @param message Error message (optional, default uses standard message)
     * @param data Additional error data (optional)
     */
    explicit MCPError(MCPErrorCode code,
                     const QString& message = QString(),
                     const QJsonValue& data = QJsonValue());

    /**
     * @brief Copy constructor
     */
    MCPError(const MCPError& other);

    /**
     * @brief Assignment operator
     */
    MCPError& operator=(const MCPError& other);

    /**
     * @brief Destructor
     */
    ~MCPError();

public:
    /**
     * @brief Get error code
     */
    MCPErrorCode getCode() const;

    /**
     * @brief Set error code
     */
    void setCode(MCPErrorCode code);

    /**
     * @brief Get error message
     */
    QString getMessage() const;

    /**
     * @brief Set error message
     */
    void setMessage(const QString& message);

    /**
     * @brief Get additional data
     */
    QJsonValue getData() const;

    /**
     * @brief Set additional data
     */
    void setData(const QJsonValue& data);

    /**
     * @brief Check if it's a server error
     */
    bool isServerError() const;


public:
    /**
     * @brief Convert to JSON object
     * @return Error object conforming to JSON-RPC 2.0 specification
     */
    QJsonObject toJson() const;

    /**
     * @brief Create JSON-RPC error response
     * @param requestId Request ID
     * @return JSON-RPC 2.0 format error response
     */
    QJsonObject toJsonResponse(const QJsonValue& requestId = QJsonValue::Null) const;

    /**
     * @brief Create error from JSON object
     * @param json JSON object
     * @return MCPError instance
     */
    static MCPError fromJson(const QJsonObject& json);


    /**
     * @brief Create standard error objects
     */
    static MCPError parseError(const QString& details = QString());
    static MCPError invalidRequest(const QString& details = QString());
    static MCPError methodNotFound(const QString& methodName);
    static MCPError invalidParams(const QString& details = QString());
    static MCPError internalError(const QString& details = QString());

    /**
     * @brief Create MCP specific error objects
     */
    static MCPError toolNotFound(const QString& toolName);
    static MCPError toolExecutionFailed(const QString& details = QString());
    static MCPError resourceNotFound(const QString& resourceUri);
    static MCPError sessionNotFound(const QString& sessionId);
    static MCPError authenticationFailed(const QString& details = QString());
    static MCPError authorizationFailed(const QString& details = QString());

private:
    MCPErrorCode m_code;        // Error code
    QString m_message;          // Error message
    QJsonValue m_data;          // Additional data
};

// Qt meta type declaration
Q_DECLARE_METATYPE(MCPError)