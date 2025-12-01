// ********************************************************************
// EoF MCP Server
// Copyright © 2025 by EoF Software Labs
// SPDX-License-Identifier: GPLv3
// ********************************************************************
#include "mycalculatorHandler.h"
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
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QStandardPaths>
#include <QTranslator>

static MCPAutoServer autoServer;
static QTimer toolsTimer;

void StartAutoMCPServer()
{
    autoServer.performStart();
}

void StopAutoMCPServer()
{
    autoServer.performStop();
}

// TODO: Seperate to directory/fileWatch class
void LoadMcpToolset()
{
    QString toolsCfgPath;

    toolsCfgPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    toolsCfgPath = toolsCfgPath + QDir::separator() + "Tools";

    QFile::Permissions permissions;
    permissions.setFlag(QFile::Permission::ReadOwner, true);
    permissions.setFlag(QFile::Permission::ReadGroup, true);
    permissions.setFlag(QFile::Permission::WriteOwner, true);
    permissions.setFlag(QFile::Permission::WriteGroup, true);
    permissions.setFlag(QFile::Permission::ExeOwner, true);
    permissions.setFlag(QFile::Permission::ExeGroup, true);

    QDir toolDir(toolsCfgPath);
    if (!toolDir.exists()) {
        if (!toolDir.mkpath(toolsCfgPath, permissions)) {
            MCP_TOOLS_LOG_CRITICAL() << "Unable to create tools directory" << toolsCfgPath << "abort.";
            return;
        }
    }

    // propagate MCP toolset, 5 sec delay
    toolsTimer.setInterval(5000);
    toolsTimer.setSingleShot(true);
    toolsTimer.connect(&toolsTimer, &QTimer::timeout, [toolDir]() {
        // get tool configuration files
        QFileInfoList files = toolDir.entryInfoList(QStringList() << "*.json");
        foreach (auto fi, files) {
            if (fi.isFile() && fi.isReadable()) {
                autoServer.loadTool(fi.absoluteFilePath().toUtf8().constData());
            }
        }
    });
}

static inline void createResources(QObjectList *handlers, const QDir &projectDir, bool bRecursive = true)
{
    QStringList strExtensions = QStringList() //
                                << ".cpp"     //
                                << ".h"       //
                                << ".hpp"     //
                                << ".c"       //
                                << ".cc"      //
                                << ".cxx"     //
                                << ".hxx"     //
                                << ".java";
    QDir::Filters filters = QDir::Files | QDir::Readable;

    if (bRecursive) {
        filters |= QDir::AllDirs;
    }

    QFileInfoList dirList = projectDir.entryInfoList();
    foreach (const QFileInfo &fileInfo, dirList) {
        if (fileInfo.isDir()) {
            if (bRecursive                           //
                && fileInfo.fileName() != "."        //
                && fileInfo.fileName() != ".."       //
                && fileInfo.fileName() != "build"    // QT/CMAKE build directory
                && fileInfo.fileName() != "bin"      // Java binaries
                && fileInfo.fileName() != "classes") // Java binaries
            {
                createResources(handlers, fileInfo.filePath(), bRecursive);
            }
        } else {
            foreach (const QString &strExt, strExtensions) {
                if (fileInfo.suffix() == strExt.mid(1)) { // skip dot
                    handlers->append(new MyResourceHandler(fileInfo, qApp));
                    break;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    // Same as in Info.plist
    QCoreApplication::setOrganizationDomain("org.eof.tools");
    QCoreApplication::setApplicationName("EoF MCP Server");

    QCoreApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "eofmcp_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    QObjectList handlers;

    // Create a Handler object (used for handling tool calls)
    // The Handler must be created; MCPHandlerResolver will locate it
    // via objectName or the "MPCToolHandlerName" property
    // The "MPCToolHandlerName" property is already set in the MyCalculatorHandler constructor
    handlers.append(new MyCalculatorHandler(qApp));

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
    createResources(&handlers,
                    QDir(QStandardPaths::writableLocation( //
                             QStandardPaths::HomeLocation)
                         + "/EoF/eofmcp"));

    // Load supported MCP toolset
    LoadMcpToolset();

    // Use automatic startup to load and start the server from the configuration file
    // The configuration file is located in the MCPServerConfig folder within the application directory
    StartAutoMCPServer();

    return a.exec();
}
