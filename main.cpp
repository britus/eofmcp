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

void StartAutoMCPServer()
{
    autoServer.performStart();
    autoServer.generateResources("/Users/eofmc/EoF/eofmcp");
    autoServer.loadMcpToolset();
}

void StopAutoMCPServer()
{
    autoServer.performStop();
}

int main(int argc, char *argv[])
{
    // Same as in Info.plist
    QCoreApplication::setOrganizationDomain("org.eof.tools");
    QCoreApplication::setApplicationName("EoF MCP Server");

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
    handlers.append(new MyResourceHandler(qApp));

    // Use automatic startup to load and start the server from the configuration file
    // The configuration file is located in the MCPServerConfig folder within the application directory
    StartAutoMCPServer();

    return a.exec();
}
