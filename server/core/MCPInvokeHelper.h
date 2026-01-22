/**
 * @file MCPInvokeHelper.h
 * @brief MCP method invocation helper class (internal implementation)
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <MCPServer_global.h>
#include <functional>
#include <QCoreApplication>
#include <QMetaMethod>
#include <QObject>
#include <QSharedPointer>
#include <QVariant>

/**
 * @brief MCP method invocation helper class
 *
 * Qt older versions do not support cross-object invoke, QTimer has strict requirements, this class is used to simulate
 *
 * Coding conventions:
 * - Class members add m_ prefix
 * - Pointer types add p prefix
 * - { and } should be on separate lines
 */
class MCPInvokeHelper : public QObject
{
    Q_OBJECT

public:
    template<typename RetType>
    static RetType syncInvokeReturnT(QObject *pTargetObj, const std::function<RetType()> &fun)
    {
        RetType retValue;
        syncInvoke(pTargetObj, [&retValue, fun]() { retValue = fun(); });
        return retValue;
    }

public:
    // Synchronous call returning bool value
    static bool syncInvokeReturn(QObject *pTargetObj, const std::function<bool()> &fun);
    //
    static void syncInvoke(QObject *pTargetObj, const std::function<void()> &fun);
    static void asynInvoke(QObject *pTargetObj, const std::function<void()> &fun);

public:
    static void setThreadName(unsigned long dwThreadID, QString strThreadName);
    static void setCurrentThreadName(QString strThreadName);

public:
    MCPInvokeHelper(QObject *pTargetObj);
    void operator+(const std::function<void()> &fun);
    void operator-(const std::function<void()> &fun);

private:
    ~MCPInvokeHelper();
private slots:
    void invoke();

private:
    std::function<void()> m_fun;
};

#define SYNC_SAFE_THREAD_INVOKE *(new MCPInvokeHelper(this)) + [&]()
#define SYNC_MAIN_THREAD_INVOKE *(new MCPInvokeHelper(QCoreApplication::instance())) + [&]()
