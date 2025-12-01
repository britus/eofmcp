/**
 * @file MCPServer.h
 * @brief MCP server base class
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <IMCPPromptService.h>
#include <IMCPResourceService.h>
#include <IMCPServerConfig.h>
#include <IMCPToolService.h>
#include <MCPCore_global.h.h>
#include <QObject>
#include <QString>

//class IMCPServerConfig;
//class IMCPToolService;
//class IMCPResourceService;
//class IMCPPromptService;

/**
 * @brief MCP server base class
 *
 * Responsibilities:
 * - Define public interface of MCP server
 * - Provide unified registration API
 * - Manage server configuration
 *
 * Coding standards:
 * - Class members add m_ prefix
 * - { and } should be on separate lines
 */
class MCPCORE_EXPORT IMCPServer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor (protected, can only be created through createServer)
     */
    explicit IMCPServer(QObject *pParent = nullptr);

    /**
     * @brief Create server instance
     * @return Server instance pointer, returns nullptr if failed
     */
    static IMCPServer *createServer();

    /**
     * @brief Destroy server instance (recommended to use this method)
     * @param pServer Server instance pointer
     *
     * Note: Don't directly use delete to destroy server instances, should use this method
     */
    static void destroyServer(IMCPServer *pServer);

public:
    /**
     * @brief Start server
     * @param nPort Listen port number, default 8888
     * @param pConfig Configuration object (optional), if provided, apply configuration from tools
     * @return true if start successful, false if start failed
     *
     * Usage example:
     * @code
     * // Method 1: Start directly (default port)
     * pServer->start();
     * @endcode
     */
    virtual bool start() = 0;

    /**
     * @brief Stop server
     */
    virtual void stop() = 0;

    /**
     * @brief Check if running
     * @return true if running, false if not running
     */
    virtual bool isRunning() = 0;

public:
    /**
     * @brief Get configuration object
     * @return Configuration object pointer (guaranteed to be non-null)
     *
     * Usage example:
     * @code
     * auto pConfig = pServer->getConfig();
     * pConfig->setServerName("My Server");
     * @endcode
     */
    virtual IMCPServerConfig *getConfig() = 0;

    /**
     * @brief Get tool service interface
     * @return Tool service interface pointer (guaranteed to be non-null)
     *
     * Note: Service objects are managed by Server, don't delete directly
     *
     * Usage example:
     * @code
     * auto pToolService = pServer->getToolService();
     * pToolService->add("myTool", "My Tool", "Tool description",
     *     inputSchema, outputSchema,
     *     []() -> QJsonObject {
     *         QJsonObject result;
     *         result["content"] = QJsonArray();
     *         result["structuredContent"] = QJsonObject();
     *         return result;
     *     });
     * @endcode
     */
    virtual IMCPToolService *getToolService() = 0;

    /**
     * @brief Get resource service interface
     * @return Resource service interface pointer (guaranteed to be non-null)
     *
     * Note: Service objects are managed by Server, don't delete directly
     *
     * Usage example:
     * @code
     * auto pResourceService = pServer->getResourceService();
     * pResourceService->add("file:///path/to/file.txt", "My File", "A text file", "/path/to/file.txt");
     * @endcode
     */
    virtual IMCPResourceService *getResourceService() = 0;

    /**
     * @brief Get prompt service interface
     * @return Prompt service interface pointer (guaranteed to be non-null)
     *
     * Note: Service objects are managed by Server, don't delete directly
     *
     * Usage example:
     * @code
     * auto pPromptService = pServer->getPromptService();
     * QList<QPair<QString, QPair<QString, bool>>> args;
     * args.append(qMakePair("name", qMakePair("Username", true)));
     * pPromptService->add("greeting", "Greeting prompt", args, "Hello {{name}}, welcome!");
     * @endcode
     */
    virtual IMCPPromptService *getPromptService() = 0;

protected:
    /**
     * @brief Destructor (protected, can only be destroyed through destroyServer)
     */
    virtual ~IMCPServer() override;
};

//#define IMCPServer_iid "org.eof.IMCPServer"
//Q_DECLARE_INTERFACE(IMCPServer, IMCPServer_iid)
