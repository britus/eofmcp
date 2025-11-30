/**
 * @file MyResourceHandler.h
 * @brief 资源Handler示例类（用于验证MCPResourceWrapper）
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
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
     * @brief 获取资源元数据
     * @return 资源元数据对象
     */
    QJsonObject getMetadata() const;

    /**
     * @brief 获取资源内容
     * @return 资源内容字符串
     */
    QString getContent() const;

    /**
     * @brief 更新资源内容（用于测试）
     * @param strNewContent 新的资源内容
     */
    void updateContent(const QString &strNewContent);

    /**
     * @brief 更新资源名称（用于测试）
     * @param strNewName 新的资源名称
     */
    void updateName(const QString &strNewName);

signals:
    /**
     * @brief 资源变化信号
     * @param strName 资源名称
     * @param strDescription 资源描述
     * @param strMimeType MIME类型
     */
    void changed(const QString &strName, const QString &strDescription, const QString &strMimeType);

private slots:
    /**
     * @brief 定时器超时，模拟资源内容变化
     */
    void onTimerTimeout();

private:
    QString m_strName;        // 资源名称
    QString m_strDescription; // 资源描述
    QString m_strMimeType;    // MIME类型
    QString m_strContent;     // 资源内容
    QTimer *m_pTimer;         // 定时器（用于模拟资源变化）
};
