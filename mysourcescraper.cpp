// ********************************************************************
// EoF MCP Server
// Copyright © 2025 by EoF Software Labs
// SPDX-License-Identifier: GPLv3
// ********************************************************************
#include "mysourcescraper.h"
#include <MCPLog.h>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QThread>

MySourceScraper::MySourceScraper(QObject *parent)
    : QObject(parent)
{
    // Set the MCPResourceHandlerName property so that
    // MCPHandlerResolver can locate this object.
    setProperty("MPCToolHandlerName", MySourceScraper::metaObject()->className());

    // Set objectName as a fallback identifier
    setObjectName(MySourceScraper::metaObject()->className());
}

QJsonObject MySourceScraper::sourceScraper(const QString &operation, const QString &url)
{
    MCP_TOOLS_LOG_INFO() << "MySourceScraper::scrapeSource:" << operation << url;

    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate) + "Z";
    // --
    // TODO:
    // --

    // result
    QJsonObject info = QJsonObject({
        QPair<QString, QJsonValue>("result", QJsonValue("OK")),
        QPair<QString, QJsonValue>("success", QJsonValue(true)),
        QPair<QString, QJsonValue>("timestamp", QJsonValue(timestamp)),
    });

    QJsonArray content = QJsonArray({QJsonObject({
        QPair<QString, QString>("type", "text"), //
        QPair<QString, QString>("text", "Hello source from MySourceScraper Content"),
    })});

    QJsonObject response = QJsonObject({
        QPair<QString, QJsonValue>("structuredContent", info),
        QPair<QString, QJsonValue>("content", content),
    });

    return response;
}
