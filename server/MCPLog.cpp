#include "MCPLog.h"
#include <QDir>
#include <QStandardPaths>
#include <QThread>

// 定义日志类别
Q_LOGGING_CATEGORY(mcpCore, "mcp.core")
Q_LOGGING_CATEGORY(mcpTransport, "mcp.transport")
Q_LOGGING_CATEGORY(mcpTools, "mcp.tools")
Q_LOGGING_CATEGORY(mcpSession, "mcp.session")
Q_LOGGING_CATEGORY(mcpResource, "mcp.resource")

// 静态成员变量初始化

MCPLog::MCPLog(QObject *parent)
    : QObject(parent)
    , m_minLogLevel(LogLevel::Debug)
    , m_bFileLoggingEnabled(false)
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt*=false"));
    QLoggingCategory::setFilterRules(QStringLiteral("qt.gui*=false"));
    QLoggingCategory::setFilterRules(QStringLiteral("qt.canbus.*=true"));
    QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus.*=true"));
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth.*=true"));
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth.bluez.*=true"));
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth.osx.*=true"));
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth.windows.*=true"));
}

MCPLog::~MCPLog()
{
    shutdown();
}

MCPLog *MCPLog::instance()
{
    static MCPLog instance;
    return &instance;
}

bool MCPLog::initialize(const QString &strLogFilePath, LogLevel minLevel, bool bEnableFileLogging)
{
    m_minLogLevel = minLevel;
    m_bFileLoggingEnabled = bEnableFileLogging;

    // 如果没有指定日志文件路径，使用当前目录
    QString cdir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

    QFile::Permissions permissions;
    permissions.setFlag(QFile::Permission::ReadOwner, true);
    permissions.setFlag(QFile::Permission::ReadGroup, true);
    permissions.setFlag(QFile::Permission::WriteOwner, true);
    permissions.setFlag(QFile::Permission::WriteGroup, true);
    permissions.setFlag(QFile::Permission::ExeOwner, true);
    permissions.setFlag(QFile::Permission::ExeGroup, true);

    QDir dir(cdir);
    if (!dir.exists()) {
        dir.mkdir(cdir, permissions);
    }

    QString strActualLogPath = strLogFilePath.isEmpty() //
                                   ? QDir(cdir).absoluteFilePath("eofmcp.log")
                                   : strLogFilePath;

    // 设置日志文件（内部会创建目录）
    if (!setLogFile(strActualLogPath)) {
        return false;
    }

    // 安装消息处理器，用于文件输出
    qInstallMessageHandler(messageHandler);

    updateLogFilterRules(minLevel);

    return true;
}

void MCPLog::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &strMsg)
{
    // 处理我们模块的日志
    if (context.category
        && (strcmp(context.category, "mcp.core") == 0          //
            || strcmp(context.category, "mcp.transport") == 0  //
            || strcmp(context.category, "mcp.tools") == 0      //
            || strcmp(context.category, "mcp.session") == 0    //
            || strcmp(context.category, "mcp.resource") == 0)) //
    {
        instance()->writeToFile(instance()->formatMessage(type, context, strMsg));

        if (type == QtDebugMsg) {
            fprintf(stdout, "[D] %s\n", qPrintable(strMsg));
        } else if (type == QtInfoMsg) {
            fprintf(stdout, "[I] %s\n", qPrintable(strMsg));
        } else {
            fprintf(stderr, "[E] %s\n", qPrintable(strMsg));
        }
        return;
    }

    // others
    fprintf(stdout, "[Q] %s\n", qPrintable(strMsg));
}

void MCPLog::shutdown()
{
    // 移除消息处理器
    qInstallMessageHandler(nullptr);

    if (m_pLogStream) {
        m_pLogStream->flush();
    }

    if (m_pLogFile) {
        m_pLogFile->close();
    }

    // QSharedPointer会自动清理内存
    m_pLogStream.reset();
    m_pLogFile.reset();

    m_bFileLoggingEnabled = false;
}

void MCPLog::setLogLevel(LogLevel level)
{
    m_minLogLevel = level;
    updateLogFilterRules(level);
}

void MCPLog::updateLogFilterRules(LogLevel level)
{
    switch (level) {
        case LogLevel::Debug:
            QLoggingCategory::setFilterRules("*.debug=true\n*.info=true\n*.warning=true\n*.critical=true");
            break;
        case LogLevel::Info:
            QLoggingCategory::setFilterRules("*.debug=false\n*.info=true\n*.warning=true\n*.critical=true");
            break;
        case LogLevel::Warning:
            QLoggingCategory::setFilterRules("*.debug=false\n*.info=false\n*.warning=true\n*.critical=true");
            break;
        case LogLevel::Critical:
        case LogLevel::Fatal:
            QLoggingCategory::setFilterRules("*.debug=false\n*.info=false\n*.warning=false\n*.critical=true");
            break;
    }
}

bool MCPLog::setLogFile(const QString &strFilePath)
{
    // 关闭现有文件
    if (m_pLogStream) {
        m_pLogStream->flush();
    }
    if (m_pLogFile) {
        m_pLogFile->close();
    }

    // 确保目录存在
    QFileInfo fileInfo(strFilePath);
    QDir logDir = fileInfo.absoluteDir();
    if (!logDir.exists()) {
        if (!logDir.mkpath(".")) {
            return false;
        }
    }

    // 创建新文件
    m_pLogFile = QSharedPointer<QFile>(new QFile(strFilePath));
    if (!m_pLogFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_pLogFile.reset();
        return false;
    }

    m_pLogStream = QSharedPointer<QTextStream>(new QTextStream(m_pLogFile.data()));
    m_pLogStream->setEncoding(QStringConverter::Utf8);
    return true;
}

void MCPLog::setFileLoggingEnabled(bool bEnabled)
{
    m_bFileLoggingEnabled = bEnabled;
}

QString MCPLog::formatMessage(QtMsgType type, const QMessageLogContext &context, const QString &strMsg)
{
    QString strFormattedMsg = QString("[%1] [%2] [Thread:0x%3] %4")
                                  .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                                  .arg(msgTypeToString(type))
                                  .arg((quintptr) QThread::currentThreadId(), 0, 16)
                                  .arg(strMsg);

    if (context.file && context.line > 0) {
        strFormattedMsg += QString(" (%1:%2)").arg(context.file).arg(context.line);
    }

    if (context.function) {
        strFormattedMsg += QString(" [%1]").arg(context.function);
    }

    return strFormattedMsg;
}

void MCPLog::writeToFile(const QString &strMessage)
{
    QMutexLocker locker(&m_fileMutex);
    if (m_pLogStream) {
        *m_pLogStream << strMessage << Qt::endl;
        m_pLogStream->flush();
    }
}

QString MCPLog::logLevelToString(LogLevel level) const
{
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Critical:
            return "CRITICAL";
        case LogLevel::Fatal:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}

QString MCPLog::msgTypeToString(QtMsgType type) const
{
    switch (type) {
        case QtDebugMsg:
            return "DEBUG   ";
        case QtInfoMsg:
            return "INFO    ";
        case QtWarningMsg:
            return "WARNING ";
        case QtCriticalMsg:
            return "CRITICAL";
        case QtFatalMsg:
            return "FATAL   ";
        default:
            return "UNKNOWN ";
    }
}
