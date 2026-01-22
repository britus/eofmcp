/**
 * @file MCPPromptsConfig.h
 * @brief MCP prompts configuration class
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
#include <QPair>

/**
 * @brief Prompt argument configuration
 */
struct MCPPromptArgumentConfig
{
    QString strName;
    QString strDescription;
    bool bRequired;
    
    MCPPromptArgumentConfig() : bRequired(false) {}
};

/**
 * @brief Prompt configuration structure
 */
struct MCPPromptConfig
{
    QString strName;
    QString strDescription;
    QList<MCPPromptArgumentConfig> listArguments;
    QString strTemplate;  // Prompt template
    
    MCPPromptConfig() {}
    
    QJsonObject toJson() const;
    static MCPPromptConfig fromJson(const QJsonObject& json);
};

/**
 * @brief MCP prompts configuration class
 * 
 * Responsibilities:
 * - Manage prompt configuration list
 * - Load multiple prompt configuration files from directory
 */
class MCPPromptsConfig : public QObject
{
    Q_OBJECT

public:
    explicit MCPPromptsConfig(QObject* pParent = nullptr);
    virtual ~MCPPromptsConfig();

    void addPrompt(const MCPPromptConfig& promptConfig);
    QList<MCPPromptConfig> getPrompts() const;
    int getPromptCount() const;
    void clear();
    
    void loadFromJson(const QJsonArray& jsonArray);
    QJsonArray toJson() const;
    
    /**
     * @brief Load all prompt configuration files from directory
     * @param strDirPath Prompt configuration directory path
     * @return Number of successfully loaded prompts
     */
    int loadFromDirectory(const QString& strDirPath);

private:
    QList<MCPPromptConfig> m_listPromptConfigs;
};
