/**
 * @file MCPHttpTransportAdapter.cpp
 * @brief MCP HTTP传输适配器实现
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */
#include "MCPHttpTransportAdapter.h"
#include "MCPHttpTransport.h"
#include "MCPMessage.h"

MCPHttpTransportAdapter::MCPHttpTransportAdapter(QObject *pParent)
    : IMCPTransport(pParent)
    , m_pHttpTransport(new MCPHttpTransport(this))
{
    QObject::connect(m_pHttpTransport, &MCPHttpTransport::messageReceived, this, &IMCPTransport::messageReceived);
    QObject::connect(m_pHttpTransport, &MCPHttpTransport::connectionDisconnected, this, &IMCPTransport::connectionDisconnected);
}

MCPHttpTransportAdapter::~MCPHttpTransportAdapter()
{
    //-
}

bool MCPHttpTransportAdapter::start(quint16 nPort)
{
    return m_pHttpTransport->start(nPort);
}

bool MCPHttpTransportAdapter::stop()
{
    return m_pHttpTransport->stop();
}

bool MCPHttpTransportAdapter::isRunning()
{
    return m_pHttpTransport->isRunning();
}

void MCPHttpTransportAdapter::sendMessage(quint64 nConnectionId, QSharedPointer<MCPMessage> pMessage)
{
    m_pHttpTransport->sendMessage(nConnectionId, pMessage);
}

void MCPHttpTransportAdapter::sendCloseMessage(quint64 nConnectionId, QSharedPointer<MCPMessage> pMessage)
{
    m_pHttpTransport->sendCloseMessage(nConnectionId, pMessage);
}
