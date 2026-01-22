#pragma once
#include "MCPMessage.h"
#include <QMap>
#include <QTcpServer>
#include <QTcpSocket>

/**
 * @brief HTTP transport layer implementation
 *
 * Responsibilities:
 * - Implement transport layer based on HTTP/1.1 protocol
 * - Support POST requests
 * - Manage HTTP connection lifecycle
 * - Handle HTTP protocol details
 *
 * Coding standards:
 * - Add m_ prefix to class members
 * - Add p prefix to pointer types
 * - { and } should be on separate lines
 */
class MCPMessage;
class MCPThreadPool;
class MCPHttpConnection;

class MCPHttpTransport : public QTcpServer
{
    Q_OBJECT

public:
    explicit MCPHttpTransport(QObject *pParent = nullptr);
    virtual ~MCPHttpTransport();

public:
    bool start(quint16 nPort);
    bool stop();
    bool isRunning();
signals:
    void messageReceived(quint64 nConnectionId, const QSharedPointer<MCPMessage> &pMessage);
    void connectionDisconnected(quint64 nConnectionId);

public slots:
    void sendMessage(quint64 nConnectionId, QSharedPointer<MCPMessage> pMessage);
    void sendCloseMessage(quint64 nConnectionId, QSharedPointer<MCPMessage> pMessage);
private slots:
    void onDisconnected();

private:
    void incomingConnection(qintptr handle);

private:
    QMap<quint64, MCPHttpConnection *> m_dictConnections;

private:
    MCPThreadPool *m_pThreadPool;
};