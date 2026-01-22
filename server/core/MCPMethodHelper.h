/**
 * @file MCPMethodHelper.h
 * @brief MCP method helper class (internal implementation)
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <MCPServer_global.h>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QVariant>
#include <QVariantMap>

class QMetaMethod;
class QObject;

/**
 * @brief MCP method helper class
 *
 * Responsibilities:
 * - Synchronously call object methods through reflection
 * - Support both parameter list and parameter dictionary approaches
 *
 * Coding standards:
 * - Add m_ prefix to class members
 * - Add p prefix to pointer types
 * - Add str prefix to string types
 * - { and } should be on separate lines
 */
class MCPMethodHelper
{
public:
    static QVariant syncCallMethod(QObject *pHandler, const QString &strMethodName, const QVariantList &lstArguments);
    static QVariant syncCallMethod(QObject *pHandler, const QString &strMethodName, const QVariantMap &dictArguments);

private:
    static QSharedPointer<QMetaMethod> findMethod(QObject *pHandler, const QString &strMethodName);
    static QVariant call(QObject *pHandler, const QSharedPointer<QMetaMethod> &pMetaMethod, const QVariantList &lstArguments);
    static QVariant call(QObject *pHandler, const QSharedPointer<QMetaMethod> &pMetaMethod, const QVariantMap &dictArguments);
    static QVariant call(QObject *pHandler, const QString &strMethodName, const QVariantList &lstArguments);
    static QVariant call(QObject *pHandler, const QString &strMethodName, const QVariantMap &dictArguments);

private:
    static bool customConvert(QVariant &inputArgument, int nMethodParameterType);
};
