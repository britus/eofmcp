/**
 * @file MCPCore_global.h.h
 * @brief MCPCore
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <QtCore/qglobal.h>

#ifdef LIBMCPServer
#define MCPCORE_EXPORT Q_DECL_EXPORT
#else
#define MCPCORE_EXPORT Q_DECL_IMPORT
#endif
