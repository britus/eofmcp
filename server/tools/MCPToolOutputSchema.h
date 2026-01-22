/**
 * @file MCPToolOutputSchema.h
 * @brief MCP tool output schema helper class
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <MCPServer_global.h>
#include <QJsonObject>
#include <QObject>
#include <QString>

/**
 * @brief MCPToolOutputSchema helper class
 *
 * Provides type-safe output schema building API
 *
 * Usage example:
 * @code
 * MCPToolOutputSchema schema;
 * schema.addStringField("message", "Message")
 *       .addNumberField("code", "Status code")
 *       .addBooleanField("success", "Whether successful");
 * QJsonObject jsonSchema = schema.toJsonObject();
 * @endcode
 *
 * Coding conventions:
 * - Class members add m_ prefix
 * - String types add str prefix
 * - { and } should be on separate lines
 */
class MCPCORE_EXPORT MCPToolOutputSchema
{
public:
    MCPToolOutputSchema();

    MCPToolOutputSchema *withDescription(const QString &strDescription);

    // Basic type fields
    MCPToolOutputSchema *addStringField(const QString &strFieldName, const QString &strDescription);
    MCPToolOutputSchema *addStringFieldWithLength(const QString &strFieldName, const QString &strDescription, int nMinLength, int nMaxLength);
    MCPToolOutputSchema *addStringFieldWithFormat(const QString &strFieldName, const QString &strDescription, const QString &strFormat);
    MCPToolOutputSchema *addStringFieldWithDefault(const QString &strFieldName, const QString &strDescription, const QString &strDefaultValue);

    MCPToolOutputSchema *addNumberField(const QString &strFieldName, const QString &strDescription);
    MCPToolOutputSchema *addNumberFieldWithRange(const QString &strFieldName, const QString &strDescription, double dMinimum, double dMaximum);
    MCPToolOutputSchema *addNumberFieldWithDefault(const QString &strFieldName, const QString &strDescription, double dDefaultValue);

    MCPToolOutputSchema *addBooleanField(const QString &strFieldName, const QString &strDescription);
    MCPToolOutputSchema *addBooleanFieldWithDefault(const QString &strFieldName, const QString &strDescription, bool bDefaultValue);

    MCPToolOutputSchema *addIntegerField(const QString &strFieldName, const QString &strDescription);
    MCPToolOutputSchema *addIntegerFieldWithRange(const QString &strFieldName, const QString &strDescription, int nMinimum, int nMaximum);
    MCPToolOutputSchema *addIntegerFieldWithDefault(const QString &strFieldName, const QString &strDescription, int nDefaultValue);

    MCPToolOutputSchema *addNullField(const QString &strFieldName, const QString &strDescription);

    // Enum type
    MCPToolOutputSchema *addEnumField(const QString &strFieldName, const QString &strDescription, const QStringList &strEnumValues);

    // Array types
    MCPToolOutputSchema *addStringArrayField(const QString &strFieldName, const QString &strDescription);
    MCPToolOutputSchema *addStringArrayFieldWithSize(const QString &strFieldName, const QString &strDescription, int nMinItems, int nMaxItems);
    MCPToolOutputSchema *addNumberArrayField(const QString &strFieldName, const QString &strDescription);
    MCPToolOutputSchema *addNumberArrayFieldWithSize(const QString &strFieldName, const QString &strDescription, int nMinItems, int nMaxItems);
    MCPToolOutputSchema *addBooleanArrayField(const QString &strFieldName, const QString &strDescription);
    MCPToolOutputSchema *addBooleanArrayFieldWithSize(const QString &strFieldName, const QString &strDescription, int nMinItems, int nMaxItems);
    MCPToolOutputSchema *addArrayField(const QString &strFieldName, const QString &strDescription, const QString &strItemType);
    MCPToolOutputSchema *addArrayFieldWithSize(const QString &strFieldName, const QString &strDescription, const QString &strItemType, int nMinItems, int nMaxItems);

    // Object types (nested)
    MCPToolOutputSchema *addObjectField(const QString &strFieldName, const QString &strDescription, MCPToolOutputSchema *pObjectSchema);
    MCPToolOutputSchema *addObjectArrayField(const QString &strFieldName, const QString &strDescription, MCPToolOutputSchema *pItemSchema);

    MCPToolOutputSchema *addRequiredField(const QString &strFieldName);

    /**
     * @brief Convert to JSON object
     * @return JSON Schema object
     */
    QJsonObject toJsonObject() const;

    /**
     * @brief Convert to string
     * @return JSON formatted string
     */
    QString toString() const;

private:
    // Helper methods: create field schemas
    QJsonObject createFieldSchema(const QString &strType, const QString &strDescription) const;
    QJsonObject createFieldSchemaWithDefault(const QString &strType, const QString &strDescription, const QJsonValue &defaultValue) const;
    QJsonObject createNumberFieldSchemaWithRange(const QString &strDescription, double dMinimum, double dMaximum) const;
    QJsonObject createIntegerFieldSchemaWithRange(const QString &strDescription, int nMinimum, int nMaximum) const;
    QJsonObject createStringFieldSchemaWithLength(const QString &strDescription, int nMinLength, int nMaxLength) const;
    QJsonObject createArrayFieldSchema(const QString &strItemType, const QString &strDescription) const;
    QJsonObject createArrayFieldSchemaWithSize(const QString &strItemType, const QString &strDescription, int nMinItems, int nMaxItems) const;

private:
    QString m_strDescription;
    QJsonObject m_schema;
    QStringList m_requiredFields;
};