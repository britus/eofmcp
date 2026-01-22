#pragma once
#include <MCPContext.h>
#include <MCPMessage.h>
#include <MCPServerMessage.h>
#include <MCPSession.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QSet>
#include <QSharedPointer>
#include <QString>

class MCPHttpReplyMessage : public MCPServerMessage
{
public:
    MCPHttpReplyMessage(const QSharedPointer<MCPServerMessage> &pServerMessage, MCPMessageType::Flags flags);

public:
    static QSharedPointer<MCPHttpReplyMessage> CreateSseAcceptNotification();
    static QSharedPointer<MCPHttpReplyMessage> CreateStreamableAcceptNotification();

public:
    virtual QByteArray toData() override;

private:
    QByteArray toSseConnectResponseData();
    QByteArray toSseRequestData();
    QByteArray toSseNotificationData();
    //
    QByteArray toStreamableConnectData();
    QByteArray toStreamableRequestData();
    QByteArray toStreamableNotificationData();

private:
    QByteArray toSseChannelData();
    QByteArray toAcceptData();

protected:
    MCPMessageType::Flags m_flags;
    QSharedPointer<MCPServerMessage> m_pServerMessage;
};
