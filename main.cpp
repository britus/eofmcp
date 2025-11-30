#include "mcpmainwindow.h"
#include "mycalculatorHandler.h"
#include "myresourcehandler.h"
#include "mysourcescraper.h"
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
#include <QLocale>
#include <QTranslator>

static MCPAutoServer autoServer;

void StartAutoMCPServer()
{
    autoServer.performStart();
}

void StopAutoMCPServer()
{
    autoServer.performStop();
}

void LoadAutoMCPServerTool(QObject * /*sender*/, const char *szToolConfigFile)
{
    //MCP_CORE_LOG_INFO() << "[MAIN] LoadAutoMCPServerTool: szToolConfigFile:" << szToolConfigFile;
    autoServer.loadTool(szToolConfigFile);
}

int main(int argc, char *argv[])
{
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

    // Create a resource Handler object (used for validating MCPResourceWrapper)
    // The resource Handler must be created; MCPHandlerResolver will locate it
    // via objectName or the "MCPResourceHandlerName" property
    // The "MCPResourceHandlerName" property is already set in the MyResourceHandler constructor
    handlers.append(new MyResourceHandler(qApp));

    // My source scraper handler
    handlers.append(new MySourceScraper(qApp));

    // Use automatic startup to load and start the server from the configuration file
    // The configuration file is located in the MCPServerConfig folder within the application directory
    // --
    StartAutoMCPServer();

    //MCPMainWindow w;
    //w.show();

    return a.exec();
}
