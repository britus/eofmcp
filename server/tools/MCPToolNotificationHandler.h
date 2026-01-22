/**
 * @file MCPToolNotificationHandler.h
 * @brief MCP tool notification handler
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */


#pragma once
#include "MCPNotificationHandlerBase.h"

/**
 * @brief MCP tool notification handler
 *
 * Responsibilities:
 * - Handle tool list change notifications (broadcast notifications)
 *
 * Coding conventions:
 * - Class members add m_ prefix
 * - Pointer types add p prefix
 * - { and } should be on separate lines
 */
class MCPToolNotificationHandler : public MCPNotificationHandlerBase
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param pServer Server object (required)
     * @param pParent Parent object
     */
    explicit MCPToolNotificationHandler(MCPServer *pServer, QObject *pParent = nullptr);

    virtual ~MCPToolNotificationHandler();

public slots:
    /**
     * @brief Handle tool list change event (broadcast notification)
     */
    void onToolsListChanged();
};