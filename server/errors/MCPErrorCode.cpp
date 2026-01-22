/**
 * @file MCPErrorCode.cpp
 * @brief MCP error code implementation
 * @author zhangheng
 * @date 2025-01-08
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <MCPErrorCode.h>

QString getErrorMessage(MCPErrorCode code)
{
    switch (code)
    {
    case MCPErrorCode::SUCCESS:
        return QString("Success");

    // JSON-RPC 2.0 predefined errors (according to JSON-RPC 2.0 specification, error messages should be in English)
    case MCPErrorCode::PARSE_ERROR:
        return QString("Parse error");
    case MCPErrorCode::INVALID_REQUEST:
        return QString("Invalid Request");
    case MCPErrorCode::METHOD_NOT_FOUND:
        return QString("Method not found");
    case MCPErrorCode::INVALID_PARAMS:
        return QString("Invalid params");
    case MCPErrorCode::INTERNAL_ERROR:
        return QString("Internal error");

    // MCP specific errors (according to MCP protocol specification, error messages should be in English)
    case MCPErrorCode::TOOL_NOT_FOUND:
        return QString("Tool not found");
    case MCPErrorCode::RESOURCE_NOT_FOUND:
        return QString("Resource not found");  // According to MCP protocol specification, use English messages
    case MCPErrorCode::TOOL_EXECUTION_FAILED:
        return QString("Tool execution failed");
    case MCPErrorCode::SESSION_NOT_FOUND:
        return QString("Session not found: The specified session does not exist");
    case MCPErrorCode::SESSION_EXPIRED:
        return QString("Session expired: Session has expired, please reconnect");
    case MCPErrorCode::AUTHENTICATION_FAILED:
        return QString("Authentication failed: Identity verification failed");
    case MCPErrorCode::AUTHORIZATION_FAILED:
        return QString("Authorization failed: Permission verification failed");
    case MCPErrorCode::RATE_LIMIT_EXCEEDED:
        return QString("Rate limit: Requests are too frequent, please try again later");
    case MCPErrorCode::CONFIGURATION_ERROR:
        return QString("Configuration error: Server configuration is abnormal");

    // Network and transmission errors
    case MCPErrorCode::CONNECTION_CLOSED:
        return QString("Connection closed: Network connection was unexpectedly disconnected");
    case MCPErrorCode::CONNECTION_TIMEOUT:
        return QString("Connection timeout: Network request timed out");
    case MCPErrorCode::PROTOCOL_ERROR:
        return QString("Protocol error: Protocol format does not conform to specification");
    case MCPErrorCode::MESSAGE_TOO_LARGE:
        return QString("Message too large: Message size exceeds limit");

    // Business logic errors
    case MCPErrorCode::INVALID_TOOL_SCHEMA:
        return QString("Invalid tool schema: Tool definition does not conform to specification");
    case MCPErrorCode::TOOL_VALIDATION_FAILED:
        return QString("Tool validation failed: Tool parameter validation failed");
    case MCPErrorCode::RESOURCE_ACCESS_DENIED:
        return QString("Resource access denied: No permission to access this resource");
    case MCPErrorCode::SUBSCRIPTION_NOT_SUPPORTED:
        return QString("Subscription not supported: This resource does not support subscription operations");

    default:
        return QString("Unknown error: Undefined error code");
    }
}

bool isServerError(MCPErrorCode code)
{

    // Server error code range: -32699 to -32000, and -32700 (PARSE_ERROR)
    int intCode = static_cast<int>(code);
    return (intCode >= -32700 && intCode <= -32000) || intCode == -32603;
}