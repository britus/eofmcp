#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTimer>

class MySourceScraper : public QObject
{
    Q_OBJECT

public:
    explicit MySourceScraper(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE QJsonObject sourceScraper(const QString &operation, const QString &url);
};
