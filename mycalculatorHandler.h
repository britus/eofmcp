#pragma once
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>

/**
 * @brief Example handler demonstrating how to use MCPServerHandler
 */
class MyCalculatorHandler : public QObject
{
    Q_OBJECT

public:
    explicit MyCalculatorHandler(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE QJsonObject calculateOperation(double a, double b, const QString &operation);
};
