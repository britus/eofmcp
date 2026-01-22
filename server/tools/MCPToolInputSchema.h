/**
 * @file MCPToolInputSchema.h
 * @brief MCP tool input schema helper class
 * @author zhangheng
 * @date 2025-01-01
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include <MCPServer_global.h>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief MCPToolInputSchema helper class
 *
 * Provides type-safe input schema building API to simplify schema definition
 *
 * Use cases:
 * - Simple schemas: directly use JSON objects
 * - Complex schemas: use this helper class (chain calls are clearer)
 *
 * Usage example:
 * @code
 * MCPToolInputSchema schema;
 * schema.addStringField("name", "Username", true)
 *       .addNumberField("age", "Age", true)
 *       .addEnumField("gender", "Gender", QStringList() << "male" << "female", true);
 * QJsonObject jsonSchema = schema.toJsonObject();
 * @endcode
 *
 * Coding conventions:
 * - Class members add m_ prefix
 * - String types add str prefix
 * - Numeric types add n or d prefix
 * - { and } should be on separate lines
 */
class MCPCORE_EXPORT MCPToolInputSchema
{
public:
    MCPToolInputSchema();

    // Basic type fields
    MCPToolInputSchema *addStringField(const QString &strFieldName, const QString &strDescription, bool bRequired = true);
    MCPToolInputSchema *addStringFieldWithLength(const QString &strFieldName, const QString &strDescription, int nMinLength, int nMaxLength, bool bRequired = true);
    MCPToolInputSchema *addStringFieldWithFormat(const QString &strFieldName, const QString &strDescription, const QString &strFormat, bool bRequired = true);
    MCPToolInputSchema *addStringFieldWithDefault(const QString &strFieldName, const QString &strDescription, const QString &strDefaultValue, bool bRequired = false);

    MCPToolInputSchema *addNumberField(const QString &strFieldName, const QString &strDescription, bool bRequired = true);
    MCPToolInputSchema *addNumberFieldWithRange(const QString &strFieldName, const QString &strDescription, double dMinimum, double dMaximum, bool bRequired = true);
    MCPToolInputSchema *addNumberFieldWithDefault(const QString &strFieldName, const QString &strDescription, double dDefaultValue, bool bRequired = false);

    MCPToolInputSchema *addBooleanField(const QString &strFieldName, const QString &strDescription, bool bRequired = true);
    MCPToolInputSchema *addBooleanFieldWithDefault(const QString &strFieldName, const QString &strDescription, bool bDefaultValue, bool bRequired = false);

    MCPToolInputSchema *addIntegerField(const QString &strFieldName, const QString &strDescription, bool bRequired = true);
    MCPToolInputSchema *addIntegerFieldWithRange(const QString &strFieldName, const QString &strDescription, int nMinimum, int nMaximum, bool bRequired = true);
    MCPToolInputSchema *addIntegerFieldWithDefault(const QString &strFieldName, const QString &strDescription, int nDefaultValue, bool bRequired = false);

    // Enum type
    MCPToolInputSchema *addEnumField(const QString &strFieldName, const QString &strDescription, const QStringList &strEnumValues, bool bRequired = true);

    // Array types
    MCPToolInputSchema *addStringArrayField(const QString &strFieldName, const QString &strDescription, bool bRequired = true);
    MCPToolInputSchema *addStringArrayFieldWithSize(const QString &strFieldName, const QString &strDescription, int nMinItems, int nMaxItems, bool bRequired = true);
    MCPToolInputSchema *addNumberArrayField(const QString &strFieldName, const QString &strDescription, bool bRequired = true);
    MCPToolInputSchema *addNumberArrayFieldWithSize(const QString &strFieldName, const QString &strDescription, int nMinItems, int nMaxItems, bool bRequired = true);
    MCPToolInputSchema *addBooleanArrayField(const QString &strFieldName, const QString &strDescription, bool bRequired = true);
    MCPToolInputSchema *addBooleanArrayFieldWithSize(const QString &strFieldName, const QString &strDescription, int nMinItems, int nMaxItems, bool bRequired = true);
    MCPToolInputSchema *addArrayField(const QString &strFieldName, const QString &strDescription, const QString &strItemType, bool bRequired = true);
    MCPToolInputSchema *addArrayFieldWithSize(const QString &strFieldName, const QString &strDescription, const QString &strItemType, int nMinItems, int nMaxItems, bool bRequired = true);

    // Object types (nested)
    MCPToolInputSchema *addObjectField(const QString &strFieldName, const QString &strDescription, MCPToolInputSchema *pObjectSchema, bool bRequired = true);
    MCPToolInputSchema *addObjectArrayField(const QString &strFieldName, const QString &strDescription, MCPToolInputSchema *pItemSchema, bool bRequired = true);

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
    QJsonObject m_schema;
    QStringList m_requiredFields;
};