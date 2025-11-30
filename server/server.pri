INCLUDEPATH += $$PWD

QMAKE_INCDIR += $$PWD/include

include($$PWD/3rdparty/3rdparty.pri)

HEADERS += \
    $$PWD/IMCPServer.h \
    $$PWD/IMCPPromptService.h \
    $$PWD/IMCPResourceService.h \
    $$PWD/IMCPServerConfig.h \
    $$PWD/IMCPTransport.h \
    $$PWD/IMCPMiddleware.h \
    $$PWD/IMCPToolService.h

HEADERS += \
    $$PWD/MCPHelper.h \
    $$PWD/MCPToolInputSchema.h \
    $$PWD/MCPToolOutputSchema.h \
    $$PWD/MCPLog.h \
    $$PWD/MCPClientInitializeMessage.h \
    $$PWD/MCPClientMessage.h \
    $$PWD/MCPMessage.h \
    $$PWD/MCPMessageSender.h \
    $$PWD/MCPMessageType.h \
    $$PWD/MCPServerMessage.h \
    $$PWD/MCPMiddlewares.h \
    $$PWD/MCPPrompt.h \
    $$PWD/MCPPromptService.h \
    $$PWD/MCPContentResource.h \
    $$PWD/MCPFileResource.h \
    $$PWD/MCPResource.h \
    $$PWD/MCPResourceService.h \
    $$PWD/MCPResourceWrapper.h \
    $$PWD/MCPContext.h \
    $$PWD/MCPInitializeHandler.h \
    $$PWD/MCPRequestDispatcher.h \
    $$PWD/MCPRouter.h \
    $$PWD/MCPSubscriptionHandler.h \
    $$PWD/MCPTool.h \
    $$PWD/MCPToolService.h \
    $$PWD/MCPHttpTransport.h \
    $$PWD/MCPHttpTransportAdapter.h \
    $$PWD/MCPPendingNotification.h \
    $$PWD/MCPSession.h \
    $$PWD/MCPSessionService.h \
    $$PWD/MCPServer.h \
    $$PWD/MCPNotificationHandlerBase.h \
    $$PWD/MCPPromptNotificationHandler.h \
    $$PWD/MCPResourceNotificationHandler.h \
    $$PWD/MCPServerHandler.h \
    $$PWD/MCPToolNotificationHandler.h \
    $$PWD/MCPError.h \
    $$PWD/MCPErrorCode.h \
    $$PWD/MCPErrorHandler.h \
    $$PWD/MCPPromptsConfig.h \
    $$PWD/MCPResourcesConfig.h \
    $$PWD/MCPServerConfig.h \
    $$PWD/MCPToolsConfig.h \
    $$PWD/MCPHandlerResolver.h \
    $$PWD/MCPInvokeHelper.h \
    $$PWD/MCPMetaObjectHelper.h \
    $$PWD/MCPMethodHelper.h \
    $$PWD/MCPResourceContentGenerator.h \
    $$PWD/MCPAutoServer.h

SOURCES += \
    $$PWD/MCPLog.cpp \
    $$PWD/MCPClientInitializeMessage.cpp \
    $$PWD/MCPClientMessage.cpp \
    $$PWD/MCPMessage.cpp \
    $$PWD/MCPMessageSender.cpp \
    $$PWD/MCPMessageType.cpp \
    $$PWD/MCPServerMessage.cpp \
    $$PWD/MCPMiddlewares.cpp \
    $$PWD/MCPPrompt.cpp \
    $$PWD/MCPPromptService.cpp \
    $$PWD/MCPContentResource.cpp \
    $$PWD/MCPFileResource.cpp \
    $$PWD/MCPResource.cpp \
    $$PWD/MCPResourceService.cpp \
    $$PWD/MCPResourceWrapper.cpp \
    $$PWD/MCPContext.cpp \
    $$PWD/MCPInitializeHandler.cpp \
    $$PWD/MCPRequestDispatcher.cpp \
    $$PWD/MCPRouter.cpp \
    $$PWD/MCPSubscriptionHandler.cpp \
    $$PWD/MCPTool.cpp \
    $$PWD/MCPToolService.cpp \
    $$PWD/MCPHttpTransport.cpp \
    $$PWD/MCPHttpTransportAdapter.cpp \
    $$PWD/MCPPendingNotification.cpp \
    $$PWD/MCPSession.cpp \
    $$PWD/MCPSessionService.cpp \
    $$PWD/IMCPServer.cpp \
    $$PWD/MCPServer.cpp \
    $$PWD/MCPNotificationHandlerBase.cpp \
    $$PWD/MCPPromptNotificationHandler.cpp \
    $$PWD/MCPResourceNotificationHandler.cpp \
    $$PWD/MCPServerHandler.cpp \
    $$PWD/MCPToolNotificationHandler.cpp \
    $$PWD/IMCPServerConfig.cpp \
    $$PWD/MCPPromptsConfig.cpp \
    $$PWD/MCPResourcesConfig.cpp \
    $$PWD/MCPServerConfig.cpp \
    $$PWD/MCPToolsConfig.cpp \
    $$PWD/MCPError.cpp \
    $$PWD/MCPErrorCode.cpp \
    $$PWD/MCPErrorHandler.cpp \
    $$PWD/MCPHandlerResolver.cpp \
    $$PWD/MCPHelper.cpp \
    $$PWD/MCPInvokeHelper.cpp \
    $$PWD/MCPMetaObjectHelper.cpp \
    $$PWD/MCPMethodHelper.cpp \
    $$PWD/MCPResourceContentGenerator.cpp \
    $$PWD/MCPToolInputSchema.cpp \
    $$PWD/MCPToolOutputSchema.cpp \
    $$PWD/MCPAutoServer.cpp

include($$PWD/impl/impl.pri)
