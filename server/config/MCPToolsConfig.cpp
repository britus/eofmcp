/**
 * @file MCPToolsConfig.cpp
 * @brief MCP tools configuration class implementation
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */


#include "MCPToolsConfig.h"
#include "MCPLog.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>

// ============================================================================ 
// MCPToolConfig implementation
// ============================================================================

QJsonObject MCPToolConfig::toJson() const
{
    QJsonObject json;
    json["name"] = strName;
    json["title"] = strTitle;
    json["description"] = strDescription;
    json["inputSchema"] = jsonInputSchema;
    json["outputSchema"] = jsonOutputSchema;
    json["execHandler"] = strExecHandler;
    json["execMethod"] = strExecMethod;

    // Add annotations (if exists)
    if (!annotations.isEmpty()) {
        json["annotations"] = annotations;
    }

    return json;
}

MCPToolConfig MCPToolConfig::fromJson(const QJsonObject &json)
{
    MCPToolConfig config;
    config.strName = json["name"].toString();
    config.strTitle = json["title"].toString();
    config.strDescription = json["description"].toString();
    config.jsonInputSchema = json["inputSchema"].toObject();
    config.jsonOutputSchema = json["outputSchema"].toObject();
    config.strExecHandler = json["execHandler"].toString();
    config.strExecMethod = json["execMethod"].toString();

    // Parse annotations (if exists)
    if (json.contains("annotations") && json["annotations"].isObject()) {
        config.annotations = json["annotations"].toObject();
    }

    return config;
}

// ============================================================================ 
// MCPToolsConfig implementation
// ============================================================================

MCPToolsConfig::MCPToolsConfig(QObject *pParent)
    : QObject(pParent)
{}

MCPToolsConfig::~MCPToolsConfig() {}

void MCPToolsConfig::addTool(const MCPToolConfig &toolConfig)
{
    m_listToolConfigs.append(toolConfig);
}

QList<MCPToolConfig> MCPToolsConfig::getTools() const
{
    return m_listToolConfigs;
}

int MCPToolsConfig::getToolCount() const
{
    return m_listToolConfigs.size();
}

void MCPToolsConfig::clear()
{
    m_listToolConfigs.clear();
}

void MCPToolsConfig::loadFromJson(const QJsonArray &jsonArray)
{
    m_listToolConfigs.clear();

    for (const QJsonValue &toolValue : jsonArray) {
        if (toolValue.isObject()) {
            m_listToolConfigs.append(MCPToolConfig::fromJson(toolValue.toObject()));
        }
    }
}

QJsonArray MCPToolsConfig::toJson() const
{
    QJsonArray jsonArray;

    for (const MCPToolConfig &toolConfig : m_listToolConfigs) {
        jsonArray.append(toolConfig.toJson());
    }

    return jsonArray;
}

int MCPToolsConfig::loadFromDirectory(const QString &strDirPath)
{
    QDir dir(strDirPath);
    if (!dir.exists()) {
        MCP_CORE_LOG_WARNING() << "MCPToolsConfig: missing dir:" << strDirPath;
        return 0;
    }

    m_listToolConfigs.clear();

    // Recursively find all .json files (including all subdirectories, recursive traversal)
    QDirIterator it(strDirPath, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString fullPath = it.next();

        QFile file(fullPath);
        if (!file.open(QIODevice::ReadOnly)) {
            MCP_CORE_LOG_WARNING() << "MCPToolsConfig: failed to open:" << fullPath;
            continue;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        file.close();

        if (parseError.error != QJsonParseError::NoError) {
            MCP_CORE_LOG_WARNING() << "MCPToolsConfig: JSON-ERROR:" << fullPath << parseError.errorString();
            continue;
        }

        if (doc.isObject()) {
            m_listToolConfigs.append(MCPToolConfig::fromJson(doc.object()));
        }
    }

    MCP_CORE_LOG_INFO() << "MCPToolsConfig: done. count:" << m_listToolConfigs.size();
    return m_listToolConfigs.size();
}