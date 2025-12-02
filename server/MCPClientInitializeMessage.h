#pragma once
#include <QObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QString>
#include <QSharedPointer>
#include "MCPClientMessage.h"

class MCPClientInitializeMessage : public MCPClientMessage
{
public:
    explicit MCPClientInitializeMessage(const MCPClientMessage&clientMessage);
public:
    //
    QString getClientName();
    QString getClientTitle();
    QString getClientVersion();
    //

    QString getClientProtocolVersion();

    /**
     * @brief Check if protocolVersion parameter exists
     * @return true if exists, false if not exists
     */
    bool hasProtocolVersion() const;

    /**
     * @brief Check if protocolVersion format is valid (YYYY-MM-DD format)
     * @return true if format is valid, false if format is invalid
     */
    bool isProtocolVersionFormatValid() const;

    /**
     * @brief Check if protocol version is in supported list
     * @param supportedVersions List of supported protocol versions
     * @return true if supported, false if not supported
     */
    bool isProtocolVersionSupported(const QStringList& supportedVersions) const;



    /**
     * @brief Check if capabilities parameter is valid (if provided, must be an object)
     * @return true if valid, false if invalid
     */
    bool isCapabilitiesValid() const;

    /**
     * @brief Check if clientInfo parameter is valid (if provided, must be an object)
     * @return true if valid, false if invalid
     */
    bool isClientInfoValid() const;

protected:
    QString m_strClientName;
    QString m_strClientTitle;
    QString m_strClientVersion;
    //
    QString m_strClientProtocolVersion;
private:
    friend class MCPHttpRequestData;
    friend class MCPHttpMessageParser;
};