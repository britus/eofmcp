/**
 * @file MCPResourceWrapper.cpp
 * @brief MCP resource wrapper implementation
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPResourceWrapper.h"
#include "MCPLog.h"
#include "MCPMetaObjectHelper.h"
#include <QMetaMethod>
#include <QMetaObject>
#include <QThread>
#include <QTimer>
#include <QVariant>

MCPResourceWrapper::MCPResourceWrapper( //
    const QString &strUri,
    QObject *pWrappedObject,          //
    const QMetaMethod &changedSignal, //
    const QMetaMethod &getMetadata,   //
    const QMetaMethod &getContent,    //
    const QMetaMethod &getAnnotations,
    QObject *pParent)
    : MCPResource(strUri, pParent)
    , m_pWrappedObject(pWrappedObject)
    , m_changedSignal(changedSignal)
    , m_getMetadata(getMetadata)
    , m_getContent(getContent)
    , m_getAnnotations(getAnnotations) // Passed from create() method, may be invalid
{
    QTimer::singleShot(5, [this]() {
        // Get metadata (name, description, mimeType, annotations) from the wrapped object's getMetadata() method
        updatePropertiesFromWrappedObject();
        // Initialize wrapper connection
        initWrapperConnection();
    });
}

MCPResourceWrapper::~MCPResourceWrapper()
{
    //-
}

MCPResourceWrapper *MCPResourceWrapper::create(const QString &strUri, QObject *pWrappedObject, QObject *pParent)
{
    if (pWrappedObject == nullptr) {
        MCP_CORE_LOG_CRITICAL() << "MCPResourceWrapper::create: Wrapped object cannot be null";
        return nullptr;
    }

    if (strUri.isEmpty()) {
        MCP_CORE_LOG_CRITICAL() << "MCPResourceWrapper::create: URI cannot be empty";
        return nullptr;
    }

    QString strErrorMessage;

    // Get changed signal and validate it
    QMetaMethod changedSignal = MCPMetaObjectHelper::getSignal(pWrappedObject, "changed(QString,QString,QString)");
    if (!changedSignal.isValid()) {
        MCP_CORE_LOG_CRITICAL() << "MCPResourceWrapper::create: Missing changed(QString,QString,QString) signal";
        return nullptr;
    }
    if (!validateChangedSignal(changedSignal, strErrorMessage)) {
        MCP_CORE_LOG_CRITICAL() << "MCPResourceWrapper::create: Wrapped object does not meet minimum implementation requirements:" << strErrorMessage;
        return nullptr;
    }

    // Get getMetadata() method and validate it
    QMetaMethod getMetadataMethod = MCPMetaObjectHelper::getMethod(pWrappedObject, "getMetadata()");
    if (!getMetadataMethod.isValid()) {
        MCP_CORE_LOG_CRITICAL() << "MCPResourceWrapper::create: Missing getMetadata() method";
        return nullptr;
    }
    if (!validateGetMetadata(getMetadataMethod, strErrorMessage)) {
        MCP_CORE_LOG_CRITICAL() << "MCPResourceWrapper::create: Wrapped object does not meet minimum implementation requirements:" << strErrorMessage;
        return nullptr;
    }

    // Get getContent() method and validate it
    QMetaMethod getContentMethod = MCPMetaObjectHelper::getMethod(pWrappedObject, "getContent()");
    if (!getContentMethod.isValid()) {
        MCP_CORE_LOG_CRITICAL() << "MCPResourceWrapper::create: Missing getContent() method";
        return nullptr;
    }
    if (!validateGetContent(getContentMethod, strErrorMessage)) {
        MCP_CORE_LOG_CRITICAL() << "MCPResourceWrapper::create: Wrapped object does not meet minimum implementation requirements:" << strErrorMessage;
        return nullptr;
    }

    // Try to get getAnnotations() method (optional)
    QMetaMethod getAnnotationsMethod;
    QMetaMethod getAnnotationsMethodCandidate = MCPMetaObjectHelper::getMethod(pWrappedObject, "getAnnotations()");
    if (getAnnotationsMethodCandidate.isValid() && getAnnotationsMethodCandidate.returnType() == QMetaType::QJsonObject && getAnnotationsMethodCandidate.parameterCount() == 0) {
        getAnnotationsMethod = getAnnotationsMethodCandidate;
        //MCP_CORE_LOG_DEBUG() << "MCPResourceWrapper::create: Wrapped object supports getAnnotations() method";
    }

    // All validations passed, create the object
    MCPResourceWrapper *pWrapper = new MCPResourceWrapper(strUri, pWrappedObject, changedSignal, getMetadataMethod, getContentMethod, getAnnotationsMethod, pParent);

    return pWrapper;
}

QString MCPResourceWrapper::readContent() const
{
    // Check if the wrapped object has been deleted
    if (m_pWrappedObject == nullptr) {
        MCP_CORE_LOG_WARNING() << "MCPResourceWrapper: Wrapped object has been deleted, cannot get content";
        return QString();
    }

    // Call the wrapped object's getContent() method using the saved QMetaMethod
    // Use Qt::AutoConnection to handle threading automatically:
    // - Same thread: use DirectConnection (direct call)
    // - Cross-thread: use QueuedConnection (asynchronous call, but here we need return value, so using BlockingQueuedConnection)
    QString strContent;
    m_getContent.invoke(m_pWrappedObject,
                        QThread::currentThread() == m_pWrappedObject->thread() //
                            ? Qt::DirectConnection                             //
                            : Qt::BlockingQueuedConnection,
                        Q_RETURN_ARG(QString, strContent));
    return strContent;
}

QObject *MCPResourceWrapper::getWrappedObject() const
{
    return m_pWrappedObject;
}

QJsonObject MCPResourceWrapper::getMetadata() const
{
    // Check if the wrapped object has been deleted
    if (m_pWrappedObject == nullptr) {
        MCP_CORE_LOG_WARNING() << "MCPResourceWrapper: Wrapped object has been deleted, return base metadata";
        return MCPResource::getMetadata();
    }

    // Call the wrapped object's getMetadata() method using the saved QMetaMethod
    // Use Qt::AutoConnection to handle threading automatically:
    // - Same thread: use DirectConnection (direct call)
    // - Cross-thread: use BlockingQueuedConnection (synchronous call to get return value)
    QJsonObject metadata;

    // Check if cross-thread
    m_getMetadata.invoke(m_pWrappedObject, QThread::currentThread() == m_pWrappedObject->thread() ? Qt::DirectConnection : Qt::BlockingQueuedConnection, Q_RETURN_ARG(QJsonObject, metadata));
    return metadata;
}

QJsonObject MCPResourceWrapper::getAnnotations() const
{
    // Check if the wrapped object has been deleted
    if (m_pWrappedObject == nullptr) {
        MCP_CORE_LOG_WARNING() << "MCPResourceWrapper: Wrapped object has been deleted, return base annotations";
        return MCPResource::getAnnotations();
    }

    // If the wrapped object implements getAnnotations() method, call it
    if (m_getAnnotations.isValid()) {
        QJsonObject annotations;
        m_getAnnotations.invoke(m_pWrappedObject, QThread::currentThread() == m_pWrappedObject->thread() ? Qt::DirectConnection : Qt::BlockingQueuedConnection, Q_RETURN_ARG(QJsonObject, annotations));
        return annotations;
    }

    // If the wrapped object does not implement getAnnotations() method, return base implementation
    return MCPResource::getAnnotations();
}

bool MCPResourceWrapper::validateChangedSignal(const QMetaMethod &changedSignal, QString &strErrorMessage)
{
    if (!changedSignal.isValid()) {
        strErrorMessage = "changed signal is invalid";
        return false;
    }

    if (changedSignal.parameterCount() != 3) {
        strErrorMessage = "changed signal must have 3 QString parameters";
        return false;
    }

    // Validate parameter types are all QString
    if (changedSignal.parameterType(0) != QMetaType::QString || changedSignal.parameterType(1) != QMetaType::QString || changedSignal.parameterType(2) != QMetaType::QString) {
        strErrorMessage = "all 3 parameters of changed signal must be QString type";
        return false;
    }

    return true;
}

bool MCPResourceWrapper::validateGetMetadata(const QMetaMethod &getMetadataMethod, QString &strErrorMessage)
{
    if (!getMetadataMethod.isValid()) {
        strErrorMessage = "getMetadata() method is invalid";
        return false;
    }

    if (getMetadataMethod.returnType() != QMetaType::QJsonObject) {
        strErrorMessage = "getMetadata() method must return QJsonObject type";
        return false;
    }

    if (getMetadataMethod.parameterCount() != 0) {
        strErrorMessage = "getMetadata() method must have no parameters";
        return false;
    }

    return true;
}

bool MCPResourceWrapper::validateGetContent(const QMetaMethod &getContentMethod, QString &strErrorMessage)
{
    if (!getContentMethod.isValid()) {
        strErrorMessage = "getContent() method is invalid";
        return false;
    }

    if (getContentMethod.returnType() != QMetaType::QString) {
        strErrorMessage = "getContent() method must return QString type";
        return false;
    }

    if (getContentMethod.parameterCount() != 0) {
        strErrorMessage = "getContent() method must have no parameters";
        return false;
    }

    return true;
}

void MCPResourceWrapper::onWrappedObjectChanged(const QString &strName, const QString &strDescription, const QString &strMimeType)
{
    // Check if the wrapped object has been deleted (theoretically shouldn't happen, but for safety)
    if (m_pWrappedObject == nullptr) {
        MCP_CORE_LOG_WARNING() << "MCPResourceWrapper: Wrapped object has been deleted, ignore changed signal";
        return;
    }

    // Update local properties (without emitting signals since this synchronizes data from the wrapped object)
    m_strName = strName;
    m_strDescription = strDescription;
    m_strMimeType = strMimeType;

    // Forward changed signal
    emit changed(strName, strDescription, strMimeType);
}

void MCPResourceWrapper::initWrapperConnection()
{
    // Use QMetaObject::connect to connect the wrapped object's changed signal to onWrappedObjectChanged slot
    // Note: QObject::connect cannot directly use QMetaMethod, need to use QMetaObject::connect
    const QMetaObject *pMetaObject = this->metaObject();
    int nSlotIndex = pMetaObject->indexOfSlot("onWrappedObjectChanged(QString,QString,QString)");

    if (nSlotIndex >= 0) {
        bool bConnected = QMetaObject::connect(m_pWrappedObject, m_changedSignal.methodIndex(), this, nSlotIndex, Qt::AutoConnection, nullptr);
        if (!bConnected) {
            MCP_CORE_LOG_WARNING() << "MCPResourceWrapper: Cannot connect wrapped object's changed() signal";
        } else {
            MCP_CORE_LOG_DEBUG() << "MCPResourceWrapper: Connected wrapped object's changed() signal";
        }
    } else {
        MCP_CORE_LOG_WARNING() << "MCPResourceWrapper: Cannot find onWrappedObjectChanged slot";
    }

    // Connect the wrapped object's destroyed signal to onWrappedObjectDestroyed slot
    // When the wrapped object is deleted, automatically emit resource invalidation signal
    bool bConnected = QObject::connect(m_pWrappedObject, &QObject::destroyed, this, &MCPResourceWrapper::onWrappedObjectDestroyed);
    if (!bConnected) {
        MCP_CORE_LOG_WARNING() << "MCPResourceWrapper: Cannot connect wrapped object's destroyed() signal";
    } else {
        MCP_CORE_LOG_DEBUG() << "MCPResourceWrapper: Connected wrapped object's destroyed() signal";
    }
}

void MCPResourceWrapper::onWrappedObjectDestroyed()
{
    // The wrapped object has been deleted, clear the pointer and send invalidation signal
    MCP_CORE_LOG_WARNING() << "MCPResourceWrapper: Wrapped object has been deleted, resource invalidated:" << getUri();
    m_pWrappedObject = nullptr;
    notifyInvalidated();
}

void MCPResourceWrapper::updatePropertiesFromWrappedObject()
{
    // Get metadata from wrapped object
    QJsonObject metadata = getMetadata(); // Calls the overridden getMetadata, which gets from wrapped object

    // Update local properties (without emitting signals since this synchronizes data from the wrapped object)
    if (metadata.contains("name")) {
        m_strName = metadata["name"].toString();
    }

    if (metadata.contains("description")) {
        m_strDescription = metadata["description"].toString();
    }

    if (metadata.contains("mimeType")) {
        m_strMimeType = metadata["mimeType"].toString();
    }

    // Extract annotations (if present)
    // Prefer to get from the annotations field in metadata
    if (metadata.contains("annotations") && metadata["annotations"].isObject()) {
        QJsonObject annotations = metadata["annotations"].toObject();
        setAnnotations(annotations);
    }
    // If metadata doesn't contain annotations, directly call getAnnotations() method
    // getAnnotations() will automatically handle: if the wrapped object implements it, call it; otherwise return base implementation
    else {
        QJsonObject annotations = getAnnotations();
        if (!annotations.isEmpty()) {
            setAnnotations(annotations);
        }
    }
}
