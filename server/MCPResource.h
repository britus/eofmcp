/**
 * @file MCPResource.h
 * @brief MCP resource base class (using Qt Meta mechanism)
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

/**
 * @brief MCP resource base class
 * 
 * Responsibilities:
 * - Define metadata of resources (URI, name, description, MIME type, etc.)
 * - Use Qt Meta mechanism (Q_PROPERTY) to provide property system
 * - Provide signal notification for resource changes
 * - Define read interface for resource content (pure virtual function)
 * 
 * Important notes:
 * - URI is the unique identifier of the resource, cannot be modified after creation
 * - Resource identity is determined by URI, metadata changes don't affect resource identity
 * - Metadata (name, description, mimeType) can be modified, modification emits changed signal
 * 
 * Coding standards:
 * - Class members add m_ prefix
 * - String types add str prefix
 * - { and } should be on separate lines
 */
class MCPResource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString uri READ getUri CONSTANT)  // URI is immutable, use CONSTANT
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(QString description READ getDescription WRITE setDescription)
    Q_PROPERTY(QString mimeType READ getMimeType WRITE setMimeType)
    
public:
    explicit MCPResource(const QString& strUri, 
                         QObject* pParent = nullptr);
    virtual ~MCPResource();
signals:
    /**
     * @brief Resource changed signal
     * Emitted when resource metadata or content changes
     * @param strName Resource name
     * @param strDescription Resource description
     * @param strMimeType MIME type
     * 
     * Note: Changed signal is emitted when metadata (name, description, mimeType) or content changes
     */
    void changed(const QString& strName, const QString& strDescription, const QString& strMimeType);
    
    /**
     * @brief Resource invalidated signal
     * Emitted when resource is invalidated (e.g., file deleted, resource unavailable)
     * 
     * Note: After resource invalidation, the resource object may still exist but cannot be used normally
     * Subscribers should cancel subscriptions or handle invalidation cases
     */
    void invalidated();
public:
    // ============ Property accessors (Q_PROPERTY) ============
    
    /**
     * @brief Get resource URI
     * @return Resource URI
     * 
     * Note: URI is the unique identifier of the resource, cannot be modified after creation
     */
    QString getUri() const;
    
    /**
     * @brief Get resource name
     * @return Resource name
     */
    QString getName() const;
    
    /**
     * @brief Set resource name
     * @param strName Resource name
     * 
     * Note: Modifying name doesn't change resource identity (URI unchanged), but emits changed signal
     */
    void setName(const QString& strName);
    
    /**
     * @brief Get resource description
     * @return Resource description
     */
    QString getDescription() const;
    
    /**
     * @brief Set resource description
     * @param strDescription Resource description
     * 
     * Note: Modifying description doesn't change resource identity (URI unchanged), but emits changed signal
     */
    void setDescription(const QString& strDescription);
    
    /**
     * @brief Get MIME type
     * @return MIME type
     */
    QString getMimeType() const;
    
    /**
     * @brief Set MIME type
     * @param strMimeType MIME type
     * 
     * Note: Modifying MIME type doesn't change resource identity (URI unchanged), but emits changed signal
     */
    void setMimeType(const QString& strMimeType);
    
    /**
     * @brief Set resource annotations (Annotations)
     * @param annotations Annotation object
     * 
     * According to MCP protocol specification, annotations include:
     * - audience: Array, valid values are "user" and "assistant"
     * - priority: Number between 0.0 and 1.0, indicating importance
     * - lastModified: ISO 8601 formatted timestamp
     */
    void setAnnotations(const QJsonObject& annotations);
    
    /**
     * @brief Get audience (audience)
     * @return Audience array, containing "user" and/or "assistant"
     */
    QJsonArray getAudience() const;
    
    /**
     * @brief Set audience (audience)
     * @param audience Audience array, valid values are "user" and "assistant"
     */
    void setAudience(const QJsonArray& audience);
    
    /**
     * @brief Get priority (priority)
     * @return Priority value, range 0.0 to 1.0
     */
    double getPriority() const;
    
    /**
     * @brief Set priority (priority)
     * @param priority Priority value, range 0.0 to 1.0 (1.0 means most important, 0.0 means least important)
     */
    void setPriority(double priority);
    
    /**
     * @brief Get last modified time (lastModified)
     * @return ISO 8601 formatted timestamp string
     */
    QString getLastModified() const;
    
    /**
     * @brief Set last modified time (lastModified)
     * @param lastModified ISO 8601 formatted timestamp string (e.g., "2025-01-12T15:00:58Z")
     */
    void setLastModified(const QString& lastModified);
    
    /**
     * @brief Update last modified time to current time
     */
    void updateLastModified();
    
    /**
     * @brief Notify resource changed
     * Manually trigger resource change signal, used to notify subscribers when content changes
     */
    void notifyChanged();
    
    /**
     * @brief Notify resource invalidated
     * Manually trigger resource invalidation signal, used to notify subscribers when resource is unavailable
     * 
     * Use cases:
     * - Resource file deleted
     * - Resource provider unavailable
     * - Resource permissions revoked
     * - Other cases where resource cannot be used
     */
    void notifyInvalidated();
    
public slots:
    // ============ Resource metadata and content interfaces (Qt Meta mechanism) ============
    
    /**
     * @brief Get resource metadata (JSON format)
     * @return Resource metadata object
     * 
     * As slot, can be called via QMetaObject::invokeMethod
     * Subclasses can override this method to provide custom metadata retrieval logic
     */
    virtual QJsonObject getMetadata() const;
    
    /**
     * @brief Get resource annotations
     * @return Resource annotation object, including audience, priority, lastModified fields
     * 
     * According to MCP protocol specification, annotations are optional and provide hints for how clients should use or display resources
     * As slot, can be called via QMetaObject::invokeMethod
     */
    virtual QJsonObject getAnnotations() const;
    
    /**
     * @brief Get resource content
     * @return Resource content string
     * 
     * This is the public interface for getting resource content, internally calls readContent()
     * For text resources, returns text content; for binary resources, returns Base64 encoded content
     * 
     * As slot, can be called via QMetaObject::invokeMethod
     */
    QString getContent() const;
    
    
public:
    // ============ Resource content reading interface (pure virtual function, implemented by subclass) ============
    
    /**
     * @brief Read resource content (pure virtual function, implemented by subclass)
     * @return Resource content string
     * 
     * Note: Subclasses must implement this method, providing specific resource content reading logic
     * This method should be const, because reading content shouldn't modify object state
     * 
     * Note: This method is not a slot, because it's a pure virtual function and needs to be called via getContent() slot
     */
    virtual QString readContent() const = 0;
protected:
    QString m_strUri;
    QString m_strName;
    QString m_strDescription;
    QString m_strMimeType;
    
    // Resource annotations (Annotations), according to MCP protocol specification
    QJsonArray m_audience;        // Audience array, valid values are "user" and "assistant"
    double m_priority;             // Priority, range 0.0 to 1.0
    QString m_strLastModified;     // Last modified time, ISO 8601 format
};