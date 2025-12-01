/**
 * @file MCPPromptService.cpp
 * @brief MCP prompt service implementation
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPPromptService.h"
#include "MCPPrompt.h"
#include "MCPLog.h"
#include "MCPInvokeHelper.h"
#include "MCPPromptsConfig.h"

MCPPromptService::MCPPromptService(QObject* pParent)
    : IMCPPromptService(pParent)
{
}


MCPPromptService::~MCPPromptService()
{
}


bool MCPPromptService::add(const QString& strName,
                           const QString& strDescription,
                           const QList<QPair<QString, QPair<QString, bool>>>& arguments,
                           std::function<QString(const QMap<QString, QString>&)> generator)
{
    return MCPInvokeHelper::syncInvokeReturn(this, [this, strName, strDescription, arguments, generator]()
    {
        return doAddImpl(strName, strDescription, arguments, generator) != nullptr;
    });
}


bool MCPPromptService::add(const QString& strName,
                           const QString& strDescription,
                           const QList<QPair<QString, QPair<QString, bool>>>& arguments,
                           const QString& strTemplate)
{
    return MCPInvokeHelper::syncInvokeReturn(this, [this, strName, strDescription, arguments, strTemplate]()
    {
        return doAddImpl(strName, strDescription, arguments, strTemplate) != nullptr;
    });
}


bool MCPPromptService::registerPrompt(MCPPrompt* pPrompt)
{
    QString strName = pPrompt->getName();

    // If it already exists, first remove the old one (overwrite), don't send signal because a new object will be sent later
    if (m_dictPrompts.contains(strName))
    {
        MCP_CORE_LOG_INFO() << "MCPPromptService: Prompt already exists, overwriting old prompt:" << strName;
        doRemoveImpl(strName, false);
    }

    m_dictPrompts[strName] = pPrompt;
    MCP_CORE_LOG_INFO() << "MCPPromptService: Prompt registered:" << strName;

    emit promptChanged(strName);
    emit promptsListChanged();
    return true;
}


bool MCPPromptService::remove(const QString& strName)
{
    return MCPInvokeHelper::syncInvokeReturn(this, [this, strName]()
    {
        return doRemoveImpl(strName);
    });
}


bool MCPPromptService::has(const QString& strName) const
{
    return MCPInvokeHelper::syncInvokeReturn(const_cast<MCPPromptService*>(this), [this, strName]()
    {
        return doHasImpl(strName);
    });
}


QJsonArray MCPPromptService::list() const
{
    return MCPInvokeHelper::syncInvokeReturnT<QJsonArray>(const_cast<MCPPromptService*>(this), [this]()->QJsonArray
    {
        return doListImpl();
    });
}


QJsonObject MCPPromptService::getPrompt(const QString& strName, const QMap<QString, QString>& arguments)
{
    QJsonObject objResult;
    MCPInvokeHelper::syncInvoke(this, [this, &objResult, strName, arguments]()
    {
        objResult = doGetPromptImpl(strName, arguments);
    });
    return objResult;
}


bool MCPPromptService::addFromJson(const QJsonObject& jsonPrompt)
{
    return MCPInvokeHelper::syncInvokeReturn(this, [this, jsonPrompt]()
    {
        // Convert JSON object to MCPPromptConfig
        MCPPromptConfig promptConfig = MCPPromptConfig::fromJson(jsonPrompt);
        return addFromConfig(promptConfig);
    });
}


bool MCPPromptService::addFromConfig(const MCPPromptConfig& promptConfig)
{
    // Convert argument format
    QList<QPair<QString, QPair<QString, bool>>> arguments;
    for (const MCPPromptArgumentConfig& arg : promptConfig.listArguments)
    {
        arguments.append(qMakePair(
            arg.strName,
            qMakePair(arg.strDescription, arg.bRequired)
        ));
    }

    // Register using template method (will automatically use default template replacement generator)
    return doAddImpl(
        promptConfig.strName,
        promptConfig.strDescription,
        arguments,
        promptConfig.strTemplate) != nullptr;
}


MCPPrompt* MCPPromptService::createAndConfigurePrompt(const QString& strName,
                                                       const QString& strDescription,
                                                       const QList<QPair<QString, QPair<QString, bool>>>& arguments)
{
    // Create prompt object (parent set to this)
    MCPPrompt* pPrompt = new MCPPrompt(strName, this);
    pPrompt->withDescription(strDescription);

    // Add arguments
    for (const auto& arg : arguments)
    {
        pPrompt->withArgument(arg.first, arg.second.first, arg.second.second);
    }

    return pPrompt;
}


MCPPrompt* MCPPromptService::doAddImpl(const QString& strName,
                                       const QString& strDescription,
                                       const QList<QPair<QString, QPair<QString, bool>>>& arguments,
                                       std::function<QString(const QMap<QString, QString>&)> generator)
{
    // Create and configure prompt object
    MCPPrompt* pPrompt = createAndConfigurePrompt(strName, strDescription, arguments);
    pPrompt->withGenerator(generator);

    // Register to service
    if (!registerPrompt(pPrompt))
    {
        // If registration fails, delete Prompt object to avoid memory leak
        pPrompt->deleteLater();
        return nullptr;
    }

    return pPrompt;
}


MCPPrompt* MCPPromptService::doAddImpl(const QString& strName,
                                       const QString& strDescription,
                                       const QList<QPair<QString, QPair<QString, bool>>>& arguments,
                                       const QString& strTemplate)
{
    // Create and configure prompt object
    MCPPrompt* pPrompt = createAndConfigurePrompt(strName, strDescription, arguments);
    pPrompt->withTemplate(strTemplate);

    // Register to service
    if (!registerPrompt(pPrompt))
    {
        // If registration fails, delete Prompt object to avoid memory leak
        pPrompt->deleteLater();
        return nullptr;
    }

    return pPrompt;
}


bool MCPPromptService::doRemoveImpl(const QString& strName, bool bEmitSignal)
{
    if (!m_dictPrompts.contains(strName))
    {
        MCP_CORE_LOG_WARNING() << "MCPPromptService: Prompt does not exist:" << strName;
        return false;
    }

    MCPPrompt* pPrompt = m_dictPrompts.take(strName);
    if (pPrompt)
    {
        pPrompt->deleteLater();
    }

    MCP_CORE_LOG_INFO() << "MCPPromptService: Prompt unregistered:" << strName;
    if (bEmitSignal)
    {
        emit promptChanged(strName);
        emit promptsListChanged();
    }
    return true;
}


bool MCPPromptService::doHasImpl(const QString& strName) const
{
    return m_dictPrompts.contains(strName);
}


QJsonArray MCPPromptService::doListImpl() const
{
    QJsonArray arrPrompts;

    for (auto it = m_dictPrompts.constBegin(); it != m_dictPrompts.constEnd(); ++it)
    {
        MCPPrompt* pPrompt = it.value();
        arrPrompts.append(pPrompt->getMetadata());
    }

    return arrPrompts;
}


QJsonObject MCPPromptService::doGetPromptImpl(const QString& strName, const QMap<QString, QString>& arguments)
{
    if (!m_dictPrompts.contains(strName))
    {
        MCP_CORE_LOG_WARNING() << "MCPPromptService: Attempting to get non-existent prompt:" << strName;
        return QJsonObject();
    }

    MCPPrompt* pPrompt = m_dictPrompts[strName];
    QJsonArray messages = pPrompt->generate(arguments);

    QJsonObject result;
    result["description"] = pPrompt->getDescription();
    result["messages"] = messages;

    return result;
}
