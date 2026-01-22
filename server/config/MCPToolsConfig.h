/**
 * @file MCPToolsConfig.h
 * @brief MCP tools configuration class
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QList>

/**
 * @brief Tool configuration structure
 */
struct MCPToolConfig
{
    QString strName;
    QString strTitle;
    QString strDescription;
    QJsonObject jsonInputSchema;
    QJsonObject jsonOutputSchema;
    QString strExecHandler;
    QString strExecMethod;
    
    // Tool annotations (Annotations), optional according to MCP protocol specification
    QJsonObject annotations;   // Contains audience, priority, lastModified and other fields
    
    MCPToolConfig() {}
    
    QJsonObject toJson() const;
    static MCPToolConfig fromJson(const QJsonObject& json);
};

/**
 * @brief MCP tools configuration class
 * 
 * Responsibilities:
 * - Manage tool configuration list
 * - Load/save tool configurations
 * 
 * Coding conventions:
 * - Class members add m_ prefix
 * - String types add str prefix
 * - { and } should be on separate lines
 */
class MCPToolsConfig : public QObject
{
    Q_OBJECT

public:
    explicit MCPToolsConfig(QObject* pParent = nullptr);
    virtual ~MCPToolsConfig();

    /**
     * @brief Add tool configuration
     * @param toolConfig Tool configuration
     */
    void addTool(const MCPToolConfig& toolConfig);
    
    /**
     * @brief Get all tool configurations
     * @return Tool configuration list
     */
    QList<MCPToolConfig> getTools() const;
    
    /**
     * @brief Get tool count
     * @return Tool count
     */
    int getToolCount() const;
    
    /**
     * @brief Clear all tool configurations
     */
    void clear();
    
    /**
     * @brief Load tool configurations from JSON array
     * @param jsonArray Tool configuration JSON array
     */
    void loadFromJson(const QJsonArray& jsonArray);
    
    /**
     * @brief Export to JSON array
     * @return Tool configuration JSON array
     */
    QJsonArray toJson() const;
    
    /**
     * @brief Load all tool configuration files from directory
     * @param strDirPath Tool configuration directory path
     * @return Number of successfully loaded tools
     */
    int loadFromDirectory(const QString& strDirPath);

private:
    QList<MCPToolConfig> m_listToolConfigs;
};
