// ********************************************************************
// EoF MCP Server
// Copyright © 2025 by EoF Software Labs
// SPDX-License-Identifier: GPLv3
// ********************************************************************
#pragma once

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>

/**
 * @brief Handler for Source Code File Management
 *
 * Responsibilities:
 * - Listing source code files
 * - Reading file contents
 * - Saving changes
 * - Displaying project files
 *
 * Coding standard:
 * - Class members with m_ prefix
 * - String variables with str prefix
 * - Pointers with p prefix
 * - { and } on separate lines
 */
class SourceCodeHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param pParent Parent QObject
     */
    explicit SourceCodeHandler(QObject *pParent = nullptr);

    /**
     * @brief Destructor
     */
    virtual ~SourceCodeHandler();

public slots:
    /**
     * @brief Lists all source code files in the project directory
     * @param project_path Project path
     * @param extensions File extension filter
     * @return JSON object with results
     */
    Q_INVOKABLE QJsonObject listSourceFiles(const QVariant &project_path, const QVariant &extensions);

    /**
     * @brief Reads the contents of a source code file
     * @param jsonInput JSON object with file path
     * @return JSON object with file content
     */
    Q_INVOKABLE QJsonObject readSourceFile(const QVariant &file_path);

    /**
     * @brief Reads the contents of a source code file
     * @param jsonInput JSON object with file path
     * @return JSON object with file content
     */
    Q_INVOKABLE QJsonObject readSourceFile(const QVariant &file_path, const QVariant length, const QVariant &offset);

    /**
     * @brief Saves changes to a source code file
     * @param file_path file path
     * @param content new content
     * @param create_backup true or false
     * @return JSON object with result status
     */
    Q_INVOKABLE QJsonObject writeSourceFile(const QVariant &file_path, const QVariant &content, const QVariant &create_backup);

    /**
     * @brief Displays all source code files in the project
     * @param project_path Path name
     * @param recursive true/false
     * @param sort_by sorting
     * @return JSON object with file list and summary
     */
    Q_INVOKABLE QJsonObject displayProjectFiles(const QVariant &project_path, const QVariant &recursive, const QVariant &sort_by);

private:
    /**
     * @brief Finds all source code files in a directory
     * @param strPath Directory path
     * @param strExtensions List of file extensions to search for
     * @param bRecursive Search through subdirectories recursively
     * @return List of QFileInfo objects
     */
    QList<QFileInfo> findSourceFiles(const QString &strPath, const QStringList &strExtensions, bool bRecursive = true);

    /**
     * @brief Validates a file path
     * @param strPath The path to validate
     * @return true if valid, false otherwise
     */
    bool isValidPath(const QString &strPath);

    /**
     * @brief Creates a backup of a file
     * @param strOriginalPath Path to the original file
     * @return Path to the backup or an empty string on error
     */
    QString createBackup(const QString &strOriginalPath);

    /**
     * @brief Converts QFileInfo to JSON object
     * @param fileInfo The QFileInfo object
     * @param strBaseDir Base directory path
     * @return JSON object with file information
     */
    QJsonObject fileInfoToJson(const QFileInfo &fileInfo, const QString &strBaseDir = QString());

    /**
     * @brief Extracts standard file extensions
     * @param jsonArray JSON array with extensions or null for defaults
     * @return QStringList with file extensions
     */
    QStringList getFileExtensions(const QJsonArray &jsonArray);

    /**
     * @brief Creates an error JSON object
     * @param strErrorMsg Error message
     * @return JSON object with error information
     */
    QJsonObject createErrorResponse(const QString &strErrorMsg);

private:
    // Standard file extensions for source code
    QStringList DEFAULT_EXTENSIONS = QStringList() //
                                     << ".cpp"     //
                                     << ".h"       //
                                     << ".hpp"     //
                                     << ".c"       //
                                     << ".cc"      //
                                     << ".cxx"     //
                                     << ".hxx"     //
                                     << ".java";
};
