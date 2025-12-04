/**
 * @file MCPResourcesConfig.h
 * @brief MCP resource configuration class
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>

/**
 * @brief Resource configuration structure
 */
struct MCPResourceConfig
{
    QString strUri;          // Resource URI
    QString strName;         // Resource name
    QString strDescription;  // Resource description
    QString strMimeType;     // MIME type
    QString strType;         // Resource type: "file" (file resource), "wrapper" (wrapper resource), "content" (content resource, default)
    QJsonObject jsonContent; // Static content (optional, for content type)
    QString strFilePath;     // File path (optional, for file type)
    QString strHandlerName;  // Handler name (optional, for wrapper type, found by MCPResourceHandlerName property in QObject)

    // Resource annotations (Annotations), optional according to MCP protocol specification
    QJsonObject annotations; // Contains fields such as audience, priority, lastModified

    MCPResourceConfig()
        : strMimeType("text/plain")
        , strType("content")
    {}

    QJsonObject toJson() const;
    static MCPResourceConfig fromJson(const QJsonObject &json);
};

/**
 * @brief MCP resource configuration class
 *
 * Responsibilities:
 * - Manage resource configuration list
 * - Load multiple resource configuration files from directory
 *
 * Coding conventions:
 * - Add m_ prefix to class members
 * - Add str prefix to string types
 * - { and } should be on separate lines
 */
class MCPResourcesConfig : public QObject
{
    Q_OBJECT

public:
    explicit MCPResourcesConfig(QObject *pParent = nullptr);
    virtual ~MCPResourcesConfig();

    void addResource(const MCPResourceConfig &resourceConfig);
    QList<MCPResourceConfig> getResources() const;
    int getResourceCount() const;
    void clear();

    void loadFromJson(const QJsonArray &jsonArray);
    QJsonArray toJson() const;

    /**
     * @brief Load all resource configuration files from directory
     * @param strDirPath Resource configuration directory path
     * @return Number of successfully loaded resources
     */
    int loadFromDirectory(const QString &strDirPath);

private:
    QList<MCPResourceConfig> m_listResourceConfigs;
};
