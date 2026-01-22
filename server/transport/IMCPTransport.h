/**
 * @file IMCPTransport.h
 * @brief MCP transport layer abstract interface
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <QObject>
#include <QSharedPointer>

class MCPMessage;

/**
 * @brief MCP transport layer abstract interface
 * 
 * Responsibilities:
 * - Define common behaviors of the transport layer
 * - Support multiple transport implementations (HTTP, WebSocket, Stdio, etc.)
 * - Provide unified message sending and receiving interface
 * 
 * Design notes:
 * - Inherit QObject to support signal-slot mechanism
 * - Specific implementations aggregate underlying transport classes through adapter pattern to avoid diamond inheritance
 * - All methods use pure virtual functions, implemented by subclasses
 * 
 * Coding standards:
 * - Interface methods use pure virtual functions
 * - Parameter naming follows project conventions (n prefix for numeric, p prefix for pointers, str prefix for strings)
 * - { and } on separate lines
 */
class IMCPTransport : public QObject
{
    Q_OBJECT
    
public:
    explicit IMCPTransport(QObject* pParent = nullptr) : QObject(pParent) {}
    virtual ~IMCPTransport() {}
    
    /**
     * @brief Start the transport layer
     * @param nPort Listening port number (for network transport)
     * @return true if successfully started, false if failed to start
     */
    virtual bool start(quint16 nPort = 8888) = 0;
    
    /**
     * @brief Stop the transport layer
     * @return true if successfully stopped, false if failed to stop
     */
    virtual bool stop() = 0;
    
    /**
     * @brief Check if running
     * @return true if running, false if not running
     */
    virtual bool isRunning() = 0;
    /**
     * @brief Send message
     * @param nConnectionId Connection ID
     * @param pMessage Message object pointer
     */
    virtual void sendMessage(quint64 nConnectionId, QSharedPointer<MCPMessage> pMessage) = 0;
    
    /**
     * @brief Send message and close connection
     * @param nConnectionId Connection ID
     * @param pMessage Message object pointer
     */
    virtual void sendCloseMessage(quint64 nConnectionId, QSharedPointer<MCPMessage> pMessage) = 0;

signals:
    /**
     * @brief Message received signal
     * @param nConnectionId Connection ID
     * @param pMessage Message object pointer
     */
    void messageReceived(quint64 nConnectionId, const QSharedPointer<MCPMessage>& pMessage);

    /**
     * @brief Connection disconnected signal
     * @param nConnectionId Connection ID
     */
    void connectionDisconnected(quint64 nConnectionId);
};
