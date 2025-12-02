/**
 * @file MCPHttpTransport.cpp
 * @brief MCP HTTP transport implementation
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPHttpTransport.h"
#include "MCPClientMessage.h"
#include "MCPInvokeHelper.h"
#include "MCPLog.h"
#include "impl/MCPHttpConnection.h"
#include "impl/MCPHttpRequestData.h"
#include <impl/MCPThreadPool.h>
#include <QCoreApplication>
#include <QDateTime>
#include <QHostAddress>
#include <QJsonDocument>
#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>

MCPHttpTransport::MCPHttpTransport(QObject *pParent)
    : QTcpServer(pParent)
    , m_pThreadPool(new MCPThreadPool(2, this))
{
    qRegisterMetaType<QSharedPointer<MCPHttpRequestData>>("QSharedPointer<HttpRequestData>");
    qRegisterMetaType<QSharedPointer<MCPMessage>>("QSharedPointer<MCPMessage>");
    qRegisterMetaType<QSharedPointer<MCPClientMessage>>("QSharedPointer<MCPClientMessage>");
    qRegisterMetaType<QSharedPointer<MCPServerMessage>>("QSharedPointer<MCPResponse>");
}

MCPHttpTransport::~MCPHttpTransport() {}

bool MCPHttpTransport::start(quint16 nPort)
{
    if (isListening()) {
        return true; // Already started
    }

    //if (!listen(QHostAddress::Any, nPort))
    if (!listen(QHostAddress("0.0.0.0"), nPort)) {
        MCP_TRANSPORT_LOG_WARNING() << "start port:" << nPort << "error:" << errorString();
        return false;
    }
    MCP_TRANSPORT_LOG_INFO() << "MCP HTTP start:" << nPort << "OK";
    return true;
}

bool MCPHttpTransport::stop()
{
    MCP_TRANSPORT_LOG_INFO() << "MCP HTTP stop";
    close();
    return true;
}

bool MCPHttpTransport::isRunning()
{
    return isListening();
}

void MCPHttpTransport::sendMessage(quint64 nConnectionId, QSharedPointer<MCPMessage> pResponse)
{
    if (auto pConnection = m_dictConnections.value(nConnectionId)) {
        MCPInvokeHelper::syncInvoke(pConnection, [pConnection, pResponse]() { //
            pConnection->sendMessage(pResponse);
        });
    }
}

void MCPHttpTransport::sendCloseMessage(quint64 nConnectionId, QSharedPointer<MCPMessage> pResponse)
{
    if (auto pConnection = m_dictConnections.value(nConnectionId)) {
        MCPInvokeHelper::asynInvoke(pConnection, [pConnection, pResponse]() {
            pConnection->sendMessage(pResponse);
            //pConnection->disconnectFromHost();
        });
    }
}

void MCPHttpTransport::incomingConnection(qintptr handle)
{
    //() << "incomingConnection: handle:" << handle;
    MCPHttpConnection *pConnection = new MCPHttpConnection(handle, nullptr);
    QObject::connect(pConnection, &MCPHttpConnection::messageReceived, this, &MCPHttpTransport::messageReceived);
    QObject::connect(pConnection, &MCPHttpConnection::disconnected, this, &MCPHttpTransport::onDisconnected);
    m_dictConnections[pConnection->getConnectionId()] = pConnection;
    m_pThreadPool->addWorker(pConnection);
}

void MCPHttpTransport::onDisconnected()
{
    if (auto pConnection = qobject_cast<MCPHttpConnection *>(sender())) {
        quint64 nConnectionId = pConnection->getConnectionId();
        MCP_TRANSPORT_LOG_INFO() << "onDisconnected: nConnectionId:" << nConnectionId;
        m_dictConnections.remove(nConnectionId);
        m_pThreadPool->removeWorker(pConnection);
        pConnection->deleteLater();

        // Send connection disconnected signal
        emit connectionDisconnected(nConnectionId);
    }
}