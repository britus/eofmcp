/**
 * @file MCPResourceWrapper.h
 * @brief MCP resource wrapper (wrapping QObject as MCPResource)
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#pragma once
#include "MCPResource.h"
#include <QMetaObject>
#include <QMetaMethod>

/**
 * @brief MCP resource wrapper
 * 
 * Responsibilities:
 * - Wrap any QObject object as MCPResource
 * - Dynamically call wrapped object's methods and signals through Qt Meta mechanism
 * - Validate the wrapped object meets minimum implementation requirements at construction time
 * - Provide friendly error handling and validation feedback
 * 
 * The wrapped object needs to provide the following interface:
 * - QJsonObject getMetadata() const;  // slot method, get resource metadata
 * - QString getContent() const;        // slot method, get resource content
 * - void changed(const QString&, const QString&, const QString&);  // signal, notify resource changes
 * 
 * Use cases:
 * - When there's an existing QObject that implements the resource interface, and needs to be adapted as MCPResource
 * 
 * Coding conventions:
 * - Class members add m_ prefix
 * - Pointer types add p prefix
 * - { and } should be on separate lines
 */
class MCPResourceWrapper : public MCPResource
{
    Q_OBJECT
    
public:
    virtual ~MCPResourceWrapper();
    
    /**
     * @brief Static creation function, creates an MCPResourceWrapper object
     * @param strUri Resource URI
     * @param pWrappedObject Pointer to the QObject object to wrap
     * @param pParent Parent object
     * @return Pointer to created MCPResourceWrapper object, returns nullptr if pWrappedObject is null or doesn't meet minimum requirements
     * 
     * This function will automatically get the minimal interface metadata (getMetadata, getContent methods, changed signal) 
     * from QObject and perform parameter validation. After passing validation, it calls the private constructor to create the object
     * 
     * Usage example:
     * @code
     * QObject* pMyObject = new MyObject();
     * MCPResourceWrapper* pWrapper = MCPResourceWrapper::create("uri://my-resource", pMyObject);
     * if (pWrapper)
     * {
     *     // Use the wrapper (create() guarantees object is valid)
     * }
     * @endcode
     */
    static MCPResourceWrapper* create(const QString& strUri,
                                      QObject* pWrappedObject,
                                      QObject* pParent = nullptr);
    
public:
    /**
     * @brief Read resource content (implements base class pure virtual function)
     * @return Resource content string
     * 
     * Call the wrapped object's getContent() method through Qt Meta mechanism
     */
    QString readContent() const override;
    
    /**
     * @brief Get resource metadata (override base class method)
     * @return Resource metadata object
     * 
     * Call the wrapped object's getMetadata() method through Qt Meta mechanism
     */
    QJsonObject getMetadata() const override;
    
    /**
     * @brief Get resource annotations (Annotations) (override base class method)
     * @return Resource annotation object, containing fields such as audience, priority, lastModified
     * 
     * If the wrapped object implements getAnnotations() method, call it
     * Otherwise return base class implementation (possibly an empty object)
     */
    QJsonObject getAnnotations() const override;
    
    /**
     * @brief Get pointer to wrapped object
     * @return Pointer to wrapped object
     */
    QObject* getWrappedObject() const;
    
private:
    /**
     * @brief Validate changed signal
     * @param changedSignal QMetaMethod of the obtained changed signal
     * @param strErrorMessage Output parameter, error message when validation fails
     * @return Returns true if validation passes, otherwise returns false
     */
    static bool validateChangedSignal(const QMetaMethod& changedSignal, QString& strErrorMessage);
    
    /**
     * @brief Validate getMetadata method
     * @param getMetadataMethod QMetaMethod of the obtained getMetadata method
     * @param strErrorMessage Output parameter, error message when validation fails
     * @return Returns true if validation passes, otherwise returns false
     */
    static bool validateGetMetadata(const QMetaMethod& getMetadataMethod, QString& strErrorMessage);
    
    /**
     * @brief Validate getContent method
     * @param getContentMethod QMetaMethod of the obtained getContent method
     * @param strErrorMessage Output parameter, error message when validation fails
     * @return Returns true if validation passes, otherwise returns false
     */
    static bool validateGetContent(const QMetaMethod& getContentMethod, QString& strErrorMessage);
    
    /**
     * @brief Private constructor
     * @param strUri Resource URI
     * @param pWrappedObject Pointer to the QObject object to wrap (must not be null)
     * @param changedSignal QMetaMethod of the changed signal (must be valid)
     * @param getMetadata QMetaMethod of the getMetadata method (must be valid)
     * @param getContent QMetaMethod of the getContent method (must be valid)
     * @param getAnnotations QMetaMethod of the getAnnotations method (optional, may be invalid)
     * @param pParent Parent object
     * 
     * Note:
     * - This constructor is private, can only be called by create() static function
     * - Constructor does not perform parameter validation, assumes all parameters have been validated by create()
     * - Will get metadata (name, description, mimeType, annotations) from the pWrappedObject's getMetadata() method
     */
    explicit MCPResourceWrapper(const QString& strUri,
                                QObject* pWrappedObject,
                                const QMetaMethod& changedSignal,
                                const QMetaMethod& getMetadata,
                                const QMetaMethod& getContent,
                                const QMetaMethod& getAnnotations,
                                QObject* pParent = nullptr);
    
    /**
     * @brief Initialize wrapper connection
     * Connects the wrapped object's changed signal to onWrappedObjectChanged slot
     * Connects the wrapped object's destroyed signal to onWrappedObjectDestroyed slot
     */
    void initWrapperConnection();
    
    /**
     * @brief Get metadata from wrapped object and update local properties
     * Extract name, description, mimeType from the JSON returned by getMetadata() of wrapped object
     * If the wrapped object implements getAnnotations() method, also extract annotations and set to base class member variables
     */
    void updatePropertiesFromWrappedObject();
    
private slots:
    /**
     * @brief Handle changed signal from wrapped object
     * When the wrapped object emits a changed signal, forward it as MCPResource's changed signal
     */
    void onWrappedObjectChanged(const QString& strName, const QString& strDescription, const QString& strMimeType);
    
    /**
     * @brief Handle destroyed signal from wrapped object
     * When the wrapped object is deleted, emit resource invalidation signal
     */
    void onWrappedObjectDestroyed();
    
private:
    QObject* m_pWrappedObject;        // Pointer to the wrapped QObject object
    QMetaMethod m_changedSignal;     // QMetaMethod of changed signal
    QMetaMethod m_getMetadata;       // QMetaMethod of getMetadata method
    QMetaMethod m_getContent;        // QMetaMethod of getContent method
    QMetaMethod m_getAnnotations;    // QMetaMethod of getAnnotations method (optional, may be invalid)
};