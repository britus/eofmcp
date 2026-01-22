#pragma once
#include <MCPHttpRequestData.h>
#include <MCPMessage.h>
#include <MCPServerMessage.h>
#include <QAbstractSocket>
#include <QByteArray>
#include <QObject>
#include <QSharedPointer>

class QTcpSocket;
class MCPHttpRequestParser;

class MCPHttpConnection : public QObject
{
    Q_OBJECT
public:
    explicit MCPHttpConnection(qintptr nSocketDescriptor, QObject *parent = nullptr);
    ~MCPHttpConnection();

signals:
    // HTTP request received signal
    void messageReceived(quint64 nConnectionId, const QSharedPointer<MCPMessage> &pMessage);

    void disconnected();

public:
    //
    quint64 getConnectionId();
    //
public slots:
    // Send data
    void sendMessage(QSharedPointer<MCPMessage> pResponse);
    void disconnectFromHost();
private slots:
    // Handle ready read
    void onReadyRead();
    // Handle error
    void onError(QAbstractSocket::SocketError error);
    void onDisconnected();
private slots:
    void onHttpRequestReceived(QByteArray data, QSharedPointer<MCPHttpRequestData> pRequestData);

private:
    quint64 m_nId;
    QTcpSocket *m_pSocket;

private:
    MCPHttpRequestParser *m_pHttpRequestParser;
};