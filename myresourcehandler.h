/**
 * @file MyResourceHandler.h
 * @brief Example class for a resource handler (used to verify MCPResourceWrapper)
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <QFileInfo>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTimer>

/**
 * @brief Example class for a resource handler
 *
 * Responsibilities:
 * - Implements the minimum interface requirements of MCPResource
 * - Provides getMetadata(), getContent() methods and the changed() signal
 * - Used to verify the functionality of MCPResourceWrapper
 *
 * Coding conventions:
 * - Class members use the m_ prefix
 * - String types use the str prefix
 * - { and } must be placed on separate lines
 */
class MyResourceHandler : public QObject
{
    Q_OBJECT

public:
    explicit MyResourceHandler(QObject *pParent = nullptr);
    virtual ~MyResourceHandler();

public slots:
    /**
     * @brief Get resource metadata
     * @return Resource metadata object
     */
    Q_INVOKABLE QJsonObject getMetadata() const;

    /**
     * @brief Get resource content
     * @return Resource content string
     */
    Q_INVOKABLE QString getContent() const;

    /**
     * @brief Get resource annotation
     * @return Resource annotation string
     */
    Q_INVOKABLE QJsonObject getAnnotations() const;

    /**
     * @brief Update resource content (for testing)
     * @param strNewContent New resource content
     */
    Q_INVOKABLE void updateContent(const QString &strNewContent);

    /**
     * @brief Update resource name (for testing)
     * @param strNewName New resource name
     */
    Q_INVOKABLE void updateName(const QString &strNewName);

signals:
    /**
     * @brief Resource change signal
     * @param strName Resource name
     * @param strDescription Resource description
     * @param strMimeType MIME type
     */
    void changed(const QString &strName, const QString &strDescription, const QString &strMimeType);

private:
    QFileInfo m_fileInfo;
    QString m_strName;        // Resource name
    QString m_strDescription; // Resource description
    QString m_strMimeType;    // MIME type
    QString m_strContent;     // Resource content
};
