/**
 * @file MCPResourceContentGenerator.cpp
 * @brief MCP resource content generator implementation
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPResourceContentGenerator.h"
#include <MCPLog.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>

QJsonObject MCPResourceContentGenerator::generateResourceContent(const QString &strMimeType, const QString &strFilePath, const QString &strUri)
{
    QJsonObject result;

    // Check if file exists
    QFileInfo fileInfo(strFilePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        MCP_CORE_LOG_WARNING() << "MCPResourceContentGenerator: File does not exist or is not a file:" << strFilePath;
        return result;
    }

    // Generate or use provided URI
    QString strResourceUri = strUri;
    if (strResourceUri.isEmpty()) {
        strResourceUri = generateUriFromFilePath(strFilePath);
    }

    result["uri"] = strResourceUri;

    // Read file content
    QJsonArray contents;
    QJsonObject contentObj;
    contentObj["uri"] = strResourceUri;
    contentObj["mimeType"] = strMimeType;

    if (isTextMimeType(strMimeType)) {
        // Text type, use text field
        QString strTextContent = readFileAsText(strFilePath);
        if (strTextContent.isEmpty() && fileInfo.size() > 0) {
            MCP_CORE_LOG_WARNING() << "MCPResourceContentGenerator: Text file read failed or is empty:" << strFilePath;
            return QJsonObject();
        }
        contentObj["text"] = strTextContent;
    } else {
        // Binary type, use blob field (Base64 encoded)
        QString strBase64Content = readFileAsBase64(strFilePath);
        if (strBase64Content.isEmpty()) {
            MCP_CORE_LOG_WARNING() << "MCPResourceContentGenerator: Binary file read failed:" << strFilePath;
            return QJsonObject();
        }
        contentObj["blob"] = strBase64Content;
    }

    contents.append(contentObj);
    result["contents"] = contents;

    MCP_CORE_LOG_DEBUG() << "MCPResourceContentGenerator: Successfully generated resource content - URI:" << strResourceUri << ", MIME type:" << strMimeType << ", File path:" << strFilePath;

    return result;
}

bool MCPResourceContentGenerator::isTextMimeType(const QString &strMimeType)
{
    // List of text type MIME types
    QString strLowerMimeType = strMimeType.toLower();

    // All types starting with text/ are text types
    if (strLowerMimeType.startsWith("text/")) {
        return true;
    }

    // Common text types
    static const QStringList commonTextMimeTypes = {
        "application/json",         "application/xml",           //
        "application/javascript",   "application/x-javascript",  //
        "application/ecmascript",   "application/x-ecmascript",  //
        "application/typescript",   "application/x-typescript",  //
        "application/x-sh",         "application/x-shellscript", //
        "application/x-python",     "application/x-c",           //
        "application/x-cpp",        "application/x-c++",         //
        "application/x-csharp",     "application/x-java",        //
        "application/x-html",       "application/x-css",         //
        "application/x-sql",        "application/x-yaml",        //
        "application/x-toml",       "application/x-markdown",    //
        "application/x-svg+xml",    "application/x-json",        //
        "application/x-ld+json",    "application/x-jsonld",      //
        "application/x-rtf",        "application/x-rtfd",        //
        "application/x-tex",        "application/x-latex",       //
        "application/x-postscript", "application/x-ps",          //
        "application/x-eps",
    };
    return commonTextMimeTypes.contains(strLowerMimeType);
}

QString MCPResourceContentGenerator::readFileAsText(const QString &strFilePath)
{
    QFile file(strFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        MCP_CORE_LOG_WARNING() << "MCPResourceContentGenerator: Cannot open text file:" << strFilePath << ", Error:" << file.errorString();
        return QString();
    }

    // Read with UTF-8 encoding
    QTextStream textStream(&file);
    textStream.setEncoding(QStringConverter::Utf8);
    QString strContent = textStream.readAll();
    file.close();

    return strContent;
}

QString MCPResourceContentGenerator::readFileAsBase64(const QString &strFilePath)
{
    QFile file(strFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        MCP_CORE_LOG_WARNING() << "MCPResourceContentGenerator: Cannot open binary file:" << strFilePath << ", Error:" << file.errorString();
        return QString();
    }

    QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        MCP_CORE_LOG_WARNING() << "MCPResourceContentGenerator: Binary file is empty:" << strFilePath;
        return QString();
    }

    return base64Encode(data);
}

QString MCPResourceContentGenerator::base64Encode(const QByteArray &data)
{
    return QString::fromLatin1(data.toBase64());
}

QString MCPResourceContentGenerator::generateUriFromFilePath(const QString &strFilePath)
{
    QFileInfo fileInfo(strFilePath);
    QString strAbsolutePath = fileInfo.absoluteFilePath();

    // Convert path to file:// protocol URI
    QUrl url = QUrl::fromLocalFile(strAbsolutePath);
    return url.toString();
}
