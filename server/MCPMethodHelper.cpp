/**
 * @file MCPMethodHelper.cpp
 * @brief MCP方法助手实现
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPMethodHelper.h"
#include "MCPInvokeHelper.h"
#include <MCPLog.h>
#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaMethod>
#include <QScopedPointer>
#include <QThread>
#include <QVariantList>
#include <QVariantMap>

QVariant MCPMethodHelper::syncCallMethod(QObject *pHandler, const QString &strMethodName, const QVariantList &lstArguments)
{
    if (pHandler->thread() == QThread::currentThread()) {
        return call(pHandler, strMethodName, lstArguments);
    }
    QVariant retValue;
    MCPInvokeHelper::syncInvoke(pHandler, [&]() { //
        retValue = call(pHandler, strMethodName, lstArguments);
    });
    return retValue;
}

QVariant MCPMethodHelper::syncCallMethod(QObject *pHandler, const QString &strMethodName, const QVariantMap &dictArguments)
{
    if (pHandler->thread() == QThread::currentThread()) {
        return call(pHandler, strMethodName, dictArguments);
    }
    QVariant retValue;
    MCPInvokeHelper::syncInvoke(pHandler, [&]() { //
        retValue = call(pHandler, strMethodName, dictArguments);
    });
    return retValue;
}

QVariant MCPMethodHelper::call(QObject *pHandler, const QString &strMethodName, const QVariantList &lstArguments)
{
    if (auto pMetaMethod = findMethod(pHandler, strMethodName)) {
        return call(pHandler, pMetaMethod, lstArguments);
    }
    return QVariant();
}

QVariant MCPMethodHelper::call(QObject *pHandler, const QString &strMethodName, const QVariantMap &dictArguments)
{
    if (auto pMetaMethod = findMethod(pHandler, strMethodName)) {
        return call(pHandler, pMetaMethod, dictArguments);
    }
    return QVariant();
}

QVariant MCPMethodHelper::call(QObject *pHandler, const QSharedPointer<QMetaMethod> &pMetaMethod, const QVariantList &lstArguments)
{
    if (!pHandler || !pMetaMethod)
        return QVariant();

    const QList<QByteArray> parameterTypes = pMetaMethod->parameterTypes();
    if (parameterTypes.size() < lstArguments.size()) {
        MCP_CORE_LOG_WARNING().noquote() << "MCPMethodHelper::call: invalid argument count:" //
                                         << lstArguments << parameterTypes.size()            //
                                         << "vs" << lstArguments.size();
        return QVariant();
    }

    MCP_CORE_LOG_DEBUG().noquote() << "TOOL-CALL(" << pHandler->objectName() << "):" //
                                   << pMetaMethod->name() << ":" << parameterTypes;

    // Storage for converted arguments - must stay alive until after invoke()
    QVariant storage[10];      // Qt supports up to 10 args for invoke
    QGenericArgument args[10]; // stack array - safe lifetime

    // Prepare return value container
    QMetaType returnMetaType = QMetaType::fromName(pMetaMethod->typeName());
    QVariant returnValue;
    if (returnMetaType.id() != QMetaType::UnknownType) {
        returnValue = QVariant(returnMetaType, nullptr); // will hold returned data
    } else {
        // fallback: empty QVariant
    }
    QGenericReturnArgument retArg(pMetaMethod->typeName(), const_cast<void *>(returnValue.constData()));

    // use C++ declared parameter count
    const int maxArgs = parameterTypes.size(); // qMin(10, qMin(parameterTypes.size(), lstArguments.size()));
    for (int i = 0; i < maxArgs; ++i) {
        // Get meta info for parameter type (Qt6)
        const QByteArray &typeName = parameterTypes[i];
        QMetaType parameterMeta = QMetaType::fromName(typeName);
        int parameterTypeId = parameterMeta.id();

        // copy input into storage so we have stable memory for the pointer QGenericArgument will hold
        // if less parameter given from MCP client, add empty variants.
        storage[i] = (i < lstArguments.count() ? lstArguments[i] : QVariant());

        // get reference of variant object
        QVariant &inputArg = storage[i];

        if (parameterTypeId == QMetaType::QVariant) {
            // keep as-is
        } else if (inputArg.userType() == parameterTypeId) {
            // already correct type
        } else if (inputArg.canConvert(QMetaType(parameterTypeId))) {
            inputArg.convert(QMetaType(parameterTypeId));
        } else if (customConvert(inputArg, parameterTypeId)) {
            inputArg.convert(QMetaType(parameterTypeId));
        } else {
            MCP_CORE_LOG_DEBUG().noquote() << "MCPMethodHelper::call(arguments type error): " << typeName;
            return QVariant();
        }

        // QGenericArgument takes const char* type name and a pointer to data
        args[i] = QGenericArgument(typeName.constData(), inputArg.constData());
    }

    // Logging arg names/addresses (optional)
    QString _args;
    for (int i = 0; i < maxArgs; ++i) {
        if (!_args.isEmpty())
            _args += ", ";
        _args += QStringLiteral("%1 @%2").arg(parameterTypes[i].constData()).arg((qulonglong) args[i].data(), 8, 16, QChar('0'));
    }
    MCP_CORE_LOG_DEBUG().noquote() << "TOOL-CALL(" << pHandler->objectName() << "):" //
                                   << pMetaMethod->name() << "args:" << _args;
    MCP_CORE_LOG_DEBUG().noquote() << "TOOL-CALL(" << pHandler->objectName() //
                                   << "): MainThread(" << (QThread::currentThread() == QCoreApplication::instance()->thread()) << ")";

    // call invoke with up to 10 args
    pMetaMethod->invoke(pHandler,
                        Qt::DirectConnection, //
                        retArg,
                        args[0],
                        args[1],
                        args[2],
                        args[3],
                        args[4],
                        args[5],
                        args[6],
                        args[7],
                        args[8],
                        args[9]);
    if (returnValue.isValid()) {
        QJsonDocument doc = QJsonDocument(returnValue.toJsonObject());
        MCP_CORE_LOG_DEBUG().noquote() << "TOOL-CALL(" << pHandler->objectName() << "):" //
                                       << pMetaMethod->name() << "" << doc.toJson();
    } else {
        MCP_CORE_LOG_WARNING().noquote() << "TOOL-CALL:" << pMetaMethod->name() << ":" << returnValue;
    }
    return returnValue;
}

QVariant MCPMethodHelper::call(QObject *pHandler, const QSharedPointer<QMetaMethod> &pMetaMethod, const QVariantMap &dictArguments)
{
    auto lstMethodParameterNames = pMetaMethod->parameterNames();
    if (dictArguments.size() > lstMethodParameterNames.size()) {
        MCP_CORE_LOG_WARNING().noquote() << "MCPMethodHelper::createMethodArguments(arguments count error): " << dictArguments.values() << dictArguments;
        return QVariant();
    }

    MCP_CORE_LOG_DEBUG().noquote() << "TOOL-CALL(" << pHandler->objectName() << "):" //
                                   << pMetaMethod->name() << ": dictArguments:" << dictArguments;
    MCP_CORE_LOG_DEBUG().noquote() << "TOOL-CALL(" << pHandler->objectName() << "):" //
                                   << pMetaMethod->name() << ": lstMethodParameterNames:" << lstMethodParameterNames;

    // The MCP client must generate the same parameter name from
    // the JSON input scheme as is declared in C++.
    // dictArguments contain names from MCP client.
    QVariantList lstArguments;
    foreach (const auto &strMethodParameterName, lstMethodParameterNames) {
        auto it = dictArguments.find(strMethodParameterName);
        if (it != dictArguments.end()) {
            lstArguments.append(it.value());
        }
    }

    if (lstArguments.size() != dictArguments.size()) {
        MCP_CORE_LOG_WARNING() << "MCPMethodHelper::createMethodArguments(arguments mismatch error): lstArguments:" << lstArguments.size() //
                               << "!= dictArguments:" << dictArguments.size() << dictArguments << "vs" << lstArguments;
        return QVariant();
    }

    return call(pHandler, pMetaMethod, lstArguments);
}

QSharedPointer<QMetaMethod> MCPMethodHelper::findMethod(QObject *pHandler, const QString &strMethodName)
{
    auto pHanderMetaObject = pHandler->metaObject();
    auto nMethodCount = pHanderMetaObject->methodCount();
    auto nXXMethodCount = pHandler->staticMetaObject.methodCount();
    for (int i = nXXMethodCount; i < nMethodCount; ++i) {
        const QMetaMethod method = pHanderMetaObject->method(i);
        auto strCurMethodName = method.name();
        if (strCurMethodName == strMethodName) {
            return QSharedPointer<QMetaMethod>(new QMetaMethod(method));
        }
    }
    MCP_CORE_LOG_DEBUG().noquote() << "MCPMethodHelper::findMethod:" << strMethodName << " = false";
    return QSharedPointer<QMetaMethod>();
}

bool MCPMethodHelper::customConvert(QVariant &inputArgument, int nMethodParameterType)
{
    // QVariantMap -> QJsonObject
    if (inputArgument.userType() == QMetaType::QVariantMap) {
        if (nMethodParameterType == QMetaType::fromName("QJsonObject").id()) {
            inputArgument = QJsonObject::fromVariantMap(inputArgument.toMap());
            return true;
        }
        if (nMethodParameterType == QMetaType::fromName("QJsonValue").id()) {
            inputArgument = QJsonValue(QJsonObject::fromVariantMap(inputArgument.toMap()));
            return true;
        }
    }

    // QVariantList -> QJsonArray
    if (inputArgument.userType() == QMetaType::QVariantList) {
        if (nMethodParameterType == QMetaType::fromName("QJsonArray").id()) {
            inputArgument = QJsonArray::fromVariantList(inputArgument.toList());
            return true;
        }
        if (nMethodParameterType == QMetaType::fromName("QJsonValue").id()) {
            inputArgument = QJsonValue(QJsonArray::fromVariantList(inputArgument.toList()));
            return true;
        }
    }
    return false;
}
