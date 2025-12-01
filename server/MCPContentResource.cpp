/**
 * @file MCPContentResource.cpp
 * @brief MCP content resource class implementation
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPContentResource.h"
#include "MCPLog.h"

MCPContentResource::MCPContentResource(const QString &strUri, QObject *pParent)
    : MCPResource(strUri, pParent)
{}

MCPContentResource::~MCPContentResource() {}

MCPContentResource *MCPContentResource::withName(const QString &strName)
{

    setName(strName);
    return this;
}

MCPContentResource *MCPContentResource::withDescription(const QString &strDescription)
{

    setDescription(strDescription);
    return this;
}

MCPContentResource *MCPContentResource::withMimeType(const QString &strMimeType)
{

    setMimeType(strMimeType);
    return this;
}

MCPContentResource *MCPContentResource::withContentProvider(std::function<QString()> contentProvider)
{

    m_contentProvider = contentProvider;
    return this;
}

QString MCPContentResource::readContent() const
{

    // Use contentProvider to get content
    if (m_contentProvider) {
        return m_contentProvider();
    }

    // If no contentProvider, return empty string
    MCP_CORE_LOG_WARNING() << "MCPContentResource: No content provider set, cannot read content";
    return QString();
}