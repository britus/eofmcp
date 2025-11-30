/**
 * @file SourceCodeHandler.cpp
 * @brief Implementierung des Source-Code Handler
 * @author Your Name
 * @date 2025-11-30
 * @copyright Copyright (c) 2025 Your Company. All rights reserved.
 */

#include "SourceCodeHandler.h"
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTextCodec>
#include <QJsonDocument>
#include <QStandardPaths>
#include <algorithm>

SourceCodeHandler::SourceCodeHandler(QObject* pParent)
    : QObject(pParent)
{
}

SourceCodeHandler::~SourceCodeHandler()
{
}

QJsonObject SourceCodeHandler::listSourceFiles(const QJsonObject& jsonInput)
{
    if (!jsonInput.contains("project_path"))
    {
        return createErrorResponse("Parameter 'project_path' erforderlich");
    }

    QString strProjectPath = jsonInput.value("project_path").toString();

    QJsonArray jsonExtArray = jsonInput.value("extensions").toArray();
    QStringList strExtensions = getFileExtensions(jsonExtArray);

    if (!isValidPath(strProjectPath))
    {
        return createErrorResponse(
            QString("Ungültiger Projektpfad: %1").arg(strProjectPath)
        );
    }

    QList<QFileInfo> fileList = findSourceFiles(strProjectPath, strExtensions, true);

    QJsonObject jsonResponse;
    QJsonArray jsonFiles;

    for (const QFileInfo& fileInfo : fileList)
    {
        jsonFiles.append(fileInfoToJson(fileInfo, strProjectPath));
    }

    jsonResponse["files"] = jsonFiles;
    jsonResponse["total_files"] = static_cast<int>(fileList.size());
    jsonResponse["project_path"] = strProjectPath;

    return jsonResponse;
}

QJsonObject SourceCodeHandler::readSourceFile(const QJsonObject& jsonInput)
{
    if (!jsonInput.contains("file_path"))
    {
        return createErrorResponse("Parameter 'file_path' erforderlich");
    }

    QString strFilePath = jsonInput.value("file_path").toString();

    if (!isValidPath(strFilePath))
    {
        return createErrorResponse(
            QString("Ungültiger Dateipfad: %1").arg(strFilePath)
        );
    }

    QFile file(strFilePath);

    if (!file.exists())
    {
        return createErrorResponse(
            QString("Datei nicht gefunden: %1").arg(strFilePath)
        );
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return createErrorResponse(
            QString("Datei konnte nicht geöffnet werden: %1").arg(strFilePath)
        );
    }

    QByteArray byteContent = file.readAll();
    file.close();

    QString strContent = QString::fromUtf8(byteContent);

    int iLineCount = strContent.count('\n') + (strContent.isEmpty() ? 0 : 1);

    QJsonObject jsonResponse;
    jsonResponse["file_path"] = strFilePath;
    jsonResponse["content"] = strContent;
    jsonResponse["encoding"] = "UTF-8";
    jsonResponse["line_count"] = iLineCount;
    jsonResponse["size"] = static_cast<int>(byteContent.size());

    return jsonResponse;
}

QJsonObject SourceCodeHandler::writeSourceFile(const QJsonObject& jsonInput)
{
    if (!jsonInput.contains("file_path"))
    {
        return createErrorResponse("Parameter 'file_path' erforderlich");
    }

    if (!jsonInput.contains("content"))
    {
        return createErrorResponse("Parameter 'content' erforderlich");
    }

    QString strFilePath = jsonInput.value("file_path").toString();
    QString strContent = jsonInput.value("content").toString();
    bool bCreateBackup = jsonInput.value("create_backup", true).toBool();

    if (!isValidPath(strFilePath))
    {
        return createErrorResponse(
            QString("Ungültiger Dateipfad: %1").arg(strFilePath)
        );
    }

    QJsonObject jsonResponse;
    jsonResponse["file_path"] = strFilePath;
    jsonResponse["success"] = false;

    QString strBackupPath;
    if (bCreateBackup && QFile::exists(strFilePath))
    {
        strBackupPath = createBackup(strFilePath);
        if (!strBackupPath.isEmpty())
        {
            jsonResponse["backup_path"] = strBackupPath;
        }
    }

    QFile file(strFilePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        jsonResponse["message"] = QString(
            "Fehler: Datei konnte nicht geschrieben werden - %1"
        ).arg(file.errorString());
        return jsonResponse;
    }

    QByteArray byteContent = strContent.toUtf8();
    qint64 iBytesWritten = file.write(byteContent);
    file.close();

    if (iBytesWritten == -1)
    {
        jsonResponse["message"] = "Fehler beim Schreiben der Datei";
        return jsonResponse;
    }

    jsonResponse["success"] = true;
    jsonResponse["bytes_written"] = static_cast<int>(iBytesWritten);
    jsonResponse["message"] = QString(
        "Datei erfolgreich gespeichert - %1 Bytes geschrieben"
    ).arg(iBytesWritten);

    return jsonResponse;
}

QJsonObject SourceCodeHandler::displayProjectFiles(const QJsonObject& jsonInput)
{
    if (!jsonInput.contains("project_path"))
    {
        return createErrorResponse("Parameter 'project_path' erforderlich");
    }

    QString strProjectPath = jsonInput.value("project_path").toString();
    bool bRecursive = jsonInput.value("recursive", true).toBool();
    QString strSortBy = jsonInput.value("sort_by", "name").toString();

    if (!isValidPath(strProjectPath))
    {
        return createErrorResponse(
            QString("Ungültiger Projektpfad: %1").arg(strProjectPath)
        );
    }

    QStringList strExtensions;
    for (const auto& ext : DEFAULT_EXTENSIONS)
    {
        strExtensions.append(ext);
    }

    QList<QFileInfo> fileList = findSourceFiles(
        strProjectPath,
        strExtensions,
        bRecursive
    );

    if (strSortBy == "size")
    {
        std::sort(fileList.begin(), fileList.end(),
            [](const QFileInfo& a, const QFileInfo& b)
            {
                return a.size() < b.size();
            }
        );
    }
    else if (strSortBy == "date")
    {
        std::sort(fileList.begin(), fileList.end(),
            [](const QFileInfo& a, const QFileInfo& b)
            {
                return a.lastModified() < b.lastModified();
            }
        );
    }
    else
    {
        std::sort(fileList.begin(), fileList.end(),
            [](const QFileInfo& a, const QFileInfo& b)
            {
                return a.fileName() < b.fileName();
            }
        );
    }

    QJsonObject jsonResponse;
    QJsonArray jsonFiles;
    QSet<QString> directories;
    qint64 iTotalSize = 0;

    for (const QFileInfo& fileInfo : fileList)
    {
        jsonFiles.append(fileInfoToJson(fileInfo, strProjectPath));
        directories.insert(fileInfo.path());
        iTotalSize += fileInfo.size();
    }

    jsonResponse["files"] = jsonFiles;

    QJsonObject jsonSummary;
    jsonSummary["total_files"] = static_cast<int>(fileList.size());
    jsonSummary["total_size"] = static_cast<int>(iTotalSize);
    jsonSummary["directories"] = static_cast<int>(directories.size());

    jsonResponse["summary"] = jsonSummary;

    return jsonResponse;
}

QList<QFileInfo> SourceCodeHandler::findSourceFiles(
    const QString& strPath,
    const QStringList& strExtensions,
    bool bRecursive)
{
    QList<QFileInfo> fileList;
    QDir dir(strPath);

    if (!dir.exists())
    {
        return fileList;
    }

    QDir::Filters filters = QDir::Files | QDir::Readable;

    if (bRecursive)
    {
        filters |= QDir::AllDirs;
    }

    dir.setFilter(filters);

    QFileInfoList dirList = dir.entryInfoList();

    for (const QFileInfo& fileInfo : dirList)
    {
        if (fileInfo.isDir())
        {
            if (bRecursive && fileInfo.fileName() != "." && fileInfo.fileName() != "..")
            {
                fileList.append(findSourceFiles(fileInfo.filePath(), strExtensions, bRecursive));
            }
        }
        else
        {
            for (const QString& strExt : strExtensions)
            {
                if (fileInfo.suffix() == strExt.mid(1))
                {
                    fileList.append(fileInfo);
                    break;
                }
            }
        }
    }

    return fileList;
}

bool SourceCodeHandler::isValidPath(const QString& strPath)
{
    if (strPath.isEmpty())
    {
        return false;
    }

    QFileInfo fileInfo(strPath);
    QString strAbsPath = fileInfo.absoluteFilePath();

    return !strAbsPath.isEmpty();
}

QString SourceCodeHandler::createBackup(const QString& strOriginalPath)
{
    QFileInfo fileInfo(strOriginalPath);
    QString strBackupDir = fileInfo.absolutePath();
    QString strBackupName = fileInfo.baseName() + "_backup_" +
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") +
        "." + fileInfo.suffix();
    QString strBackupPath = strBackupDir + "/" + strBackupName;

    if (QFile::copy(strOriginalPath, strBackupPath))
    {
        return strBackupPath;
    }

    return QString();
}

QJsonObject SourceCodeHandler::fileInfoToJson(
    const QFileInfo& fileInfo,
    const QString& strBaseDir)
{
    QJsonObject jsonFileInfo;
    jsonFileInfo["path"] = fileInfo.absoluteFilePath();

    if (!strBaseDir.isEmpty())
    {
        QDir baseDir(strBaseDir);
        jsonFileInfo["relative_path"] = baseDir.relativeFilePath(fileInfo.absoluteFilePath());
    }

    jsonFileInfo["size"] = static_cast<int>(fileInfo.size());
    jsonFileInfo["last_modified"] = fileInfo.lastModified().toString(Qt::ISODate);
    jsonFileInfo["directory"] = fileInfo.absolutePath();

    return jsonFileInfo;
}

QStringList SourceCodeHandler::getFileExtensions(const QJsonArray& jsonArray)
{
    QStringList strExtensions;

    if (!jsonArray.isEmpty())
    {
        for (const QJsonValue& value : jsonArray)
        {
            if (value.isString())
            {
                QString strExt = value.toString();
                if (!strExt.startsWith("."))
                {
                    strExt = "." + strExt;
                }
                strExtensions.append(strExt);
            }
        }
    }
    else
    {
        for (const auto& ext : DEFAULT_EXTENSIONS)
        {
            strExtensions.append(ext);
        }
    }

    return strExtensions;
}

QJsonObject SourceCodeHandler::createErrorResponse(const QString& strErrorMsg)
{
    QJsonObject jsonError;
    jsonError["success"] = false;
    jsonError["error"] = strErrorMsg;
    return jsonError;
}