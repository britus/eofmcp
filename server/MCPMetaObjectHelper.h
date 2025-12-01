/**
 * @file MCPMetaObjectHelper.h
 * @brief MCP Meta Object Helper class (Provides通用 tools for QObject meta object operations)
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <QObject>
#include <QMetaObject>
#include <QMetaMethod>
#include <QString>
#include <QJsonObject>

/**
 * @brief MCP Meta Object Helper class
 * 
 * Responsibilities:
 * - Provide generic operation tools for QObject meta objects
 * - Find and retrieve meta object information such as signals, methods, and properties
 * - Invoke object methods through Qt Meta mechanism
 * - Provide unified error handling and logging
 * 
 * Coding conventions:
 * - Class members add m_ prefix
 * - Pointer types add p prefix
 * - String types add str prefix
 * - { and } should be on separate lines
 */
class MCPMetaObjectHelper
{
public:
    /**
     * @brief Get signal of specified QObject
     * @param pObject QObject object pointer
     * @param strSignalSignature Signal signature (e.g., "changed()")
     * @return QMetaMethod of the signal, returns invalid QMetaMethod if not found or object is null
     * 
     * Usage example:
     * @code
     * QObject* pMyObject = new MyObject();
     * QMetaMethod changedSignal = MCPMetaObjectHelper::getSignal(pMyObject, "changed()");
     * if (changedSignal.isValid())
     * {
     *     QObject::connect(pMyObject, changedSignal, receiver, slot);
     * }
     * @endcode
     */
    static QMetaMethod getSignal(QObject* pObject, const QString& strSignalSignature);
    
    /**
     * @brief Get method (slot or method) of specified QObject
     * @param pObject QObject object pointer
     * @param strMethodSignature Method signature (e.g., "getContent()")
     * @return QMetaMethod of the method, returns invalid QMetaMethod if not found or object is null
     * 
     * Usage example:
     * @code
     * QObject* pMyObject = new MyObject();
     * QMetaMethod getContentMethod = MCPMetaObjectHelper::getMethod(pMyObject, "getContent()");
     * if (getContentMethod.isValid())
     * {
     *     // Use method
     * }
     * @endcode
     */
    static QMetaMethod getMethod(QObject* pObject, const QString& strMethodSignature);
    
    /**
     * @brief Check if QObject has specified signal
     * @param pObject QObject object pointer
     * @param strSignalSignature Signal signature (e.g., "changed()")
     * @return Returns true if exists, otherwise false
     */
    static bool hasSignal(QObject* pObject, const QString& strSignalSignature);
    
    /**
     * @brief Check if QObject has specified method (slot or method)
     * @param pObject QObject object pointer
     * @param strMethodSignature Method signature (e.g., "getContent()")
     * @return Returns true if exists, otherwise false
     */
    static bool hasMethod(QObject* pObject, const QString& strMethodSignature);
    
    /**
     * @brief Check if QObject has slot method with specified name and return type
     * @param pObject QObject object pointer
     * @param strMethodName Method name (without parameters, e.g., "getContent")
     * @param nReturnType Return type (QMetaType type)
     * @return Returns true if exists, otherwise false
     */
    static bool hasSlot(QObject* pObject, const QString& strMethodName, int nReturnType);
    
    /**
     * @brief Invoke object method through Qt Meta mechanism (no return value)
     * @param pObject QObject object pointer
     * @param strMethodName Method name
     * @return Returns true if invocation successful, otherwise false
     */
    static bool invokeMethod(QObject* pObject, const QString& strMethodName);
    
    /**
     * @brief Invoke object method through Qt Meta mechanism (returns QString)
     * @param pObject QObject object pointer
     * @param strMethodName Method name
     * @param strResult Output parameter, method return value
     * @return Returns true if invocation successful, otherwise false
     */
    static bool invokeMethod(QObject* pObject, const QString& strMethodName, QString& strResult);
    
    /**
     * @brief Invoke object method through Qt Meta mechanism (returns QJsonObject)
     * @param pObject QObject object pointer
     * @param strMethodName Method name
     * @param objResult Output parameter, method return value
     * @return Returns true if invocation successful, otherwise false
     */
    static bool invokeMethod(QObject* pObject, const QString& strMethodName, QJsonObject& objResult);
    
    /**
     * @brief Get meta object of QObject
     * @param pObject QObject object pointer
     * @return Meta object pointer, returns nullptr if object is null
     */
    static const QMetaObject* getMetaObject(QObject* pObject);

private:
    // Disable instantiation, all methods are static
    MCPMetaObjectHelper() = delete;
    ~MCPMetaObjectHelper() = delete;
};