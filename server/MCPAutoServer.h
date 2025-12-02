#pragma once
#include <QObject>
#include <QThread>

class IMCPServer;
class IMCPServerConfig;
class MCPTool;

/**
 * @brief MCP Auto Server Class - Automatically starts and manages an MCP server based on configuration files
 *
 * This class provides a zero-configuration way to start an MCP server:
 * - Automatically detects the MCPServerConfig.json configuration file in the program directory
 * - Parses server port and tool definitions from the configuration file
 * - Automatically finds and binds tool corresponding handler objects
 * - Starts the server in a separate thread, providing high concurrency processing capabilities
 * - The server runs continuously after startup without manual stop
 *
 * Usage:
 * @code
 * // Create a Handler object in main function
 * MyHandler* handler = new MyHandler(qApp);
 * handler->setObjectName("MyHandler");
 *
 * // Create auto server (auto start)
 * MCPAutoServer server;
 * @endcode
 */
class MCPAutoServer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor - Automatically detects and starts the MCP server
     *
     * The constructor will automatically perform the following operations:
     * 1. Check if MCPServerConfig.json configuration file exists
     * 2. Parse server configurations and tool definitions from the configuration file
     * 3. Find and bind tool corresponding handler objects
     * 4. Start the server in a separate thread
     *
     * If any step fails, warning messages will be output but no exceptions will be thrown.
     *
     * @param pParent Parent object pointer
     */
    explicit MCPAutoServer(QObject *pParent = nullptr);

    /**
     * @brief Destructor - Clean up server resources
     *
     * Stop the server thread and clean up related resources.
     */
    virtual ~MCPAutoServer();

public:
    /**
     * @brief Perform auto start logic
     * @deprecated Please create MCPAutoServer object directly to auto start
     */
    void performStart();

    /**
     * @brief perform stop logic
     * @deprecated The server will run continuously until the program ends
     */
    void performStop();

    /**
     * @brief Load tool configuration file
     * @param strToolConfigFile Tool configuration file path
     */
    void loadTool(const QString &strToolConfigFile);

    /**
     * @brief Generate resources configuration for files in given directory
     * @param basePath The root directory of file resources
     * @param bRecursive True to walk into sub directories or false
     */
    void generateResources(const QString basePath, bool bRecursive = true);

    /**
     * @brief Load all MCP toolset configurations
     */
    void loadMcpToolset();

    /**
     * @brief Return a pointer to managed server instance
     * @return Pointer of type IMCPServer*
     */
    inline IMCPServer *server() { return m_pServer; }

private:
    IMCPServer *m_pServer; // Server instance
};
