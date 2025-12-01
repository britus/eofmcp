/**
 * @file MCPError.cpp
 * @brief MCP error class implementation
 * @author zhangheng
 * @date 2025-01-08
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <MCPError.h>
#include <QJsonObject>

MCPError::MCPError()
    : m_code(MCPErrorCode::SUCCESS)
    , m_message(getErrorMessage(MCPErrorCode::SUCCESS))
    , m_data(QJsonValue::Null)
{

}


MCPError::MCPError(MCPErrorCode code, const QString& message, const QJsonValue& data)
    : m_code(code)
    , m_message(message.isEmpty() ? getErrorMessage(code) : message)
    , m_data(data)
{

}


MCPError::MCPError(const MCPError& other)
    : m_code(other.m_code)
    , m_message(other.m_message)
    , m_data(other.m_data)
{

}


MCPError& MCPError::operator=(const MCPError& other)
{
    if (this != &other)
    {
        m_code = other.m_code;
        m_message = other.m_message;
        m_data = other.m_data;
    }
    return *this;
}

MCPError::~MCPError()
{

}


MCPErrorCode MCPError::getCode() const
{

    return m_code;
}

void MCPError::setCode(MCPErrorCode code)
{

    m_code = code;
    // If message is empty, use default message
    if (m_message.isEmpty())
    {
        m_message = getErrorMessage(code);
    }
}

QString MCPError::getMessage() const
{

    return m_message;
}

void MCPError::setMessage(const QString& message)
{

    m_message = message;
}

QJsonValue MCPError::getData() const
{

    return m_data;
}

void MCPError::setData(const QJsonValue& data)
{

    m_data = data;
}

bool MCPError::isServerError() const
{

    return ::isServerError(m_code);
}

QJsonObject MCPError::toJson() const
{

    QJsonObject errorObj;
    errorObj.insert("code", static_cast<int>(m_code));
    errorObj.insert("message", m_message);

    if (!m_data.isNull())
    {
        errorObj.insert("data", m_data);
    }

    return errorObj;
}

QJsonObject MCPError::toJsonResponse(const QJsonValue& requestId) const
{

    QJsonObject response;
    response.insert("jsonrpc", "2.0");
    response.insert("error", toJson());

    if (!requestId.isNull())
    {
        response.insert("id", requestId);
    }

    return response;
}

MCPError MCPError::fromJson(const QJsonObject& json)
{

    MCPErrorCode code = static_cast<MCPErrorCode>(json.value("code").toInt());
    QString strMessage = json.value("message").toString();
    QJsonValue data = json.value("data");

    return MCPError(code, strMessage, data);
}

// Static factory methods

MCPError MCPError::parseError(const QString& details)
{

    QString message = getErrorMessage(MCPErrorCode::PARSE_ERROR);
    if (!details.isEmpty())
    {
        message += QString(" - ") + details;
    }
    return MCPError(MCPErrorCode::PARSE_ERROR, message);
}

MCPError MCPError::invalidRequest(const QString& details)
{

    QString message = getErrorMessage(MCPErrorCode::INVALID_REQUEST);
    if (!details.isEmpty())
    {
        message += QString(" - ") + details;
    }
    return MCPError(MCPErrorCode::INVALID_REQUEST, message);
}

MCPError MCPError::methodNotFound(const QString& methodName)
{

    // According to JSON-RPC 2.0 and MCP protocol specification, error messages should be in English
    QString message = getErrorMessage(MCPErrorCode::METHOD_NOT_FOUND);
    if (!methodName.isEmpty())
    {
        message += QString(": %1").arg(methodName);
    }
    return MCPError(MCPErrorCode::METHOD_NOT_FOUND, message);
}

MCPError MCPError::invalidParams(const QString& details)
{

    QString message = getErrorMessage(MCPErrorCode::INVALID_PARAMS);
    if (!details.isEmpty())
    {
        message += QString(" - ") + details;
    }
    return MCPError(MCPErrorCode::INVALID_PARAMS, message);
}

MCPError MCPError::internalError(const QString& details)
{

    QString message = getErrorMessage(MCPErrorCode::INTERNAL_ERROR);
    if (!details.isEmpty())
    {
        message += QString(" - ") + details;
    }
    return MCPError(MCPErrorCode::INTERNAL_ERROR, message);
}

MCPError MCPError::toolNotFound(const QString& toolName)
{

    // According to MCP protocol specification, error message for tool not found should be in English
    QString message = "Tool not found";

    // According to MCP protocol specification, data field can contain tool name
    QJsonObject data;
    if (!toolName.isEmpty())
    {
        data["name"] = toolName;
    }

    return MCPError(MCPErrorCode::TOOL_NOT_FOUND, message, data);
}

MCPError MCPError::toolExecutionFailed(const QString& details)
{

    // According to MCP protocol specification, error message for tool execution failure should be in English
    QString message = getErrorMessage(MCPErrorCode::TOOL_EXECUTION_FAILED);
    if (!details.isEmpty())
    {
        message += QString(" - ") + details;
    }
    return MCPError(MCPErrorCode::TOOL_EXECUTION_FAILED, message);
}

MCPError MCPError::resourceNotFound(const QString& resourceUri)
{

    // According to MCP protocol specification, error message for resource not found should be in English
    QString message = "Resource not found";

    // According to MCP protocol specification, data field should contain uri object
    QJsonObject data;
    data["uri"] = resourceUri;

    return MCPError(MCPErrorCode::RESOURCE_NOT_FOUND, message, data);
}

MCPError MCPError::sessionNotFound(const QString& sessionId)
{

    QString message = QString("Session not found: %1").arg(sessionId);
    return MCPError(MCPErrorCode::SESSION_NOT_FOUND, message);
}

MCPError MCPError::authenticationFailed(const QString& details)
{

    QString message = getErrorMessage(MCPErrorCode::AUTHENTICATION_FAILED);
    if (!details.isEmpty())
    {
        message += QString(" - ") + details;
    }
    return MCPError(MCPErrorCode::AUTHENTICATION_FAILED, message);
}

MCPError MCPError::authorizationFailed(const QString& details)
{

    QString message = getErrorMessage(MCPErrorCode::AUTHORIZATION_FAILED);
    if (!details.isEmpty())
    {
        message += QString(" - ") + details;
    }
    return MCPError(MCPErrorCode::AUTHORIZATION_FAILED, message);
}