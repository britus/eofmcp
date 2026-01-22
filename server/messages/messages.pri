INCLUDEPATH  += $$PWD
QMAKE_INCDIR += $$PWD

HEADERS += \
    $$PWD/MCPMessageType.h \
    $$PWD/MCPMessage.h \
    $$PWD/MCPMessageSender.h \
    $$PWD/MCPClientInitializeMessage.h \
    $$PWD/MCPClientMessage.h  \
    $$PWD/MCPServerMessage.h

SOURCES += \
    $$PWD/MCPMessageType.cpp \
    $$PWD/MCPMessage.cpp \
    $$PWD/MCPClientInitializeMessage.cpp \
    $$PWD/MCPClientMessage.cpp \
    $$PWD/MCPMessageSender.cpp \
    $$PWD/MCPServerMessage.cpp
