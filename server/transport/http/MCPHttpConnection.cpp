/**
 * @file MCPHttpConnection.cpp
 * @brief MCP HTTP connection implementation
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include <MCPHttpConnection.h>
#include <MCPHttpMessageParser.h>
#include <MCPHttpRequestData.h>
#include <MCPHttpRequestParser.h>
#include <MCPInvokeHelper.h>
#include <MCPLog.h>
#include <MCPMessage.h>
#include <QHostAddress>
#include <QTcpSocket>
#include <QThread>
static quint64 SERVER_CONNECTION_ID = 1000;
MCPHttpConnection::MCPHttpConnection(qintptr nSocketDescriptor, QObject *parent)
    : QObject(parent)
    , m_nId(SERVER_CONNECTION_ID++)
    , m_pHttpRequestParser(new MCPHttpRequestParser(this))
{
    m_pSocket = new QTcpSocket(this);
    m_pSocket->setSocketDescriptor(nSocketDescriptor);
    QObject::connect(m_pSocket, &QTcpSocket::readyRead, this, &MCPHttpConnection::onReadyRead);
    QObject::connect(m_pSocket, &QTcpSocket::disconnected, this, &MCPHttpConnection::onDisconnected);
    QObject::connect(m_pSocket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::errorOccurred), this, &MCPHttpConnection::onError);
    QObject::connect(m_pHttpRequestParser, &MCPHttpRequestParser::httpRequestReceived, this, &MCPHttpConnection::onHttpRequestReceived);

    //MCP_TRANSPORT_LOG_INFO() << "Socket Remote:" << nSocketDescriptor << ", addr:" << m_pSocket->peerAddress().toString() << ", port:" << m_pSocket->peerPort();
}

MCPHttpConnection::~MCPHttpConnection() {}

quint64 MCPHttpConnection::getConnectionId()
{
    return m_nId;
}

void MCPHttpConnection::sendMessage(QSharedPointer<MCPMessage> pMessage)
{
    auto data = pMessage->toData();

#if 1
    MCP_TRANSPORT_LOG_INFO() << "HTTP-RESP:" << m_pSocket->peerAddress().toString() << ":" << m_pSocket->peerPort() << "size:" << data.size();
    if (data.indexOf("\n{") > -1) {
        QByteArray payload = data.mid(data.indexOf("{"));
        QJsonDocument doc = QJsonDocument::fromJson(payload);
        MCP_TRANSPORT_LOG_DEBUG().noquote() << "HTTP-RESP:" << doc.toJson();
    }
#endif
    m_pSocket->write(data);
}

void MCPHttpConnection::disconnectFromHost()
{
    MCP_TRANSPORT_LOG_INFO() << "disconnectFromHost:" << m_pSocket->peerAddress().toString() << ":" << m_pSocket->peerPort();
    m_pSocket->disconnectFromHost();
}

void MCPHttpConnection::onReadyRead()
{
    bool rc;
    QByteArray data = m_pSocket->readAll();
    if (!(rc = m_pHttpRequestParser->appendData(data))) {
        MCP_TRANSPORT_LOG_INFO() << "onReadyRead: m_pHttpRequestParser failed:" << rc;
    }
}

void MCPHttpConnection::onError(QAbstractSocket::SocketError error)
{
    if (m_pSocket->isOpen()) {
        MCP_TRANSPORT_LOG_INFO() << "onError:" << error;
        m_pSocket->disconnectFromHost();
    }
}

void MCPHttpConnection::onHttpRequestReceived(QByteArray /*data*/, QSharedPointer<MCPHttpRequestData> pRequestData)
{
    if (auto pMessage = MCPHttpMessageParser::genClientMessageFromHttp(pRequestData)) {
        emit messageReceived(m_nId, pMessage);
    }
}

void MCPHttpConnection::onDisconnected()
{
    MCP_TRANSPORT_LOG_INFO() << "onDisconnected:" << m_pSocket->peerAddress().toString() << ":" << m_pSocket->peerPort();
    emit disconnected();
}
