#pragma once

/**
 * @file MCPErrorCode.h
 * @brief MCP error code definition
 * @author zhangheng
 * @date 2025-01-08
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <QString>

/**
 * @brief MCP error code enumeration
 *
 * Follows JSON-RPC 2.0 error code specification:
 * -32600 to -32699: Server errors
 * -32700 to -32000: Predefined errors
 */
enum class MCPErrorCode {
    // Success
    SUCCESS = 0,

    // JSON-RPC 2.0 predefined error codes
    PARSE_ERROR = -32700,          // Parse error
    INVALID_REQUEST = -32600,      // Invalid request
    METHOD_NOT_FOUND = -32601,     // Method not found
    INVALID_PARAMS = -32602,       // Invalid parameters
    INTERNAL_ERROR = -32603,       // Internal error

    // MCP specific error codes (-32000 to -32099)
    SERVER_ERROR_BASE = -32000,
    TOOL_NOT_FOUND = -32001,       // Tool not found
    RESOURCE_NOT_FOUND = -32002,   // Resource not found (conforms to MCP protocol specification)
    TOOL_EXECUTION_FAILED = -32003, // Tool execution failed
    SESSION_NOT_FOUND = -32004,    // Session not found
    SESSION_EXPIRED = -32005,      // Session expired
    AUTHENTICATION_FAILED = -32006, // Authentication failed
    AUTHORIZATION_FAILED = -32007,  // Authorization failed
    RATE_LIMIT_EXCEEDED = -32008,   // Rate limit
    CONFIGURATION_ERROR = -32009,   // Configuration error

    // Network and transmission errors (-32100 to -32199)
    NETWORK_ERROR_BASE = -32100,
    CONNECTION_CLOSED = -32101,     // Connection closed
    CONNECTION_TIMEOUT = -32102,    // Connection timeout
    PROTOCOL_ERROR = -32103,        // Protocol error
    MESSAGE_TOO_LARGE = -32104,     // Message too large

    // Business logic errors (-32200 to -32299)
    BUSINESS_ERROR_BASE = -32200,
    INVALID_TOOL_SCHEMA = -32201,   // Invalid tool schema
    TOOL_VALIDATION_FAILED = -32202, // Tool validation failed
    RESOURCE_ACCESS_DENIED = -32203, // Resource access denied
    SUBSCRIPTION_NOT_SUPPORTED = -32204, // Subscription not supported
};

/**
 * @brief Get error code corresponding description information
 * @param code Error code
 * @return Error description string
 */
QString getErrorMessage(MCPErrorCode code);

/**
 * @brief Check if error code is a server error
 * @param code Error code
 * @return true if it's a server error, false if it's a client error
 */
bool isServerError(MCPErrorCode code);