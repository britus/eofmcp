#pragma once
#include <IMCPServer.h>
#include <MCPPromptService.h>
#include <MCPResourceService.h>
#include <MCPToolService.h>
#include <QJsonObject>
#include <QObject>
#include <QSharedPointer>
#include <QString>
class MCPAbstractServerTransport;
class MCPMessage;
class MCPSessionService;
class IMCPTransport;
class MCPThreadPool;
class MCPServerMessage;
class MCPServerConfig;
class MCPServerHandler;
class MCPToolsConfig;
class MCPResourcesConfig;
class MCPPromptsConfig;
/**
 * @brief MCP Server Implementation Class
 *
 * Responsibilities:
 * - Server lifecycle management
 * - Component initialization coordination
 * - Extension registration interface
 * - Signal-slot connection management
 * - Running in an independent thread
 *
 * Coding standards:
 * - Add m_ prefix to class members
 * - Add p prefix to pointer types
 * - { and } should be on separate lines
 */
class MCPServer : public IMCPServer
{
    Q_OBJECT

public:
    explicit MCPServer(QObject *pParent = nullptr);

protected:
    /**
     * @brief Destructor (protected, can only be destroyed by destroyServer)
     */
    virtual ~MCPServer();

public:
    /**
     * @brief Start the server
     * @return Whether startup was successful
     */
    bool start() override;

    /**
     * @brief Stop the server
     */
    void stop() override;

    /**
     * @brief Check if the server is running
     * @return true indicates it's running, false otherwise
     */
    bool isRunning() override;

public:
    /**
     * @brief Get the configuration object
     * @return Pointer to the configuration object
     */
    IMCPServerConfig *getConfig() override;

    /**
     * @brief Get the tool service (returns a concrete implementation pointer for internal use)
     * @return Pointer to the tool service implementation
     */
    MCPToolService *getToolService() override;

    /**
     * @brief Get the resource service (returns a concrete implementation pointer for internal use)
     * @return Pointer to the resource service implementation
     */
    MCPResourceService *getResourceService() override;

    /**
     * @brief Get the prompt service (returns a concrete implementation pointer for internal use)
     * @return Pointer to the prompt service implementation
     */
    MCPPromptService *getPromptService() override;

public:
    IMCPTransport *getTransport() const;
    MCPSessionService *getSessionService() const;

private slots:
    void onThreadReady();
    void onConfigLoaded(QSharedPointer<MCPToolsConfig> pToolsConfig, QSharedPointer<MCPResourcesConfig> pResourcesConfig, QSharedPointer<MCPPromptsConfig> pPromptsConfig);

private:
    bool initServer(QSharedPointer<MCPToolsConfig> pToolsConfig, QSharedPointer<MCPResourcesConfig> pResourcesConfig, QSharedPointer<MCPPromptsConfig> pPromptsConfig);
    bool doStart();
    bool doStop();

private:
    IMCPTransport *m_pTransport;
    MCPSessionService *m_pSessionService;
    MCPToolService *m_pToolService;
    MCPResourceService *m_pResourceService;
    MCPPromptService *m_pPromptService;
    MCPServerConfig *m_pConfig;
    MCPServerHandler *m_pHandler;

private:
    QThread *m_pThread;
};
