#ifndef MCPLOG_H
#define MCPLOG_H

#include <QObject>
#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QSharedPointer>
#include <QMutex>
#include <QDateTime>

// Log level enumeration
enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Critical = 3,
    Fatal = 4
};

// Log category declarations
Q_DECLARE_LOGGING_CATEGORY(mcpCore)
Q_DECLARE_LOGGING_CATEGORY(mcpTransport)
Q_DECLARE_LOGGING_CATEGORY(mcpTools)
Q_DECLARE_LOGGING_CATEGORY(mcpSession)
Q_DECLARE_LOGGING_CATEGORY(mcpResource)

/**
 * @brief MCP Log module class
 * Provides module-level logging functionality, supporting file output and QLoggingCategory
 */
class MCPLog : public QObject
{
    Q_OBJECT

public:
    // Get singleton instance
    static MCPLog* instance();
    ~MCPLog();
public:
    // Initialize the log system
    bool initialize(const QString& strLogFilePath = QString(),
                   LogLevel minLevel = LogLevel::Debug,
                   bool bEnableFileLogging = true);
    // Shutdown the log system
    void shutdown();

    // Set log level
    void setLogLevel(LogLevel level);

    // Set log file path
    bool setLogFile(const QString& strFilePath);

    // Enable/disable file logging
    void setFileLoggingEnabled(bool bEnabled);

private:
    explicit MCPLog(QObject* parent = nullptr);
    // Disable copy constructor and assignment
    MCPLog(const MCPLog&) = delete;
    MCPLog& operator=(const MCPLog&) = delete;

    // Format log message
    QString formatMessage(QtMsgType type, const QMessageLogContext& context, const QString& strMsg);

    // Update log filter rules
    void updateLogFilterRules(LogLevel level);

    // Write log to file
    void writeToFile(const QString& strMessage);

    // Get log level string
    QString logLevelToString(LogLevel level) const;
    QString msgTypeToString(QtMsgType type) const;

    // Message handler
    static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& strMsg);

    // Data members
    LogLevel m_minLogLevel;
    bool m_bFileLoggingEnabled;
    QSharedPointer<QFile> m_pLogFile;
    QSharedPointer<QTextStream> m_pLogStream;
    QMutex m_fileMutex;
};

// Log macro definitions - using QLoggingCategory
#define MCP_LOG_DEBUG(category) qCDebug(category)
#define MCP_LOG_INFO(category) qCInfo(category)
#define MCP_LOG_WARNING(category) qCWarning(category)
#define MCP_LOG_CRITICAL(category) qCCritical(category)

// Convenience macros - general category
#define MCP_CORE_LOG_DEBUG() MCP_LOG_DEBUG(mcpCore)
#define MCP_CORE_LOG_INFO() MCP_LOG_INFO(mcpCore)
#define MCP_CORE_LOG_WARNING() MCP_LOG_WARNING(mcpCore)
#define MCP_CORE_LOG_CRITICAL() MCP_LOG_CRITICAL(mcpCore)

// Transport layer logging
#define MCP_TRANSPORT_LOG_DEBUG() MCP_LOG_DEBUG(mcpTransport)
#define MCP_TRANSPORT_LOG_INFO() MCP_LOG_INFO(mcpTransport)
#define MCP_TRANSPORT_LOG_WARNING() MCP_LOG_WARNING(mcpTransport)
#define MCP_TRANSPORT_LOG_CRITICAL() MCP_LOG_CRITICAL(mcpTransport)

// Tools logging
#define MCP_TOOLS_LOG_DEBUG() MCP_LOG_DEBUG(mcpTools)
#define MCP_TOOLS_LOG_INFO() MCP_LOG_INFO(mcpTools)
#define MCP_TOOLS_LOG_WARNING() MCP_LOG_WARNING(mcpTools)
#define MCP_TOOLS_LOG_CRITICAL() MCP_LOG_CRITICAL(mcpTools)

// Session logging
#define MCP_SESSION_LOG_DEBUG() MCP_LOG_DEBUG(mcpSession)
#define MCP_SESSION_LOG_INFO() MCP_LOG_INFO(mcpSession)
#define MCP_SESSION_LOG_WARNING() MCP_LOG_WARNING(mcpSession)
#define MCP_SESSION_LOG_CRITICAL() MCP_LOG_CRITICAL(mcpSession)

// Resource logging
#define MCP_RESOURCE_LOG_DEBUG() MCP_LOG_DEBUG(mcpResource)
#define MCP_RESOURCE_LOG_INFO() MCP_LOG_INFO(mcpResource)
#define MCP_RESOURCE_LOG_WARNING() MCP_LOG_WARNING(mcpResource)
#define MCP_RESOURCE_LOG_CRITICAL() MCP_LOG_CRITICAL(mcpResource)

#endif // MCPLOG_H