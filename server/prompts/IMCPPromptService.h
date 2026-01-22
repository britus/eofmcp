/**
 * @file IMCPPromptService.h
 * @brief MCP prompt service interface
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <MCPServer_global.h>
#include <functional>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QString>

/**
 * @brief MCP prompt service interface
 *
 * Responsibilities:
 * - Define public interface of prompt service
 * - Provide prompt registration and management functions
 * - Hide specific implementation details
 *
 * Coding standards:
 * - Use pure virtual functions for interface methods
 * - { and } should be on separate lines
 *
 * @note Deadlock risk explanation: When calling addFromJson and other methods to add prompts during service runtime,
 *       if the caller and related objects are in the same thread, it may cause a deadlock. This issue is currently not handled,
 *       and it's recommended to complete prompt addition operations during service initialization.
 */
class MCPCORE_EXPORT IMCPPromptService : public QObject
{
    Q_OBJECT

public:
    explicit IMCPPromptService(QObject *pParent = nullptr)
        : QObject(pParent)
    {}

protected:
    /**
     * @brief Destructor (protected, Service objects are managed by Server, don't delete directly)
     */
    virtual ~IMCPPromptService() {}

public:
    /**
     * @brief Register prompt
     * @param strName Prompt name
     * @param strDescription Prompt description
     * @param arguments Parameter list (name, (description, required))
     * @param generator Prompt generation function
     * @return true if registration successful, false if failed
     */
    virtual bool add(const QString &strName,
                     const QString &strDescription,
                     const QList<QPair<QString, QPair<QString, bool>>> &arguments,
                     std::function<QString(const QMap<QString, QString> &)> generator)
        = 0;

    /**
     * @brief Register prompt (using template)
     * @param strName Prompt name
     * @param strDescription Prompt description
     * @param arguments Parameter list (name, (description, required))
     * @param strTemplate Prompt template string, supports {{variable name}} format placeholders
     * @return true if registration successful, false if failed
     *
     * Usage example:
     * @code
     * auto pPromptService = pServer->getPromptService();
     * QList<QPair<QString, QPair<QString, bool>>> args;
     * args.append(qMakePair("name", qMakePair("Username", true)));
     * pPromptService->add("greeting", "Greeting prompt", args, "Hello {{name}}, welcome!");
     * @endcode
     */
    virtual bool add(const QString &strName, const QString &strDescription, const QList<QPair<QString, QPair<QString, bool>>> &arguments, const QString &strTemplate) = 0;

    /**
     * @brief Unregister prompt
     * @param strName Prompt name
     * @return true if unregistration successful, false if failed
     */
    virtual bool remove(const QString &strName) = 0;

    /**
     * @brief Check if prompt exists
     * @param strName Prompt name
     * @return true if exists, false if not exists
     */
    virtual bool has(const QString &strName) const = 0;

    /**
     * @brief Get prompt list
     * @return Prompt list (JSON array format)
     */
    virtual QJsonArray list() const = 0;

    /**
     * @brief Get prompt content
     * @param strName Prompt name
     * @param arguments Parameter dictionary
     * @return Prompt content (JSON object format)
     */
    virtual QJsonObject getPrompt(const QString &strName, const QMap<QString, QString> &arguments) = 0;

    /**
     * @brief Add prompt from JSON object
     * @param jsonPrompt JSON object containing configuration information of the prompt
     * @return true if registration successful, false if failed
     *
     * JSON object format:
     * {
     *   "name": "Prompt name",
     *   "description": "Prompt description",
     *   "template": "Prompt template string, supports {{variable name}} format placeholders",
     *   "arguments": [
     *     {
     *       "name": "Parameter name",
     *       "description": "Parameter description",
     *       "required": true/false
     *     },
     *     ...
     *   ]
     * }
     *
     * Usage example:
     * @code
     * QJsonObject json;
     * json["name"] = "greeting";
     * json["description"] = "Greeting prompt";
     * json["template"] = "Hello {{name}}, welcome!";
     * QJsonArray args;
     * QJsonObject arg;
     * arg["name"] = "name";
     * arg["description"] = "Username";
     * arg["required"] = true;
     * args.append(arg);
     * json["arguments"] = args;
     * pPromptService->addFromJson(json);
     * @endcode
     */
    virtual bool addFromJson(const QJsonObject &jsonPrompt) = 0;

signals:
    /**
     * @brief Prompts list changed signal
     * Emitted when prompts are registered or unregistered
     */
    void promptsListChanged();
};

//#define IMCPPromptService_iid "org.eof.IMCPPromptService"
//Q_DECLARE_INTERFACE(IMCPPromptService, IMCPPromptService_iid)
