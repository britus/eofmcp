INCLUDEPATH  += $$PWD
QMAKE_INCDIR += $$PWD/include

HEADERS += \
    $$PWD/MCPLog.h \
    $$PWD/MCPHelper.h \
    $$PWD/MCPNotificationHandlerBase.h \
    $$PWD/MCPInvokeHelper.h \
    $$PWD/MCPMetaObjectHelper.h \
    $$PWD/MCPMethodHelper.h

SOURCES += \
    $$PWD/MCPLog.cpp \
    $$PWD/MCPHelper.cpp \
    $$PWD/MCPNotificationHandlerBase.cpp \
    $$PWD/MCPInvokeHelper.cpp \
    $$PWD/MCPMetaObjectHelper.cpp \
    $$PWD/MCPMethodHelper.cpp
