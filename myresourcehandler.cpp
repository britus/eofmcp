/**
 * @file MyResourceHandler.cpp
 * @brief Implementation of the example resource handler class
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */
#include "myresourcehandler.h"
#include <MCPLog.h>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>
#include <QVariant>

static QMimeDatabase mimeDB;

MyResourceHandler::MyResourceHandler(QObject *pParent)
    : QObject(pParent)
    , m_fileInfo()
    , m_strName()
    , m_strDescription()
    , m_strMimeType()
    , m_strContent()
{
    // Set the MCPResourceHandlerName property so that
    // MCPHandlerResolver can locate this object.
    setProperty("MCPResourceHandlerName", MyResourceHandler::metaObject()->className());

    // Set objectName as a fallback identifier
    setObjectName(MyResourceHandler::metaObject()->className());

    // Create a async timer that updates the resource content
    QTimer::singleShot(5000, this, [this]() {
        QJsonObject contentObj;
        contentObj["message"] = tr("Updated resource: %1").arg("");
        contentObj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        contentObj["updateCount"] = 1;

        QJsonDocument doc(contentObj);
        QString strNewContent = doc.toJson(QJsonDocument::Compact);

        //  MCP_RESOURCE_LOG_DEBUG() << "MyResourceHandler: Update content:" << strNewContent;

        // Update content and emit changed signal
        updateContent(strNewContent);
    });
}

MyResourceHandler::~MyResourceHandler()
{
    //
}

QJsonObject MyResourceHandler::getMetadata() const
{
    QJsonObject metadata;
    metadata["name"] = m_strName;
    metadata["description"] = m_strDescription;
    metadata["mimeType"] = m_strMimeType;
    return metadata;
}

QString MyResourceHandler::getContent() const
{
    // Return current resource content
    return m_strContent;
}

void MyResourceHandler::updateContent(const QString &strNewContent)
{
    if (m_strContent != strNewContent) {
        m_strContent = strNewContent;
        // Emit changed signal to notify resource content change
        emit changed(m_strName, m_strDescription, m_strMimeType);
    }
}

void MyResourceHandler::updateName(const QString &strNewName)
{
    if (m_strName != strNewName) {
        m_strName = strNewName;
        // Emit changed signal to notify resource metadata change
        emit changed(m_strName, m_strDescription, m_strMimeType);
    }
}
