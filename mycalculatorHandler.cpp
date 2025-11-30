/**
 * @file MyCalculatorHandler.cpp
 * @brief 示例处理器实现
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */
#include "mycalculatorHandler.h"
#include <MCPLog.h>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QThread>

MyCalculatorHandler::MyCalculatorHandler(QObject *parent)
    : QObject(parent)
{
    // Set the MPCToolHandlerName property so that
    // MCPHandlerResolver can locate this object.
    setProperty("MPCToolHandlerName", MyCalculatorHandler::metaObject()->className());

    // Set objectName as a fallback identifier
    setObjectName(MyCalculatorHandler::metaObject()->className());
}

QJsonObject MyCalculatorHandler::calculateOperation(double a, double b, const QString &operation)
{
    double result = 0;
    bool success = true;
    QString errorMsg;

    MCP_TOOLS_LOG_INFO() << "MyCalculatorHandler::calculateOperation: a:" //
                         << a << "b:" << b << "op:" << operation;

    // --
    if (operation == "add") {
        result = a + b;
    } else if (operation == "subtract") {
        result = a - b;
    } else if (operation == "multiply") {
        result = a * b;
    } else if (operation == "divide") {
        if (b != 0) {
            result = a / b;
        } else {
            success = false;
            errorMsg = "The divisor cannot be zero";
        }
    } else {
        success = false;
        errorMsg = "Unknown operation type";
    }

    QJsonObject output;
    if (success) {
        QJsonArray operands;
        operands << a << b;
        output["operands"] = operands;

        output["operation"] = operation;
        output["result"] = result;
    } else {
        output["error"] = errorMsg;
        output["result"] = 0;
    }

    output["success"] = success;
    output["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate) + "Z";

    QJsonObject response;
    response["content"] = QJsonArray{QJsonObject{{"type", "text"},
                                                 {"text",
                                                  QString("Calculation result: %1 %2 %3 = %4")
                                                      .arg(a)
                                                      .arg(operation == "add"        ? "+"
                                                           : operation == "subtract" ? "-"
                                                           : operation == "multiply" ? "*"
                                                                                     : "/")
                                                      .arg(b)
                                                      .arg(result)}}};
    response["structuredContent"] = output;

    return response;
}
