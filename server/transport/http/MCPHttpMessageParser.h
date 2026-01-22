#pragma once
#include <QObject>
#include <QByteArray>
#include <QString>
#include "MCPHttpRequestData.h"
#include "MCPClientMessage.h"

/**
 * @brief MCP HTTP Parser
 *
 * Responsibilities:
 * - Extract MCP messages from HTTP requests
 * - Handle HTTP protocol parsing
 * - Validate HTTP format and content type
 * - Support JSON-RPC mode
 *
 * Coding conventions:
 * - Class members add m_ prefix
 * - Pointer types add p prefix
 * - { and } should be on separate lines
 */
class MCPHttpMessageParser
{
public:
    /**
     * @brief Extract MCP message from HTTP request
     * @param arrHttpData HTTP data
     * @param pRequest Output MCP message pointer
     * @return Whether extraction was successful
     */
    static QSharedPointer<MCPClientMessage> genClientMessageFromHttp(const QSharedPointer<MCPHttpRequestData>& pHttpRequestData);
private:
    static QSharedPointer<MCPClientMessage> genXXClientMessage(const QSharedPointer<MCPClientMessage>& pClientMessage);
};
