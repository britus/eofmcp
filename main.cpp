// ********************************************************************
// EoF MCP Server
// Copyright © 2025 by EoF Software Labs
// SPDX-License-Identifier: GPLv3
// ********************************************************************
#include "myresourcehandler.h"
#include "mysourcecodehandler.h"
#include <IMCPMiddleware.h>
#include <IMCPPromptService.h>
#include <IMCPResourceService.h>
#include <IMCPServer.h>
#include <IMCPServerConfig.h>
#include <IMCPToolService.h>
#include <IMCPTransport.h>
#include <MCPAutoServer.h>
#include <MCPLog.h>
#include <MCPServer.h>
#include <MCPServer_global.h>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QStandardPaths>
#include <QTranslator>

static MCPAutoServer autoServer;

static inline void StartAutoMCPServer()
{
    autoServer.performStart();
}

static inline void StopAutoMCPServer()
{
    autoServer.performStop();
}

static inline bool createDirectory(const QDir &dir)
{
    if (!dir.exists()) {
        QFile::Permissions permissions;
        permissions.setFlag(QFile::Permission::ReadOwner, true);
        permissions.setFlag(QFile::Permission::ReadGroup, true);
        permissions.setFlag(QFile::Permission::WriteOwner, true);
        permissions.setFlag(QFile::Permission::WriteGroup, true);
        permissions.setFlag(QFile::Permission::ExeOwner, true);
        permissions.setFlag(QFile::Permission::ExeGroup, true);
        if (!dir.mkpath(dir.absolutePath(), permissions)) {
            qWarning("Unable to create directory: %s", qPrintable(dir.absolutePath()));
            return false;
        }
    }
    return true;
}

#ifdef LIBMCPServer
QString preferencePath;
#endif

static inline bool createConfigDirectory(const QString &pathName, QDir &target)
{
#ifdef LIBMCPServer
    if (preferencePath.isEmpty()) {
        preferencePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    }
    QDir dir(preferencePath);
#else
    // server tools configuration
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
#endif

    // fallback to internal resource
    if (!QFileInfo::exists(dir.absoluteFilePath(pathName))) {
        if (!createDirectory(dir.absoluteFilePath(pathName))) {
            return false;
        }
    }

    target = QDir(dir.absoluteFilePath(pathName));
    return true;
}

// Helper function to copy directories recursively
static inline bool copyDirectoryRecursively(const QString &sourceDirPath, const QString &targetDirPath)
{
    QDir sourceDir(sourceDirPath);
    QDir targetDir(targetDirPath);

    if (!targetDir.exists()) {
        if (!targetDir.mkpath(".")) {
            return false;
        }
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList( //
        QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QFileInfo &fileInfo, fileInfoList) {
        QString fileName = fileInfo.fileName();
        QString sourceFilePath = fileInfo.absoluteFilePath();
        QString targetFilePath = targetDirPath + QDir::separator() + fileName;

        if (fileInfo.isDir()) {
            // Recursively copy subdirectory
            if (!copyDirectoryRecursively(sourceFilePath, targetFilePath)) {
                return false;
            }
        } else {
            // Copy file
            QFile sourceFile(sourceFilePath);
            QFile targetFile(targetFilePath);

            if (targetFile.exists()) {
                if (!targetFile.remove()) {
                    return false;
                }
            }

            if (!sourceFile.copy(targetFilePath)) {
                return false;
            }
        }
    }

    return true;
}

static inline bool deployResourceFiles(const QString &resourcePath, const QDir &targetDir)
{
    // Ensure target directory exists
    if (!targetDir.exists()) {
        if (!createDirectory(targetDir)) {
            qCritical() << "Failed to create target directory:" << targetDir.absolutePath();
            return false;
        }
    }

    bool success = true;

    // Iterate through each subdirectory
    QDir sourceDir(resourcePath);
    QFileInfo sourceInfo(sourceDir.absolutePath());

    // Check if the source subdirectory exists
    if (!sourceDir.exists()) {
        qCritical() << "Source subdirectory does not exist:" << sourceDir.absolutePath();
        return false;
    }

    // Get all files in the subdirectory
    QFileInfoList fileInfoList = sourceDir.entryInfoList( //
        QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QFileInfo &fileInfo, fileInfoList) {
        QString fileName = fileInfo.fileName();
        QString sourceFilePath = fileInfo.absoluteFilePath();
        QString targetFilePath = targetDir.absoluteFilePath(fileName);

        if (fileInfo.isDir()) {
            // Handle directories recursively
            QDir targetSubDir(targetFilePath);
            if (!targetSubDir.exists()) {
                if (!targetSubDir.mkpath(".")) {
                    qWarning() << "Failed to create directory:" << targetFilePath;
                    success = false;
                    continue;
                }
            }

            // Copy directory contents recursively
            if (!copyDirectoryRecursively(sourceFilePath, targetFilePath)) {
                qWarning() << "Failed to copy directory:" << sourceFilePath;
                success = false;
            }
        } else {
            // Handle files
            QFile sourceFile(sourceFilePath);
            QFileInfo sfi(sourceFile.fileName());
            QFile targetFile(targetFilePath);
            QFileInfo tfi(targetFile.fileName());

            // skip existing newer files
            if (sfi.lastModified() < tfi.lastModified()) {
                continue;
            }

            // Remove existing file if it exists
            if (targetFile.exists()) {
                if (!targetFile.remove()) {
                    qWarning() << "Failed to remove existing file:" << targetFilePath;
                    success = false;
                    continue;
                }
            }

            // Copy file
            if (!sourceFile.copy(targetFilePath)) {
                qWarning() << "Failed to copy file from" << sourceFilePath << "to" << targetFilePath;
                success = false;
            } else {
                qDebug() << "Copied file:" << fileName;
            }
        }
    }

    return success;
}

#ifdef LIBMCPServer
extern "C" {

int MCPCORE_EXPORT MCPServerQuit()
{
    if (qApp != nullptr) {
        qApp->quit();
    }

    return 0;
}

int MCPCORE_EXPORT MCPSetPreferencePath(const char *path)
{
    preferencePath = path;
    return 0;
}

int MCPCORE_EXPORT MCPServerStartup( //
    const char *appName,             //
    const char *displayName,
    const char *orgName,
    const char *orgDomain,
    const char *version)
{
    const char *argv[] = {"lib"};
    int argc = 0;

    // Same as in Info.plist
    QApplication::setApplicationName(appName ? appName : QStringLiteral("eofmcp"));
    QApplication::setApplicationDisplayName(displayName ? displayName : QStringLiteral("EoF MCP Server"));
    QApplication::setOrganizationName(orgName ? orgName : QStringLiteral("EoF Software Labs"));
    QApplication::setOrganizationDomain(orgDomain ? orgDomain : QStringLiteral("org.eof.tools.eofmcp"));
    QApplication::setApplicationVersion(version ? version : QStringLiteral("4.31.6"));

    QCoreApplication a(argc, (char **) argv);
    a.connect(&a, &QCoreApplication::aboutToQuit, &a, []() { //
        StopAutoMCPServer();
    });

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "eofmcp_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // deploy configuration files from internal resource
    QDir target;
    if (createConfigDirectory(QStringLiteral("Tools"), target)) {
        if (!deployResourceFiles(QStringLiteral(":/cfg/Tools"), target)) {
            qCritical("Unable to create configuration directory: Tools");
            return -1;
        }
    }
    if (createConfigDirectory(QStringLiteral("Prompts"), target)) {
        if (!deployResourceFiles(QStringLiteral(":/cfg/Prompts"), target)) {
            qCritical("Unable to create configuration directory: Prompts");
            return -1;
        }
    }
    if (createConfigDirectory(QStringLiteral("Resources"), target)) {
        if (!deployResourceFiles(QStringLiteral(":/cfg/Resources"), target)) {
            qCritical("Unable to create configuration directory: Resources");
            return -1;
        }
    }

    QObjectList handlers;

    // The source code handler has following invokable tools:
    // - displayProjectFiles <project_path>  [recursive] [sort_by]
    // - listSourceFiles <project_path>
    // - readSourceFile <file_path>
    // - writeSourceFile <file_path> <content> [create_backup]
    handlers.append(new SourceCodeHandler(qApp));

    // Create a resource Handler object (used for validating MCPResourceWrapper)
    // The resource Handler must be created; MCPHandlerResolver will locate it
    // via objectName or the "MCPResourceHandlerName" property
    // The "MCPResourceHandlerName" property is already set in the MyResourceHandler constructor
    handlers.append(new MyResourceHandler(qApp));

    // Use automatic startup to load and start the server from the configuration file
    // The configuration file is located in the config folder.
    StartAutoMCPServer();

    return a.exec();
}

} // extern "C"
#endif

#ifndef LIBMCPServer
int main(int argc, char *argv[])
{
    // Same as in Info.plist
    QApplication::setApplicationName(QStringLiteral("eofmcp"));
    QApplication::setApplicationDisplayName(QStringLiteral("EoF MCP Server"));
    QApplication::setOrganizationName(QStringLiteral("EoF Software Labs"));
    QApplication::setOrganizationDomain(QStringLiteral("org.eof.tools.eofmcp"));

#ifndef XCODE_BUILD
    QApplication::setApplicationVersion("4.31.6");
#else
    QApplication::setApplicationVersion( //
        QStringLiteral("%1.%2").arg(getBundleVersion(), getBuildNumber()));
#endif

    QCoreApplication a(argc, argv);
    a.connect(&a, &QCoreApplication::aboutToQuit, &a, []() { //
        StopAutoMCPServer();
    });

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "eofmcp_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // deploy configuration files from internal resource
    QDir target;
    if (createConfigDirectory(QStringLiteral("Tools"), target)) {
        if (!deployResourceFiles(QStringLiteral(":/cfg/Tools"), target)) {
            qCritical("Unable to create configuration directory: Tools");
            return -1;
        }
    }
    if (createConfigDirectory(QStringLiteral("Prompts"), target)) {
        if (!deployResourceFiles(QStringLiteral(":/cfg/Prompts"), target)) {
            qCritical("Unable to create configuration directory: Prompts");
            return -1;
        }
    }
    if (createConfigDirectory(QStringLiteral("Resources"), target)) {
        if (!deployResourceFiles(QStringLiteral(":/cfg/Resources"), target)) {
            qCritical("Unable to create configuration directory: Resources");
            return -1;
        }
    }

    QObjectList handlers;

    // The source code handler has following invokable tools:
    // - displayProjectFiles <project_path>  [recursive] [sort_by]
    // - listSourceFiles <project_path>
    // - readSourceFile <file_path>
    // - writeSourceFile <file_path> <content> [create_backup]
    handlers.append(new SourceCodeHandler(qApp));

    // Create a resource Handler object (used for validating MCPResourceWrapper)
    // The resource Handler must be created; MCPHandlerResolver will locate it
    // via objectName or the "MCPResourceHandlerName" property
    // The "MCPResourceHandlerName" property is already set in the MyResourceHandler constructor
    handlers.append(new MyResourceHandler(qApp));

    // Use automatic startup to load and start the server from the configuration file
    // The configuration file is located in the config folder.
    StartAutoMCPServer();

    return a.exec();
}
#endif
