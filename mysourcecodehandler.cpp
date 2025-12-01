// ********************************************************************
// EoF MCP Server
// Copyright © 2025 by EoF Software Labs
// SPDX-License-Identifier: GPLv3
// ********************************************************************
#include "mysourcecodehandler.h"
#include <algorithm>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QTextCodec>

SourceCodeHandler::SourceCodeHandler(QObject *pParent)
    : QObject(pParent)
{
    // Set the MCPResourceHandlerName property so that
    // MCPHandlerResolver can locate this object.
    setProperty("MPCToolHandlerName", SourceCodeHandler::metaObject()->className());

    // Set objectName as a fallback identifier
    setObjectName(SourceCodeHandler::metaObject()->className());
}

SourceCodeHandler::~SourceCodeHandler() {}

QJsonObject SourceCodeHandler::listSourceFiles(const QVariant &project_path)
{
    if (!project_path.isValid()) {
        return createErrorResponse("Parameter 'project_path' required");
    }

    QString strProjectPath = project_path.toString();

    QJsonArray jsonExtArray = {}; //jsonInput.value("extensions").toArray();
    QStringList strExtensions = getFileExtensions(jsonExtArray);

    if (!isValidPath(strProjectPath)) {
        return createErrorResponse(QString("Invalid project path: %1").arg(strProjectPath));
    }

    QList<QFileInfo> fileList = findSourceFiles(strProjectPath, strExtensions, true);

    QJsonObject jsonResponse;
    QJsonArray jsonFiles;

    foreach (const QFileInfo &fileInfo, fileList) {
        jsonFiles.append(fileInfoToJson(fileInfo, strProjectPath));
    }

    // result
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate) + "Z";
    QJsonObject structContent = QJsonObject({
        QPair<QString, QJsonValue>("files", jsonFiles),
        QPair<QString, QJsonValue>("total_files", QJsonValue(static_cast<int>(fileList.size()))),
        QPair<QString, QJsonValue>("project_path", QJsonValue(strProjectPath)),
    });

    QJsonDocument doc = QJsonDocument(structContent);
    QJsonObject textResult = QJsonObject({
        QPair<QString, QString>("type", "text"), //
        QPair<QString, QString>("text", doc.toJson()),
    });

    QJsonObject response = QJsonObject({
        QPair<QString, QJsonValue>("structuredContent", structContent),
        QPair<QString, QJsonValue>("content", QJsonArray({textResult})),
    });

    return response;
}

QJsonObject SourceCodeHandler::readSourceFile(const QVariant &file_path)
{
    if (!file_path.isValid()) {
        return createErrorResponse("Parameter 'file_path' required");
    }

    QString strFilePath = file_path.toString();

    if (!isValidPath(strFilePath)) {
        return createErrorResponse(QString("Invalid file path: %1").arg(strFilePath));
    }

    QFile file(strFilePath);

    if (!file.exists()) {
        return createErrorResponse(QString("File not found: %1").arg(strFilePath));
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return createErrorResponse(QString("File could not be opened: %1").arg(strFilePath));
    }

    QByteArray byteContent = file.readAll();
    file.close();

    QString strContent = QString::fromUtf8(byteContent);

    int iLineCount = strContent.count('\n') + (strContent.isEmpty() ? 0 : 1);

    QJsonObject structContent;
    structContent["file_path"] = strFilePath;
    structContent["content"] = strContent;
    structContent["encoding"] = "UTF-8";
    structContent["line_count"] = iLineCount;
    structContent["size"] = static_cast<int>(byteContent.size());

    // result
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate) + "Z";

    QJsonDocument doc = QJsonDocument(structContent);
    QJsonObject textResult = QJsonObject({
        QPair<QString, QString>("type", "text"),       //
        QPair<QString, QString>("text", doc.toJson()), //
    });

    QJsonObject response = QJsonObject({
        QPair<QString, QJsonValue>("structuredContent", structContent),
        QPair<QString, QJsonValue>("content", QJsonArray({textResult})),
    });

    return response;
}

QJsonObject SourceCodeHandler::writeSourceFile(const QVariant &file_path, const QVariant &content, const QVariant &create_backup)
{
    if (!file_path.isValid()) {
        return createErrorResponse("Parameter 'file_path' required");
    }

    if (!content.isValid()) {
        return createErrorResponse("Parameter 'content' required");
    }

    QString strFilePath = file_path.toString();
    QString strContent = content.toString();
    bool bCreateBackup = create_backup.toBool();

    if (!isValidPath(strFilePath)) {
        return createErrorResponse(QString("Invalid file path: %1").arg(strFilePath));
    }

    /*
    "file_path": "[~]/eofmcp/changed/server/IMCPMiddleware.h",
    "message": "Error: File could not be written - No such file or directory",
    "success": false
    */
    QFileInfo fi(strFilePath);
    QFile::Permissions permissions;
    permissions.setFlag(QFile::Permission::ReadOwner, true);
    permissions.setFlag(QFile::Permission::ReadGroup, true);
    permissions.setFlag(QFile::Permission::WriteOwner, true);
    permissions.setFlag(QFile::Permission::WriteGroup, true);
    permissions.setFlag(QFile::Permission::ExeOwner, true);
    permissions.setFlag(QFile::Permission::ExeGroup, true);
    QDir tdir(fi.path());
    if (!tdir.exists()) {
        tdir.mkpath(fi.path(), permissions);
    }

    QJsonObject info;
    info["file_path"] = strFilePath;
    info["success"] = false;

    QString strBackupPath;
    if (bCreateBackup && QFile::exists(strFilePath)) {
        strBackupPath = createBackup(strFilePath);
        if (!strBackupPath.isEmpty()) {
            info["backup_path"] = strBackupPath;
        }
    }

    QFile file(strFilePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        info["message"] = QString( //
                              "Error: File could not be written - %1")
                              .arg(file.errorString());
        return info;
    }

    QByteArray byteContent = strContent.toUtf8();
    qint64 iBytesWritten = file.write(byteContent);
    file.close();

    if (iBytesWritten == -1) {
        info["message"] = "Error writing the file";
        return info;
    }

    info["success"] = true;
    info["bytes_written"] = static_cast<int>(iBytesWritten);
    info["message"] = QString( //
                          "File successfully saved - %1 Bytes written")
                          .arg(iBytesWritten);

    // result
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate) + "Z";

    QJsonDocument doc = QJsonDocument(info);
    QJsonArray resp_content = QJsonArray({QJsonObject({
        QPair<QString, QString>("type", "text"), //
        QPair<QString, QString>("text", doc.toJson()),
    })});

    QJsonObject response = QJsonObject({
        QPair<QString, QJsonValue>("structuredContent", info),
        QPair<QString, QJsonValue>("content", resp_content),
    });

    return response;
}

QJsonObject SourceCodeHandler::displayProjectFiles(const QVariant &project_path, const QVariant &recursive, const QVariant &sort_by)
{
    if (!project_path.isValid()) {
        return createErrorResponse("Parameter 'project_path' required");
    }

    QString strProjectPath = project_path.toString();
    QString strSortBy = sort_by.isValid() ? sort_by.toString() : "name";
    bool bRecursive = recursive.isValid() ? recursive.toBool() : true;

    if (!isValidPath(strProjectPath)) {
        return createErrorResponse(QString("Invalid project path: %1").arg(strProjectPath));
    }

    QStringList strExtensions;
    for (const auto &ext : DEFAULT_EXTENSIONS) {
        strExtensions.append(ext);
    }

    QList<QFileInfo> fileList = findSourceFiles(strProjectPath, strExtensions, bRecursive);

    if (strSortBy == "size") {
        std::sort(fileList.begin(), fileList.end(), [](const QFileInfo &a, const QFileInfo &b) { //
            return a.size() < b.size();
        });
    } else if (strSortBy == "date") {
        std::sort(fileList.begin(), fileList.end(), [](const QFileInfo &a, const QFileInfo &b) { //
            return a.lastModified() < b.lastModified();
        });
    } else {
        std::sort(fileList.begin(), fileList.end(), [](const QFileInfo &a, const QFileInfo &b) { //
            return a.fileName() < b.fileName();
        });
    }

    QJsonObject structContent;
    QJsonArray jsonFiles;
    QSet<QString> directories;
    QStringList textLines;
    qint64 iTotalSize = 0;
    auto toTextLine = [](const QFileInfo &fileInfo, const QString &strBaseDir) -> QString { //
        QString filePath = fileInfo.absoluteFilePath();
        QString relativePath = ".";

        if (!strBaseDir.isEmpty()) {
            QDir baseDir(strBaseDir);
            relativePath = baseDir.relativeFilePath(fileInfo.absoluteFilePath());
        }

        QString size = QString::number(static_cast<int>(fileInfo.size()));
        QString lastModified = fileInfo.lastModified().toString(Qt::ISODate);
        QString directory = fileInfo.absolutePath();

        return QStringLiteral("%1|%2|%3|%4|%5").arg(filePath, size, lastModified, directory, relativePath);
    };
    foreach (const QFileInfo &fileInfo, fileList) {
        jsonFiles.append(fileInfoToJson(fileInfo, strProjectPath));
        textLines.append(toTextLine(fileInfo, strProjectPath));
        directories.insert(fileInfo.path());
        iTotalSize += fileInfo.size();
    }
    structContent["files"] = jsonFiles;

    QJsonObject jsonSummary;
    jsonSummary["total_files"] = static_cast<int>(fileList.size());
    jsonSummary["total_size"] = static_cast<int>(iTotalSize);
    jsonSummary["directories"] = static_cast<int>(directories.size());
    structContent["summary"] = jsonSummary;

    // result
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate) + "Z";

    QJsonDocument doc = QJsonDocument(jsonFiles);
    QJsonArray resultText = QJsonArray({QJsonObject({
        QPair<QString, QString>("type", "text"), //
        QPair<QString, QString>("text", doc.toJson(QJsonDocument::Compact) /*textLines.join("\n")*/),
    })});

    QJsonObject response = QJsonObject({
        QPair<QString, QJsonValue>("structuredContent", structContent),
        QPair<QString, QJsonValue>("content", QJsonArray({resultText})),
    });

    return response;
}

QList<QFileInfo> SourceCodeHandler::findSourceFiles(const QString &strPath, const QStringList &strExtensions, bool bRecursive)
{
    QList<QFileInfo> fileList;
    QDir dir(strPath);

    if (!dir.exists()) {
        return fileList;
    }

    QDir::Filters filters = QDir::Files | QDir::Readable;

    if (bRecursive) {
        filters |= QDir::AllDirs;
    }

    dir.setFilter(filters);

    QFileInfoList dirList = dir.entryInfoList();

    foreach (const QFileInfo &fileInfo, dirList) {
        if (fileInfo.isDir()) {
            if (bRecursive && fileInfo.fileName() != "." && fileInfo.fileName() != "..") {
                fileList.append(findSourceFiles(fileInfo.filePath(), strExtensions, bRecursive));
            }
        } else {
            for (const QString &strExt : strExtensions) {
                if (fileInfo.suffix() == strExt.mid(1)) {
                    fileList.append(fileInfo);
                    break;
                }
            }
        }
    }

    return fileList;
}

bool SourceCodeHandler::isValidPath(const QString &strPath)
{
    if (strPath.isEmpty()) {
        return false;
    }

    QFileInfo fileInfo(strPath);
    QString strAbsPath = fileInfo.absoluteFilePath();

    return !strAbsPath.isEmpty();
}

QString SourceCodeHandler::createBackup(const QString &strOriginalPath)
{
    QFileInfo fileInfo(strOriginalPath);
    QString strBackupDir = fileInfo.absolutePath();
    QString strBackupName = fileInfo.baseName() + "_backup_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + "." + fileInfo.suffix();
    QString strBackupPath = strBackupDir + "/" + strBackupName;

    if (QFile::copy(strOriginalPath, strBackupPath)) {
        return strBackupPath;
    }

    return QString();
}

QJsonObject SourceCodeHandler::fileInfoToJson(const QFileInfo &fileInfo, const QString &strBaseDir)
{
    QJsonObject jsonFileInfo;
    jsonFileInfo["path"] = fileInfo.absoluteFilePath();

    if (!strBaseDir.isEmpty()) {
        QDir baseDir(strBaseDir);
        jsonFileInfo["relative_path"] = baseDir.relativeFilePath(fileInfo.absoluteFilePath());
    }

    jsonFileInfo["size"] = static_cast<int>(fileInfo.size());
    jsonFileInfo["last_modified"] = fileInfo.lastModified().toString(Qt::ISODate);
    jsonFileInfo["directory"] = fileInfo.absolutePath();

    return jsonFileInfo;
}

QStringList SourceCodeHandler::getFileExtensions(const QJsonArray &jsonArray)
{
    QStringList strExtensions;

    if (!jsonArray.isEmpty()) {
        for (const QJsonValue &value : jsonArray) {
            if (value.isString()) {
                QString strExt = value.toString();
                if (!strExt.startsWith(".")) {
                    strExt = "." + strExt;
                }
                strExtensions.append(strExt);
            }
        }
    } else {
        for (const auto &ext : DEFAULT_EXTENSIONS) {
            strExtensions.append(ext);
        }
    }

    return strExtensions;
}

QJsonObject SourceCodeHandler::createErrorResponse(const QString &strErrorMsg)
{
    QJsonObject jsonError;
    jsonError["success"] = false;
    jsonError["error"] = strErrorMsg;
    return jsonError;
}
