/**
 * @file MCPServerConfig.cpp
 * @brief MCP X服务器配置实现
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPServerConfig.h"
#include "MCPLog.h"
#include "MCPPromptsConfig.h"
#include "MCPResourcesConfig.h"
#include "MCPServer.h"
#include "MCPToolsConfig.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>

// ============================================================================
// MCPServerConfig 实现
// ============================================================================

MCPServerConfig::MCPServerConfig(QObject *pParent)
    : IMCPServerConfig(pParent)
    , m_nPort(6605)
    , m_strServerName("C++ MCPServer")
    , m_strServerTitle("C++ MCP Server Implementation")
    , m_strServerVersion("1.0.0")
    , m_strInstructions(" C++ Qt MCP Instructions")
{}

MCPServerConfig::~MCPServerConfig() {}

bool MCPServerConfig::loadFromDirectory(const QString &strConfigDir)
{
    QDir configDir(strConfigDir);
    if (!configDir.exists()) {
        MCP_CORE_LOG_WARNING() << "MCPServerConfig: directory missing: " << strConfigDir;
        return false;
    }

    // 1. 加载主配置文件 ServerConfig.json
    QString strServerConfigPath = configDir.absoluteFilePath("eofmc.config");
    if (QFile::exists(strServerConfigPath)) {
        if (!loadFromFile(strServerConfigPath)) {
            MCP_CORE_LOG_WARNING() << "MCPServerConfig: loadFromFile failed:" << strServerConfigPath;
            return false;
        }
    } else {
        MCP_CORE_LOG_WARNING() << "MCPServerConfig: ok:" << strServerConfigPath;
    }

    // 2. 加载工具配置目录
    QSharedPointer<MCPToolsConfig> pToolsConfig(new MCPToolsConfig());
    QString strToolsDir = configDir.absoluteFilePath("Tools");
    if (QDir(strToolsDir).exists()) {
        pToolsConfig->loadFromDirectory(strToolsDir);
    }

    // 3. 加载资源配置目录
    QSharedPointer<MCPResourcesConfig> pResourcesConfig(new MCPResourcesConfig());
    QString strResourcesDir = configDir.absoluteFilePath("Resources");
    if (QDir(strResourcesDir).exists()) {
        pResourcesConfig->loadFromDirectory(strResourcesDir);
    }

    // 4. 加载提示词配置目录
    QSharedPointer<MCPPromptsConfig> pPromptsConfig(new MCPPromptsConfig());
    QString strPromptsDir = configDir.absoluteFilePath("Prompts");
    if (QDir(strPromptsDir).exists()) {
        pPromptsConfig->loadFromDirectory(strPromptsDir);
    }

    MCP_CORE_LOG_INFO() << "MCPServerConfig: port:" << m_nPort << ", name:" << m_strServerName //
                        << "tools:" << pToolsConfig->getToolCount()                            //
                        << "resources:" << pResourcesConfig->getResourceCount()                //
                        << "prompts:" << pPromptsConfig->getPromptCount();

    // 发送配置加载完成信号，传递配置对象
    emit configLoaded(pToolsConfig, pResourcesConfig, pPromptsConfig);

    return true;
}

bool MCPServerConfig::loadFromFile(const QString &strFilePath)
{
    QFile configFile(strFilePath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        MCP_CORE_LOG_WARNING() << "MCPServerConfig: failed to open file:" << strFilePath;
        return false;
    }

    QByteArray jsonData = configFile.readAll();
    configFile.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        MCP_CORE_LOG_WARNING() << "MCPServerConfig: JSON-ERROR:" << parseError.errorString();
        return false;
    }

    if (!jsonDoc.isObject()) {
        MCP_CORE_LOG_WARNING() << "MCPServerConfig: is not an object" << jsonDoc;
        return false;
    }

    return loadFromJson(jsonDoc.object());
}

bool MCPServerConfig::loadFromJson(const QJsonObject &jsonConfig)
{
    // 读取端口
    m_nPort = static_cast<quint16>(jsonConfig.value("port").toInt(6605));

    // 读取服务器信息
    if (jsonConfig.contains("serverInfo")) {
        QJsonObject serverInfo = jsonConfig["serverInfo"].toObject();
        m_strServerName = serverInfo["name"].toString("C++ MCPServer");
        m_strServerTitle = serverInfo["title"].toString("C++ MCP Server Implementation");
        m_strServerVersion = serverInfo["version"].toString("1.0.0");
    }

    // 读取使用说明
    if (jsonConfig.contains("instructions")) {
        m_strInstructions = jsonConfig["instructions"].toString();
    }

    MCP_CORE_LOG_INFO() << "MCPServerConfig: port:" << m_nPort << ", name:" << m_strServerName;
    return true;
}

bool MCPServerConfig::saveToFile(const QString &strFilePath) const
{
    QFile configFile(strFilePath);
    if (!configFile.open(QIODevice::WriteOnly)) {
        MCP_CORE_LOG_WARNING() << "MCPServerConfig: file to open:" << strFilePath;
        return false;
    }

    QJsonDocument jsonDoc(toJson());
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);

    qint64 nWritten = configFile.write(jsonData);
    configFile.close();

    if (nWritten != jsonData.size()) {
        MCP_CORE_LOG_WARNING() << "MCPServerConfig: failed to write:" << strFilePath;
        return false;
    }

    MCP_CORE_LOG_INFO() << "MCPServerConfig: ok:" << strFilePath;
    return true;
}

QJsonObject MCPServerConfig::toJson() const
{
    QJsonObject json;
    json["port"] = m_nPort;

    // 导出服务器信息
    QJsonObject serverInfo;
    serverInfo["name"] = m_strServerName;
    serverInfo["title"] = m_strServerTitle;
    serverInfo["version"] = m_strServerVersion;
    json["serverInfo"] = serverInfo;

    json["instructions"] = m_strInstructions;

    return json;
}

void MCPServerConfig::setPort(quint16 nPort)
{
    m_nPort = nPort;
}

quint16 MCPServerConfig::getPort() const
{
    return m_nPort;
}

void MCPServerConfig::setServerName(const QString &strName)
{
    m_strServerName = strName;
}

QString MCPServerConfig::getServerName() const
{
    return m_strServerName;
}

void MCPServerConfig::setServerTitle(const QString &strTitle)
{
    m_strServerTitle = strTitle;
}

QString MCPServerConfig::getServerTitle() const
{
    return m_strServerTitle;
}

void MCPServerConfig::setServerVersion(const QString &strVersion)
{
    m_strServerVersion = strVersion;
}

QString MCPServerConfig::getServerVersion() const
{
    return m_strServerVersion;
}

void MCPServerConfig::setInstructions(const QString &strInstructions)
{
    m_strInstructions = strInstructions;
}

QString MCPServerConfig::getInstructions() const
{
    return m_strInstructions;
}
