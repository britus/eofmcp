/**
 * @file MyResourceHandler.cpp
 * @brief Implementation of the example resource handler class
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */
#include "myresourcehandler.h"
#include "IMCPServer.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QVariant>

MyResourceHandler::MyResourceHandler(QObject *pParent)
    : QObject(pParent)
    , m_strName("My Resource")
    , m_strDescription("MCPResourceWrapper")
    , m_strMimeType("application/json")
    , m_strContent("{\"message\":\"Hello, MCP Resource!\",\"timestamp\":\"\"}")
    , m_pTimer(nullptr)
{
    // Set the MCPResourceHandlerName property so that
    // MCPHandlerResolver can locate this object.
    setProperty("MCPResourceHandlerName", MyResourceHandler::metaObject()->className());

    // Set objectName as a fallback identifier
    setObjectName(MyResourceHandler::metaObject()->className());

    // Create a timer that updates the resource content
    // every 5 seconds (used for testing resource change notifications)
    m_pTimer = new QTimer(this);
    connect(m_pTimer, &QTimer::timeout, this, &MyResourceHandler::onTimerTimeout);
    m_pTimer->setSingleShot(true);
    m_pTimer->start(5000); // 5秒触发一次
}

MyResourceHandler::~MyResourceHandler()
{
    if (m_pTimer && m_pTimer->isActive()) {
        m_pTimer->stop();
    }
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
    // 返回当前资源内容
    return m_strContent;
}

void MyResourceHandler::updateContent(const QString &strNewContent)
{
    if (m_strContent != strNewContent) {
        m_strContent = strNewContent;
        // 发出changed信号通知资源内容变化
        emit changed(m_strName, m_strDescription, m_strMimeType);
    }
}

void MyResourceHandler::updateName(const QString &strNewName)
{
    if (m_strName != strNewName) {
        m_strName = strNewName;
        // 发出changed信号通知资源元数据变化
        emit changed(m_strName, m_strDescription, m_strMimeType);
    }
}

void MyResourceHandler::onTimerTimeout()
{
    QJsonObject contentObj;
    contentObj["message"] = "Hello, MCP Resource!";
    contentObj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    contentObj["updateCount"] = 1;

    QJsonDocument doc(contentObj);
    QString strNewContent = doc.toJson(QJsonDocument::Compact);

    // 更新内容并发出changed信号
    updateContent(strNewContent);

    QString toolsDir;

    toolsDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    toolsDir = toolsDir + QDir::separator() + "Tools";

    QFile::Permissions permissions;
    permissions.setFlag(QFile::Permission::ReadOwner, true);
    permissions.setFlag(QFile::Permission::ReadGroup, true);
    permissions.setFlag(QFile::Permission::WriteOwner, true);
    permissions.setFlag(QFile::Permission::WriteGroup, true);
    permissions.setFlag(QFile::Permission::ExeOwner, true);
    permissions.setFlag(QFile::Permission::ExeGroup, true);

    QDir dir(toolsDir);
    if (!dir.exists()) {
        dir.mkpath(toolsDir, permissions);
    }

    // get tool configuration files
    QFileInfoList files = dir.entryInfoList(QStringList() << "*.json");

    // propagate tools
    foreach (auto fi, files) {
        if (fi.isFile() && fi.isReadable()) {
            QString strToolConfigFilePath = fi.absoluteFilePath();
            LoadAutoMCPServerTool(this, strToolConfigFilePath.toUtf8().data());
        }
    }

    m_pTimer->stop();
}
