/**
 * @file MCPClientInitializeMessage.cpp
 * @brief MCP client initialization message implementation
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPClientInitializeMessage.h"
#include <QJsonDocument>
#include <QRegExp>

// mcpMessage base class implementation
MCPClientInitializeMessage::MCPClientInitializeMessage(const MCPClientMessage &clientMessage)
    : MCPClientMessage(clientMessage)
{
    auto params = m_jsonRpc.value("params").toObject();
    m_strClientProtocolVersion = params.value("protocolVersion").toString();
    auto clientInfo = params.value("clientInfo").toObject();
    m_strClientName = clientInfo.value("name").toString();
    m_strClientTitle = clientInfo.value("title").toString();
    m_strClientVersion = clientInfo.value("version").toString();
}

QString MCPClientInitializeMessage::getClientName()
{
    return m_strClientName;
}

QString MCPClientInitializeMessage::getClientTitle()
{
    return m_strClientTitle;
}

QString MCPClientInitializeMessage::getClientVersion()
{
    return m_strClientVersion;
}

QString MCPClientInitializeMessage::getClientProtocolVersion()
{
    return m_strClientProtocolVersion;
}

bool MCPClientInitializeMessage::hasProtocolVersion() const
{
    return !m_strClientProtocolVersion.isEmpty();
}

bool MCPClientInitializeMessage::isProtocolVersionFormatValid() const
{
    // If empty, format is invalid
    if (m_strClientProtocolVersion.isEmpty()) {
        return false;
    }

    // Validate format (should be YYYY-MM-DD format)
    QRegExp versionRegex("^\\d{4}-\\d{2}-\\d{2}$");
    return versionRegex.exactMatch(m_strClientProtocolVersion);
}

bool MCPClientInitializeMessage::isProtocolVersionSupported(const QStringList &supportedVersions) const
{
    return supportedVersions.contains(m_strClientProtocolVersion);
}

bool MCPClientInitializeMessage::isCapabilitiesValid() const
{
    auto params = m_jsonRpc.value("params").toObject();
    // If capabilities is provided, it must be an object
    if (params.contains("capabilities")) {
        return params.value("capabilities").isObject();
    }
    // If not provided, it's also valid (optional parameter)
    return true;
}

bool MCPClientInitializeMessage::isClientInfoValid() const
{
    auto params = m_jsonRpc.value("params").toObject();
    // If clientInfo is provided, it must be an object
    if (params.contains("clientInfo")) {
        return params.value("clientInfo").isObject();
    }
    // If not provided, it's also valid (optional parameter)
    return true;
}
