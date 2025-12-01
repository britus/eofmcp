/**
 * @file MCPFileResource.cpp
 * @brief MCP file resource class implementation
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPFileResource.h"
#include "MCPResourceContentGenerator.h"
#include "MCPLog.h"
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

MCPFileResource::MCPFileResource(const QString& strUri,
                                 const QString& strFilePath,
                                 const QString& strName,
                                 QObject* pParent)
    : MCPContentResource(strUri, pParent)
    , m_strFilePath(strFilePath)
{
    // If name is provided, set it; otherwise use filename as name
    if (!strName.isEmpty())
    {
        withName(strName);
    }
    else
    {
        QFileInfo fileInfo(strFilePath);
        withName(fileInfo.fileName());
    }

    // Infer MIME type
    inferMimeType();

    // Set file content provider function
    withContentProvider(createFileContentProvider());
}

MCPFileResource::~MCPFileResource()
{

}


QString MCPFileResource::getFilePath() const
{

    return m_strFilePath;
}

std::function<QString()> MCPFileResource::createFileContentProvider() const
{

    return [this]() -> QString
    {
        if (m_strFilePath.isEmpty())
        {
            MCP_CORE_LOG_WARNING() << "MCPFileResource: File path is empty, cannot read content";
            return QString();
        }

        // Determine reading method based on MIME type
        if (MCPResourceContentGenerator::isTextMimeType(getMimeType()))
        {
            // Text type, read text content directly
            return MCPResourceContentGenerator::readFileAsText(m_strFilePath);
        }
        else
        {
            // Binary type, return Base64 encoded content
            return MCPResourceContentGenerator::readFileAsBase64(m_strFilePath);
        }
    };
}

void MCPFileResource::inferMimeType()
{

    if (m_strFilePath.isEmpty())
    {
        withMimeType("text/plain");
        return;
    }

    // If MIME type is already set, don't re-infer
    if (!getMimeType().isEmpty())
    {
        return;
    }

    // Use QMimeDatabase to infer MIME type based on file extension
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(m_strFilePath);
    if (mimeType.isValid())
    {
        withMimeType(mimeType.name());
    }
    else
    {
        // If cannot infer, default to text/plain
        withMimeType("text/plain");
        MCP_CORE_LOG_WARNING() << "MCPFileResource: Cannot infer file MIME type, using default value text/plain:" << m_strFilePath;
    }
}