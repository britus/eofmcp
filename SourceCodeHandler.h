/**
 * @file SourceCodeHandler.h
 * @brief MCP Tool Handler für Source-Code Dateienverwaltung
 * @author Your Name
 * @date 2025-11-30
 * @copyright Copyright (c) 2025 Your Company. All rights reserved.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QList>

/**
 * @brief Handler für Source-Code Dateiverwaltung
 *
 * Verantwortung:
 * - Listing von Source-Code Dateien
 * - Lesen von Dateiinhalten
 * - Speichern von Änderungen
 * - Anzeige von Projektdateien
 *
 * Coding-Standard:
 * - Klassenmember mit m_ Präfix
 * - String-Variablen mit str Präfix
 * - Pointer mit p Präfix
 * - { und } auf separaten Zeilen
 */
class SourceCodeHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor
     * @param pParent Übergeordnetes QObject
     */
    explicit SourceCodeHandler(QObject* pParent = nullptr);

    /**
     * @brief Destruktor
     */
    virtual ~SourceCodeHandler();

public slots:
    /**
     * @brief Listet alle Source-Code Dateien im Projektverzeichnis auf
     * @param jsonInput JSON-Objekt mit Eingabeparametern
     * @return JSON-Objekt mit Ergebnissen
     */
    QJsonObject listSourceFiles(const QJsonObject& jsonInput);

    /**
     * @brief Liest den Inhalt einer Source-Code Datei
     * @param jsonInput JSON-Objekt mit Dateipfad
     * @return JSON-Objekt mit Dateiinhalt
     */
    QJsonObject readSourceFile(const QJsonObject& jsonInput);

    /**
     * @brief Speichert Änderungen an einer Source-Code Datei
     * @param jsonInput JSON-Objekt mit Dateipfad und neuem Inhalt
     * @return JSON-Objekt mit Ergebnisstatus
     */
    QJsonObject writeSourceFile(const QJsonObject& jsonInput);

    /**
     * @brief Zeigt alle Source-Code Dateien im Projekt an
     * @param jsonInput JSON-Objekt mit Projektpfad
     * @return JSON-Objekt mit Dateiliste und Zusammenfassung
     */
    QJsonObject displayProjectFiles(const QJsonObject& jsonInput);

private:
    /**
     * @brief Findet alle Source-Code Dateien in einem Verzeichnis
     * @param strPath Verzeichnispath
     * @param strExtensions Liste der zu suchenden Dateierweiterungen
     * @param bRecursive Rekursiv durch Unterverzeichnisse suchen
     * @return Liste von QFileInfo Objekten
     */
    QList<QFileInfo> findSourceFiles(
        const QString& strPath,
        const QStringList& strExtensions,
        bool bRecursive = true
    );

    /**
     * @brief Validiert einen Dateipfad
     * @param strPath Der zu validierende Pfad
     * @return true wenn gültig, false sonst
     */
    bool isValidPath(const QString& strPath);

    /**
     * @brief Erstellt ein Backup einer Datei
     * @param strOriginalPath Pfad zur Originaldatei
     * @return Pfad zum Backup oder leerer String bei Fehler
     */
    QString createBackup(const QString& strOriginalPath);

    /**
     * @brief Wandelt QFileInfo in JSON-Objekt um
     * @param fileInfo Das QFileInfo Objekt
     * @param strBaseDir Basis-Verzeichnispath
     * @return JSON-Objekt mit Dateiinformationen
     */
    QJsonObject fileInfoToJson(
        const QFileInfo& fileInfo,
        const QString& strBaseDir = QString()
    );

    /**
     * @brief Extrahiert Standard-Dateierweiterungen
     * @param jsonArray JSON-Array mit Erweiterungen oder null für Standard
     * @return QStringList mit Dateierweiterungen
     */
    QStringList getFileExtensions(const QJsonArray& jsonArray);

    /**
     * @brief Erstellt ein Fehler-JSON-Objekt
     * @param strErrorMsg Fehlermeldung
     * @return JSON-Objekt mit Fehlerinformationen
     */
    QJsonObject createErrorResponse(const QString& strErrorMsg);

private:
    // Standard-Dateierweiterungen für Source-Code
    static constexpr const char* DEFAULT_EXTENSIONS[] = {
        ".cpp", ".h", ".hpp", ".c", ".cc", ".cxx", ".hxx"
    };
};