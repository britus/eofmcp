/**
 * @file MCPTool.h
 * @brief MCP tool class (internal implementation)
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <functional>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>

/**
 * @brief MCP tool class
 *
 * Responsibilities:
 * - Define tool metadata
 * - Execute tool calls
 * - Validate input and output
 *
 * Coding conventions:
 * - Class members add m_ prefix
 * - String types add str prefix
 * - { and } should be on separate lines
 */
class MCPTool : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     * @param strName Tool name
     * @param pParent Parent object (usually MCPToolService)
     */
    explicit MCPTool(const QString &strName, QObject *pParent = nullptr);
    ~MCPTool();

public:
    MCPTool *withTitle(const QString &strTitle);
    MCPTool *withDescription(const QString &strDescription);
    MCPTool *withInputSchema(const QJsonObject &jsonInputSchema);
    MCPTool *withOutputSchema(const QJsonObject &jsonOutputSchema);

    /**
     * @brief Set tool annotations
     * @param annotations Annotation object
     *
     * According to the MCP protocol specification, annotations include:
     * - audience: array, valid values are "user" and "assistant"
     * - priority: number between 0.0 and 1.0, indicating importance
     * - lastModified: ISO 8601 formatted timestamp
     */
    MCPTool *withAnnotations(const QJsonObject &annotations);

    /**
     * @brief Get tool annotations
     * @return Annotation object
     */
    QJsonObject getAnnotations() const;

    /**
     * @brief Set target audience
     * @param audience Audience array, valid values are "user" and "assistant"
     */
    MCPTool *withAudience(const QJsonArray &audience);

    /**
     * @brief Set priority
     * @param priority Priority value, range 0.0 to 1.0 (1.0 is most important, 0.0 is least important)
     */
    MCPTool *withPriority(double priority);

    /**
     * @brief Set last modified time
     * @param lastModified ISO 8601 formatted timestamp string (e.g., "2025-01-12T15:00:58Z")
     */
    MCPTool *withLastModified(const QString &lastModified);

    /**
     * @brief Update last modified time to current time
     */
    MCPTool *updateLastModified();

public:
    QString getName() const;
    QJsonObject execute(const QJsonObject &jsonCallArguments);
    QJsonObject getSchema() const;
    QString toString() const;

signals:
    /**
     * @brief Handler destroyed signal
     * @param strToolName Tool name
     *
     * Emitted when the bound Handler object is destroyed,
     * MCPToolService will listen to this signal and automatically unregister the Tool
     */
    void handlerDestroyed(const QString &strToolName);

private slots:
    void onHandlerDestroyed();

private:
    MCPTool *withExecHandler(QObject *pExecHandler, const QString &strMethodName = QString());
    MCPTool *withExecFun(std::function<QJsonObject()> execFun);

private:
    void initSchemaValidator();
    bool validateInput(const QJsonObject &inputObject);
    bool validateOutput(const QJsonObject &outputObject);

private:
    QString m_strName;
    QString m_strTitle;
    QString m_strDescription;
    QJsonObject m_jsonInputSchema;
    QJsonObject m_jsonOutputSchema;

    // Tool annotations, according to MCP protocol specification, optional
    QJsonArray m_audience;     // Audience array, valid values are "user" and "assistant"
    double m_priority;         // Priority, range 0.0 to 1.0
    QString m_strLastModified; // Last modified time, ISO 8601 format

    QObject *m_pExecHandler;
    QString m_strExecMethodName;
    std::function<QJsonObject()> m_execFun;

private:
    friend class MCPToolService;
    friend class MCPAutoServer;
};