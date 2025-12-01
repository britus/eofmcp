/**
 * @file MCPResource.cpp
 * @brief MCP resource base class implementation
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPResource.h"
#include <QMetaObject>
#include <QMetaProperty>
#include <QVariant>
#include <QDateTime>

MCPResource::MCPResource(const QString& strUri, 
                         QObject* pParent)
    : QObject(pParent)
    , m_strUri(strUri)
    , m_strName("")
    , m_strDescription("")
    , m_strMimeType("text/plain")
    , m_audience(QJsonArray())
    , m_priority(0.5)  // Default priority is 0.5
    , m_strLastModified("")
{
}


MCPResource::~MCPResource()
{
}


QString MCPResource::getUri() const
{

    return m_strUri;
}

QString MCPResource::getName() const
{

    return m_strName;
}

void MCPResource::setName(const QString& strName)
{

    if (m_strName != strName)
    {
        m_strName = strName;
        emit changed(m_strName, m_strDescription, m_strMimeType);
    }
}

QString MCPResource::getDescription() const
{

    return m_strDescription;
}

void MCPResource::setDescription(const QString& strDescription)
{

    if (m_strDescription != strDescription)
    {
        m_strDescription = strDescription;
        emit changed(m_strName, m_strDescription, m_strMimeType);
    }
}

QString MCPResource::getMimeType() const
{

    return m_strMimeType;
}

void MCPResource::setMimeType(const QString& strMimeType)
{

    if (m_strMimeType != strMimeType)
    {
        m_strMimeType = strMimeType;
        emit changed(m_strName, m_strDescription, m_strMimeType);
    }
}

void MCPResource::notifyChanged()
{

    emit changed(m_strName, m_strDescription, m_strMimeType);
}

void MCPResource::notifyInvalidated()
{

    emit invalidated();
}

QJsonObject MCPResource::getMetadata() const
{

    QJsonObject metadata;
    // Note: URI is not in metadata, because URI is the unique identifier of the resource, obtained via getUri()
    metadata["name"] = m_strName;
    
    if (!m_strDescription.isEmpty())
    {
        metadata["description"] = m_strDescription;
    }
    
    if (!m_strMimeType.isEmpty())
    {
        metadata["mimeType"] = m_strMimeType;
    }
    
    // Add annotations (if any)
    QJsonObject annotations;
    bool bHasAnnotations = false;
    
    if (!m_audience.isEmpty())
    {
        annotations["audience"] = m_audience;
        bHasAnnotations = true;
    }
    
    if (m_priority >= 0.0 && m_priority <= 1.0)
    {
        annotations["priority"] = m_priority;
        bHasAnnotations = true;
    }
    
    if (!m_strLastModified.isEmpty())
    {
        annotations["lastModified"] = m_strLastModified;
        bHasAnnotations = true;
    }
    
    if (bHasAnnotations)
    {
        metadata["annotations"] = annotations;
    }
    
    return metadata;
}

QString MCPResource::getContent() const
{

    // Call readContent() method implemented by subclass
    return readContent();
}

QJsonObject MCPResource::getAnnotations() const
{

    QJsonObject annotations;
    
    if (!m_audience.isEmpty())
    {
        annotations["audience"] = m_audience;
    }
    
    if (m_priority >= 0.0 && m_priority <= 1.0)
    {
        annotations["priority"] = m_priority;
    }
    
    if (!m_strLastModified.isEmpty())
    {
        annotations["lastModified"] = m_strLastModified;
    }
    
    return annotations;
}

void MCPResource::setAnnotations(const QJsonObject& annotations)
{

    if (annotations.contains("audience"))
    {
        QJsonValue audienceValue = annotations["audience"];
        if (audienceValue.isArray())
        {
            m_audience = audienceValue.toArray();
        }
    }
    
    if (annotations.contains("priority"))
    {
        QJsonValue priorityValue = annotations["priority"];
        if (priorityValue.isDouble())
        {
            double priority = priorityValue.toDouble();
            // Limit priority range between 0.0 and 1.0
            if (priority < 0.0)
            {
                m_priority = 0.0;
            }
            else if (priority > 1.0)
            {
                m_priority = 1.0;
            }
            else
            {
                m_priority = priority;
            }
        }
    }
    
    if (annotations.contains("lastModified"))
    {
        m_strLastModified = annotations["lastModified"].toString();
    }
}

QJsonArray MCPResource::getAudience() const
{

    return m_audience;
}

void MCPResource::setAudience(const QJsonArray& audience)
{

    m_audience = audience;
}

double MCPResource::getPriority() const
{

    return m_priority;
}

void MCPResource::setPriority(double priority)
{

    // Limit priority range between 0.0 and 1.0
    if (priority < 0.0)
    {
        m_priority = 0.0;
    }
    else if (priority > 1.0)
    {
        m_priority = 1.0;
    }
    else
    {
        m_priority = priority;
    }
}

QString MCPResource::getLastModified() const
{

    return m_strLastModified;
}

void MCPResource::setLastModified(const QString& lastModified)
{

    m_strLastModified = lastModified;
}

void MCPResource::updateLastModified()
{

    // Use ISO 8601 format: YYYY-MM-DDTHH:mm:ssZ
    QDateTime now = QDateTime::currentDateTimeUtc();
    m_strLastModified = now.toString(Qt::ISODate);
}