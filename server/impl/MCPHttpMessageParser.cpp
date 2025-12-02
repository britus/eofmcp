/**
 * @file MCPHttpMessageParser.cpp
 * @brief MCP message parser implementation
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <MCPClientInitializeMessage.h>
#include <MCPHttpMessageParser.h>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QList>
#include <QString>

// Here is sub-thread operation, although it doesn't quite conform to the hierarchy, we still try to process everything here
QSharedPointer<MCPClientMessage> MCPHttpMessageParser::genClientMessageFromHttp(const QSharedPointer<MCPHttpRequestData> &pHttpRequestData)
{
    /*
    POST:
        2025-06-18:
        Recv:
            HTTP_HEADER:
                    MUST include an Accept header, listing both application/json and text/event-stream as supported content types.
                    {Accept: application/json, text/event-stream}
            HTTP_BODY: A single JSON-RPC request, notification, or response
        SEND:
            response -> accepts:HTTP status code 202 Accepted: with no body.
                        cannot accept:HTTP status code 400(Bad Request): no body or JSON-RPC error response that has no id.
            request  ->
                       SSE:Content-Type: text/event-stream + ONE JSON OBJECT
                       COMMON:Content-Type: application/json + ONE JSON OBJECT
        2025-03-26:
        Recv:
            HTTP_HEADER:
                    MUST include an Accept header, listing both application/json and text/event-stream as supported content types.
                    {Accept: application/json, text/event-stream}
            HTTP_BODY:
                     A single JSON-RPC request, notification, or response
                     An array batching one or more requests and/or notifications
                     An array batching one or more responses
        SEND:
        https://modelcontextprotocol.io/specification/2025-03-26/basic/transports
        If the input contains any number of JSON-RPC requests,
            the server MUST either return Content-Type: text/event-stream, to initiate an SSE stream,
            or Content-Type: application/json, to return one JSON object.
        If the input consists solely of (any number of) JSON-RPC responses or notifications:
        If the server accepts the input, the server MUST return HTTP status code 202 Accepted with no body.
        If the server cannot accept the input, it MUST return an HTTP error status code (e.g., 400 Bad Request).
            The HTTP response body MAY comprise a JSON-RPC error response that has no id.
            ......
    
    //GET
    //DELETE close 2025-06-18
    */
    //
    /*
    2024-11-15：
        two endpoints:SSE or Regular HTTP POST
        Open SSE connection GET {Accept: text/event-stream}

    2025-03-26 -
        a single HTTP endpoint supports both POST and GET methods.
        MUST POST {Accept: application/json, text/event-stream}
    */
    //GET POST DELETE
    auto strHttpMethod = pHttpRequestData->getMethod();
    auto strConnection = pHttpRequestData->getHeader("connection"); //keep-alive
    //application/json,text/event-stream
    auto strAcceptHeader = pHttpRequestData->getHeader("Accept");
    auto setAcceptTypes = strAcceptHeader.split(",").toList(); //Set();
    // Clean spaces
    QList<QString> cleanedAcceptTypes;
    foreach (const QString &type, setAcceptTypes) {
        cleanedAcceptTypes.append(type.trimmed());
    }
    setAcceptTypes = cleanedAcceptTypes;
    //application/json,text/event-stream
    auto strContentType = pHttpRequestData->getHeader("content-type");

    //sse?Mcp-Session-Id=123 NOT 2025-06-18
    auto strQuerySessionId = pHttpRequestData->getQueryParameter("Mcp-Session-Id");
    auto strMcpSessionId = pHttpRequestData->getHeader("Mcp-Session-Id");
    auto strProtocolVersion = pHttpRequestData->getHeader("MCP-Protocol-Version");

    //1、First, remove the HTTP that sneakily comes in - here we fix it and don't support setting
    auto strPath = pHttpRequestData->getPath();
    if (strPath != "/sse" && strPath != "/mcp") {
        return QSharedPointer<MCPClientMessage>();
    }

    // Validate Accept header (for POST requests, must include application/json and text/event-stream)
    if (strHttpMethod == "POST") {
        static QList<QString> setRequiredTypes = {"application/json", "text/event-stream"};
        if (!setAcceptTypes.contains(setRequiredTypes.at(0))    //2015-06-18
            && !setAcceptTypes.contains(setRequiredTypes.at(1)) //
            && !setAcceptTypes.contains("*/*")) {
            // Accept header does not meet MCP specification requirements
            return QSharedPointer<MCPClientMessage>();
        }
    }

    //2、Then remove unsupported protocols
    /* Stream resumption - 2025-03-26
    https://modelcontextprotocol.io/specification/2025-03-26/basic/transports#resumability-and-redelivery
    Designing Last-Event-ID = SessionId + EventId-123, parsing SessionId and EventId from this is not supported at all - too complicated and messy with no real use
    */
    auto strLastEventId = pHttpRequestData->getHeader("Last-Event-ID"); //Last-Event-ID
    if (strHttpMethod == "GET" && !strLastEventId.isEmpty()) {
        return QSharedPointer<MCPClientMessage>();
    }
    /* Client close - 2025-03-26
    https://modelcontextprotocol.io/specification/2025-03-26/basic/transports#streamable-http
    Don't quite understand the meaning, close it if it's closed, feels like it has no real use
    */
    if (strHttpMethod == "DELETE") {
        return QSharedPointer<MCPClientMessage>();
    }

    auto pClientMessage = QSharedPointer<MCPClientMessage>::create(MCPMessageType::None);

    pClientMessage->m_strMcpSessionId = strQuerySessionId.isEmpty() ? strMcpSessionId : strQuerySessionId;
    //
    //
    //To be compatible with [2024-11-15], first check if it's an SSE connection coming in - only SSE connections will do this
    //https://modelcontextprotocol.io/specification/2024-11-05/basic/transports
    if (strHttpMethod == "GET" && strQuerySessionId.isEmpty()                         // no sse sessionid
        && strMcpSessionId.isEmpty()                                                  //no mcp sessionid
        && strLastEventId.isEmpty()                                                   // not reconnecting
        && setAcceptTypes.size() == 1 && setAcceptTypes.contains("text/event-stream") //only sse
        && strConnection == "keep-alive")                                             //sse keep-alive
    {
        //This connect simulates an RPC call
        pClientMessage->m_jsonRpc.insert("method", "connect");
        pClientMessage->appendType(MCPMessageType::SseTransport | MCPMessageType::Connect);
        return pClientMessage;
    }
    //Batch operations abandon support - 2025-06-18 has clearly abandoned
    //https://modelcontextprotocol.io/specification/2025-03-26/basic/transports#streamable-http
    if (strHttpMethod == "POST" && strContentType == "application/json") {
        auto jsonRpc = QJsonDocument::fromJson(pHttpRequestData->getBody()).object();

        // Validate JSON-RPC version field (must be string "2.0")
        QJsonValue jsonrpcValue = jsonRpc.value("jsonrpc");
        if (!jsonrpcValue.isString() || jsonrpcValue.toString() != "2.0") {
            // JSON-RPC version field missing or malformed
            return QSharedPointer<MCPClientMessage>();
        }

        auto strXMethodName = jsonRpc.value("method").toString();
        auto bRequest = jsonRpc.contains("id") && jsonRpc.contains("method");
        auto bResponse = jsonRpc.contains("id") && ((jsonRpc.contains("result") + jsonRpc.contains("error")) == 1);
        auto bNotification = !jsonRpc.contains("id");
        if (bRequest || bResponse || bNotification) {
            pClientMessage->m_jsonRpc = jsonRpc;
            //
            bRequest && pClientMessage->appendType(MCPMessageType::Request);
            bResponse && pClientMessage->appendType(MCPMessageType::Response);
            bNotification && pClientMessage->appendType(MCPMessageType::Notification);
            //
            pClientMessage->appendType(strQuerySessionId.isEmpty() ? MCPMessageType::StreamableTransport : MCPMessageType::SseTransport);
            //Put this here for now
            return genXXClientMessage(pClientMessage);
        }
    }

    return QSharedPointer<MCPClientMessage>();
}

QSharedPointer<MCPClientMessage> MCPHttpMessageParser::genXXClientMessage(const QSharedPointer<MCPClientMessage> &pClientMessage)
{
    if (pClientMessage->getMethodName() == "ping") {
        pClientMessage->appendType(MCPMessageType::Ping);
    }
    if (pClientMessage->getMethodName() == "initialize") {
        pClientMessage->appendType(MCPMessageType::Initialize);
        return QSharedPointer<MCPClientInitializeMessage>::create(*pClientMessage);
    }
    return pClientMessage;
}