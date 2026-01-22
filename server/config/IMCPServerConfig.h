/**
 * @file MCPServerConfig.h
 * @brief MCP server configuration interface class (abstract base class)
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <MCPServer_global.h>
#include <QObject>
#include <QSharedPointer>
#include <QString>

class MCPToolsConfig;
class MCPResourcesConfig;
class MCPPromptsConfig;

/**
 * @brief MCP server configuration interface class (abstract base class)
 *
 * Responsibilities:
 * - Define public interfaces for server configuration
 * - Does not contain specific implementation and data
 *
 * Usage example:
 * @code
 * // Load from directory
 * MCPXServerConfig config;
 * config.loadFromDirectory("MCPServerConfig");
 * pServer->start(config.getPort(), &config);
 * @endcode
 *
 * Coding standards:
 * - Add m_ prefix to class members
 * - Add n prefix to numeric types
 * - { and } should be on separate lines
 */
class MCPCORE_EXPORT IMCPServerConfig : public QObject
{
    Q_OBJECT

public:
    explicit IMCPServerConfig(QObject *pParent = nullptr);
    virtual ~IMCPServerConfig();

public:
    // ============ Configuration Loading/Saving ============

    /**
     * @brief Load all configurations from configuration directory
     * @param strConfigDir Configuration root directory (contains ServerConfig.json and subdirectories)
     * @return true if loading is successful, false if it fails
     *
     * Directory structure:
     * MCPServerConfig/
     *   ├── ServerConfig.json      # Main configuration
     *   ├── Tools/                 # Tools directory
     *   │   ├── calculator.json
     *   │   └── ...
     *   ├── Resources/             # Resources directory
     *   │   └── ...
     *   └── Prompts/               # Prompts directory
     *       └── ...
     */
    virtual bool loadFromDirectory(const QString &strConfigDir) = 0;

public:
    // ============ Configuration Access ============

    virtual void setPort(quint16 nPort) = 0;
    virtual quint16 getPort() const = 0;

    virtual void setServerName(const QString &strName) = 0;
    virtual QString getServerName() const = 0;

    virtual void setServerTitle(const QString &strTitle) = 0;
    virtual QString getServerTitle() const = 0;

    virtual void setServerVersion(const QString &strVersion) = 0;
    virtual QString getServerVersion() const = 0;

    virtual void setInstructions(const QString &strInstructions) = 0;
    virtual QString getInstructions() const = 0;

signals:
    /**
     * @brief Configuration loaded signal
     * Emitted when configuration is loaded from directory or file
     * @param pToolsConfig Tool configuration object (lifecycle managed by QSharedPointer)
     * @param pResourcesConfig Resource configuration object (lifecycle managed by QSharedPointer)
     * @param pPromptsConfig Prompt configuration object (lifecycle managed by QSharedPointer)
     * MCPServer will listen to this signal and automatically apply the configuration
     */
    void configLoaded(QSharedPointer<MCPToolsConfig> pToolsConfig, QSharedPointer<MCPResourcesConfig> pResourcesConfig, QSharedPointer<MCPPromptsConfig> pPromptsConfig);
};

//#define IMCPServerConfig_iid "org.eof.IMCPServerConfig"
//Q_DECLARE_INTERFACE(IMCPServerConfig, IMCPServerConfig_iid)