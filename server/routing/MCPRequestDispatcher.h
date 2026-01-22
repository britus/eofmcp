/**
 * @file MCPRequestDispatcher.h
 * @brief MCP request dispatcher
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include "MCPServerMessage.h"
#include <QJsonArray>
#include <QObject>
#include <QSharedPointer>
#include <QString>

class MCPToolService;
class MCPResourceService;
class MCPPromptService;
class MCPSessionService;
class MCPRouter;
class MCPContext;
class MCPInitializeHandler;
class MCPSubscriptionHandler;
class IMCPServerConfig;
class MCPServer;

/**
 * @brief MCP request dispatcher
 *
 * Responsibilities:
 * - Configure and manage routing table
 * - Receive client requests and dispatch to corresponding handlers
 * - Provide implementations of various MCP protocol methods
 * - Coordinate various service layers (Tool, Resource, Session, etc.)
 *
 * Design notes:
 * - Use MCPRouter for routing dispatch
 * - Register handler methods to router using Lambda
 * - Maintain centralized management of protocol handling methods
 *
 * Coding conventions:
 * - Add m_ prefix to class members
 * - Add p prefix to pointer types
 * - { and } should be on separate lines
 */
class MCPRequestDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit MCPRequestDispatcher(MCPServer *pServer, QObject *pParent = nullptr);
    virtual ~MCPRequestDispatcher();

signals:
    void serverMessageReceived(const QSharedPointer<MCPServerMessage> &pServerMessage);

public:
    /**
     * @brief Handle client message
     * @param pContext Request context
     * @return Server response message
     */
    QSharedPointer<MCPServerMessage> handleClientMessage(const QSharedPointer<MCPContext> &pContext);

private:
    /**
     * @brief Initialize routing table
     * Register all MCP protocol methods to router
     */
    void initializeRoutes();

private:
    // MCP protocol method handlers
    QSharedPointer<MCPServerMessage> handleConnect(const QSharedPointer<MCPContext> &pContext);
    QSharedPointer<MCPServerMessage> handleToolsList(const QSharedPointer<MCPContext> &pContext);
    QSharedPointer<MCPServerMessage> handleToolsCall(const QSharedPointer<MCPContext> &pContext);
    QSharedPointer<MCPServerMessage> handleListResources(const QSharedPointer<MCPContext> &pContext);
    QSharedPointer<MCPServerMessage> handleListResourceTemplates(const QSharedPointer<MCPContext> &pContext);
    QSharedPointer<MCPServerMessage> handleReadResource(const QSharedPointer<MCPContext> &pContext);
    QSharedPointer<MCPServerMessage> handleListPrompts(const QSharedPointer<MCPContext> &pContext);
    QSharedPointer<MCPServerMessage> handleGetPrompt(const QSharedPointer<MCPContext> &pContext);
    QSharedPointer<MCPServerMessage> handlePing(const QSharedPointer<MCPContext> &pContext);

private:
    QSharedPointer<MCPServerMessage> syncHandleToolsCall(const QSharedPointer<MCPContext> &pContext);

private:
    MCPServer *m_pServer;
    MCPRouter *m_pRouter;
    MCPInitializeHandler *m_pInitializeHandler;
    MCPSubscriptionHandler *m_pSubscriptionHandler;
};
