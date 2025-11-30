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
    setProperty("MCPResourceHandlerName", MySourceScraper::metaObject()->className());

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
    QJsonObject result = QJsonObject({
        QPair<QString, QJsonValue>("result", QJsonValue("OK")),
        QPair<QString, QJsonValue>("success", QJsonValue(true)),
        QPair<QString, QJsonValue>("timestamp", QJsonValue(timestamp)),
    });

    QJsonObject response;
    response["structuredContent"] = result;
    response["content"] = QJsonArray({QJsonObject({
        QPair<QString, QString>("type", "text"), //
        QPair<QString, QString>("text", "Hello source from MySourceScraper Content"),
    })});

    return response;
}
