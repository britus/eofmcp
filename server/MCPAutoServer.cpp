/**
 * @file MCPAutoServer.cpp
 * @brief MCP Auto Server Implementation
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */
#ifdef Q_OS_WINDOWS
#include <windows.h>
#endif

#include <IMCPServer.h>
#include <IMCPToolService.h>
#include <MCPAutoServer.h>
#include <MCPHandlerResolver.h>
#include <MCPInvokeHelper.h>
#include <MCPLog.h>
#include <MCPServer.h>
#include <MCPServerConfig.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QMap>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QTimer>

MCPAutoServer::MCPAutoServer(QObject *pParent)
    : QObject(pParent)
    , m_pServer(nullptr)
{
    //-
}

MCPAutoServer::~MCPAutoServer() {}

void MCPAutoServer::performStart()
{
    MCP_CORE_LOG_INFO() << "MCPAutoServer: performStart...";
    m_pServer = IMCPServer::createServer();
}

void MCPAutoServer::performStop()
{
    MCP_CORE_LOG_INFO() << "MCPAutoServer: performStop...";
    if (m_pServer != nullptr) {
        IMCPServer::destroyServer(m_pServer);
        m_pServer = nullptr;
    }
}

void MCPAutoServer::loadTool(const QString &strToolConfigFile)
{
    if (m_pServer == nullptr) {
        return;
    }

    QFile file(strToolConfigFile);
    if (!file.open(QIODevice::ReadOnly)) {
        MCP_CORE_LOG_WARNING() << "MCPAutoServer: Unable to open file: " << strToolConfigFile;
        return;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError error;
    auto jsonTool = QJsonDocument::fromJson(jsonData, &error).object();
    if (error.error != QJsonParseError::NoError) {
        MCP_CORE_LOG_WARNING() << "MCPAutoServer: Failed to parse JSON file: " //
                               << strToolConfigFile << "\n"
                               << error.errorString();
        return;
    }

    MCP_CORE_LOG_DEBUG().noquote() << "MCPAutoServer: Register tool:" << strToolConfigFile;

    MCPInvokeHelper::asynInvoke(m_pServer, [this, jsonTool]() { //
        if (!m_pServer->getToolService()->addFromJson(jsonTool)) {
            MCP_CORE_LOG_WARNING() << "MCPAutoServer: Failed to add tool: " //
                                   << jsonTool;
        }
    });
}

static inline QList<QFileInfo> findFileResources(const QString basePath, const QStringList &extensions, bool bRecursive)
{
    QList<QFileInfo> fileList;
    QDir dir(basePath);

    if (!dir.exists()) {
        return {};
    }

    QDir::Filters filters = QDir::Files | QDir::Readable;

    if (bRecursive) {
        filters |= QDir::AllDirs;
    }

    dir.setFilter(filters);

    QFileInfoList dirList = dir.entryInfoList();

    foreach (const QFileInfo &fileInfo, dirList) {
        if (fileInfo.isDir()) {
            if (bRecursive                           //
                && fileInfo.fileName() != "."        //
                && fileInfo.fileName() != ".."       //
                && fileInfo.fileName() != "build"    // QT/CMAKE build directory
                && fileInfo.fileName() != "bin"      // Java binaries
                && fileInfo.fileName() != "classes") // Java binaries
            {
                fileList.append(findFileResources(fileInfo.filePath(), extensions, bRecursive));
            }
        } else {
            foreach (const QString &strExt, extensions) {
                if (fileInfo.suffix() == strExt.mid(1)) {
                    fileList.append(fileInfo);
                    break;
                }
            }
        }
    }

    return fileList;
}

void MCPAutoServer::generateResources(const QString basePath, bool bRecursive)
{
    MCP_CORE_LOG_INFO() << "MCPAutoServer: generateResources...";

    // Standard file extensions for source code
    const QStringList DEFAULT_EXTENSIONS = QStringList() //
                                           << ".cpp"     //
                                           << ".h"       //
                                           << ".hpp"     //
                                           << ".c"       //
                                           << ".cc"      //
                                           << ".cxx"     //
                                           << ".hxx"     //
                                           << ".java";
#if 0
    QFile::Permissions permissions;
    permissions.setFlag(QFile::Permission::ReadOwner, true);
    permissions.setFlag(QFile::Permission::ReadGroup, true);
    permissions.setFlag(QFile::Permission::WriteOwner, true);
    permissions.setFlag(QFile::Permission::WriteGroup, true);
    permissions.setFlag(QFile::Permission::ExeOwner, true);
    permissions.setFlag(QFile::Permission::ExeGroup, true);

    QString resCfgPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir resCfgDir(resCfgPath + QDir::separator() + "Resources");
    if (!resCfgDir.exists()) {
        if (!resCfgDir.mkpath(resCfgDir.absolutePath(), permissions)) {
            MCP_TOOLS_LOG_CRITICAL() << "MCPAutoServer: Unable to create directory:" << resCfgDir.absolutePath();
            return;
        }
    }
#endif

    QList<QFileInfo> resources = findFileResources(basePath, DEFAULT_EXTENSIONS, bRecursive);
    if (resources.isEmpty()) {
        return;
    }

    // propagate MCP resources, 5 sec delay
    QTimer::singleShot(5000, this, [this, resources]() {
        QMimeDatabase mdb;
        foreach (auto res, resources) {
            QJsonObject entry({
                QPair<QString, QString>("uri", "file://" + res.absoluteFilePath()),
                QPair<QString, QString>("name", res.fileName()),
                QPair<QString, QString>("description", tr("Local file resource %1 in directory %2").arg(res.fileName(), res.path())),
                QPair<QString, QString>("mimeType", mdb.mimeTypeForFile(res.fileName()).name()),
                QPair<QString, QString>("type", "file"),
                QPair<QString, QString>("filePath", res.absoluteFilePath()),
            });
#if 0
            QJsonDocument doc(entry);
            QFile file(resCfgDir.absoluteFilePath(res.fileName().replace(".", "_").append(".json")));
            if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
                MCP_CORE_LOG_WARNING() << "MCPAutoServer: Unable to create file:" << file.fileName();
                return;
            }
            file.write(doc.toJson(QJsonDocument::Indented));
            file.flush();
            file.close();
#else
            MCPInvokeHelper::asynInvoke(m_pServer, [this, entry]() { //
                if (!m_pServer->getResourceService()->addFromJson(entry)) {
                    MCP_CORE_LOG_WARNING() << "MCPAutoServer: Failed to add resource: " //
                                           << entry;
                }
            });
#endif
        }
    });
}

void MCPAutoServer::loadMcpToolset()
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
            MCP_TOOLS_LOG_CRITICAL() << "Unable to create tools directory" << toolsCfgPath;
            return;
        }
    }

    // propagate MCP toolset, 5 sec delay
    QTimer::singleShot(5000, this, [this, toolDir]() {
        // get tool configuration files
        QFileInfoList files = toolDir.entryInfoList(QStringList() << "*.json");
        foreach (auto fi, files) {
            if (fi.isFile() && fi.isReadable()) {
                loadTool(fi.absoluteFilePath().toUtf8().constData());
            }
        }
    });
}

#ifdef Q_OS_WINDOWS
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            // Increase module reference count
            //GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, (LPCTSTR)hModule, &hModule);
            break;
    }
    return TRUE;
}
#endif
