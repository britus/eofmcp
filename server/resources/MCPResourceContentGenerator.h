/**
 * @file MCPResourceContentGenerator.h
 * @brief MCP resource content generator (internal implementation)
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

/**
 * @brief MCP resource content generator
 *
 * Responsibilities:
 * - Generate MCP protocol compliant resource content based on MIME type and file path
 * - Support handling of text and binary resources
 * - Automatically detect content type and choose appropriate encoding method
 *
 * Coding standards:
 * - Class members add m_ prefix
 * - String types add str prefix
 * - { and } should be on separate lines
 */
class MCPResourceContentGenerator
{
public:
    /**
     * @brief Generate MCP protocol compliant resource content based on MIME type and file path
     * @param strMimeType MIME type (e.g., "text/plain", "image/png", etc.)
     * @param strFilePath File path
     * @param strUri Resource URI (optional, if not provided it will be generated from file path)
     * @return QJsonObject containing resource content compliant with MCP protocol, including uri and contents array
     *         If file read fails, returns empty QJsonObject
     */
    static QJsonObject generateResourceContent(const QString &strMimeType, const QString &strFilePath, const QString &strUri = QString());

    /**
     * @brief Check if MIME type is a text type
     * @param strMimeType MIME type
     * @return true if it's a text type, false otherwise
     */
    static bool isTextMimeType(const QString &strMimeType);

    /**
     * @brief Read file content as text
     * @param strFilePath File path
     * @return File text content, returns empty string if read fails
     */
    static QString readFileAsText(const QString &strFilePath);

    /**
     * @brief Read file content and encode as Base64
     * @param strFilePath File path
     * @return Base64 encoded string, returns empty string if read fails
     */
    static QString readFileAsBase64(const QString &strFilePath);

    /**
     * @brief Encode QByteArray as Base64
     * @param data QByteArray
     * @return Base64 encoded string
     */
    static QString base64Encode(const QByteArray &data);

    /**
     * @brief Generate resource URI from file path
     * @param strFilePath File path
     * @return Resource URI (file:// protocol)
     */
    static QString generateUriFromFilePath(const QString &strFilePath);

private:
    // Prevent instantiation, all methods are static
    MCPResourceContentGenerator() = delete;
    ~MCPResourceContentGenerator() = delete;
};
