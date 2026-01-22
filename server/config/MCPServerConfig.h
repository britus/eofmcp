/**
 * @file MCPXServerConfig.h
 * @brief MCP X server configuration implementation class
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <IMCPServerConfig.h>
#include <QJsonObject>
#include <QMap>

class IMCPServer;

/**
 * @brief MCP X server configuration implementation class
 *
 * Responsibilities:
 * - Manage server configuration (port, server information, etc.)
 * - Load configuration files
 * - Apply configuration to Server
 *
 * Coding conventions:
 * - Class members add m_ prefix
 * - Numeric types add n prefix
 * - { and } should be on separate lines
 */
class MCPServerConfig : public IMCPServerConfig
{
    Q_OBJECT
    friend class IMCPServer; // Allow MCPServer to access private methods
    friend class MCPServer;  // Allow MCPXServer to access private methods

public:
    explicit MCPServerConfig(QObject *pParent = nullptr);
    virtual ~MCPServerConfig();

public:
    // ============ Configuration loading/saving ============

    bool loadFromDirectory(const QString &strConfigDir) override;

public:
    // ============ Configuration access ============

    void setPort(quint16 nPort) override;
    quint16 getPort() const override;

    void setServerName(const QString &strName) override;
    QString getServerName() const override;

    void setServerTitle(const QString &strTitle) override;
    QString getServerTitle() const override;

    void setServerVersion(const QString &strVersion) override;
    QString getServerVersion() const override;

    void setInstructions(const QString &strInstructions) override;
    QString getInstructions() const override;

private:
    // Internal methods
    bool loadFromFile(const QString &strFilePath);
    bool saveToFile(const QString &strFilePath) const;
    bool loadFromJson(const QJsonObject &jsonConfig);
    QJsonObject toJson() const;

private:
    quint16 m_nPort;
    QString m_strServerName;
    QString m_strServerTitle;
    QString m_strServerVersion;
    QString m_strInstructions;

private:
    friend class MCPServer;
};