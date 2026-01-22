/**
 * @file MCPServerHandler.h
 * @brief MCP server business handler class (internal implementation)
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QSharedPointer>
#include <QSet>

class MCPMessage;
class MCPServerMessage;
class MCPSessionService;
class MCPToolService;
class MCPResourceService;
class MCPPromptService;
class MCPRequestDispatcher;
class IMCPTransport;
class MCPServerConfig;
class MCPServer;
class MCPResourceNotificationHandler;
class MCPToolNotificationHandler;
class MCPPromptNotificationHandler;
class MCPPendingNotification;
class MCPMessageSender;

/**
 * @brief MCP server business handler class
 * 
 * Responsibilities:
 * - Handle client message reception and response
 * - Manage request dispatcher (MCPRequestDispatcher), responsible for routing and dispatching client requests
 * - Handle server message sending (through MCPMessageSender for unified handling)
 * - Handle connection lifecycle events
 * - Handle subscription notifications
 * - Coordinate various notification handlers (resources, tools, prompts)
 * - Generate and send notification messages (for StreamableTransport)
 * 
 * Note:
 * - Specific notification handling for resources, tools, and prompts is handled by corresponding NotificationHandler
 * - Request routing and dispatching is handled by internal MCPRequestDispatcher
 * - Message sending is handled by MCPMessageSender for unified logic
 * - This class mainly handles coordination of message processing flow and notification sending
 * 
 * Coding conventions:
 * - Class members add m_ prefix
 * - Pointer types add p prefix
 * - { and } should be on separate lines
 */
class MCPServerHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param pServer Server object (required)
     * @param pParent Parent object
     */
    explicit MCPServerHandler(MCPServer* pServer,
                               QObject* pParent = nullptr);
    
    virtual ~MCPServerHandler();

public slots:
    /**
     * @brief Handle client message reception
     * @param nConnectionId Connection ID
     * @param pMessage Message object
     */
    void onClientMessageReceived(quint64 nConnectionId, const QSharedPointer<MCPMessage>& pMessage);

    /**
     * @brief Handle server message reception
     * @param pMessage Server message object
     */
    void onServerMessageReceived(const QSharedPointer<MCPMessage>& pMessage);

    /**
     * @brief Handle connection closed
     * @param nConnectionId Connection ID
     */
    void onConnectionClosed(quint64 nConnectionId);
    
    /**
     * @brief Handle subscription notifications
     * @param strSessionId Session ID
     * @param objNotification Notification message
     */
    void onSubscriptionNotification(const QString& strSessionId, const QJsonObject& objNotification);
    
    /**
     * @brief Handle resource content change event (forward to resource notification handler)
     * @param strUri Resource URI
     */
    void onResourceContentChanged(const QString& strUri);
    
    /**
     * @brief Handle resource deletion event (forward to resource notification handler)
     * @param strUri Resource URI
     */
    void onResourceDeleted(const QString& strUri);
    
    /**
     * @brief Handle resource list change event (forward to resource notification handler)
     */
    void onResourcesListChanged();
    
    /**
     * @brief Handle tool list change event (forward to tool notification handler)
     */
    void onToolsListChanged();
    
    /**
     * @brief Handle prompt list change event (forward to prompt notification handler)
     */
    void onPromptsListChanged();
    
    /**
     * @brief Get resource notification handler
     * @return Pointer to resource notification handler
     */
    MCPResourceNotificationHandler* getResourceNotificationHandler() const;
    
    /**
     * @brief Get tool notification handler
     * @return Pointer to tool notification handler
     */
    MCPToolNotificationHandler* getToolNotificationHandler() const;
    
    /**
     * @brief Get prompt notification handler
     * @return Pointer to prompt notification handler
     */
    MCPPromptNotificationHandler* getPromptNotificationHandler() const;

private slots:
    /**
     * @brief Handle notification requests (emitted by various notification handlers)
     * @param strSessionId Session ID
     * @param objNotification Notification message
     */
    void onNotificationRequested(const QString& strSessionId, const QJsonObject& objNotification);
    
private:
    /**
     * @brief Send pending notifications for Streamable transport
     * @param pServerMessage Server message
     * 
     * Note: This method is called before sending response messages, used to send pending notifications
     */
    void sendStreamableTransportPendingNotifications(const QSharedPointer<MCPServerMessage>& pServerMessage);
private:
    /**
     * @brief Generate notification message by notification method name
     * @param strMethod Notification method name (e.g., "notifications/tools/list_changed")
     * @return Notification message JSON object
     */
    QJsonObject generateNotificationByMethod(const QString& strMethod);
    
    /**
     * @brief Generate resource change notification message
     * @param notification Resource change notification object
     * @return Notification message JSON object, returns empty object if URI is empty
     */
    QJsonObject generateResourceChangedNotification(const MCPPendingNotification& notification);

private:
    MCPServer* m_pServer;  // Server object, through which various services are accessed
    
    // Request dispatcher (responsible for routing and dispatching client requests)
    MCPRequestDispatcher* m_pRequestDispatcher;
    
    // Message sender (unified handling of message sending logic)
    MCPMessageSender* m_pMessageSender;
    
    // Various notification handlers
    MCPResourceNotificationHandler* m_pResourceNotificationHandler;
    MCPToolNotificationHandler* m_pToolNotificationHandler;
    MCPPromptNotificationHandler* m_pPromptNotificationHandler;
};
