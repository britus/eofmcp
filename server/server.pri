INCLUDEPATH  += $$PWD
QMAKE_INCDIR += $$PWD/include

SUBDIRS += \
    $$PWD/server/3rdparty \
    $$PWD/server/core \
    $$PWD/server/errors \
    $$PWD/server/config \
    $$PWD/server/messages \
    $$PWD/server/transport \
    $$PWD/server/session \
    $$PWD/server/routing \
    $$PWD/server/middleware \
    $$PWD/server/prompts \
    $$PWD/server/resources \
    $$PWD/server/tools

HEADERS += \
    $$PWD/MCPServer_global.h \
    $$PWD/IMCPServer.h \
    $$PWD/MCPServer.h \
    $$PWD/MCPServerHandler.h \
    $$PWD/MCPHandlerResolver.h \
    $$PWD/MCPAutoServer.h

SOURCES += \
    $$PWD/IMCPServer.cpp \
    $$PWD/MCPServer.cpp \
    $$PWD/MCPServerHandler.cpp \
    $$PWD/MCPHandlerResolver.cpp \
    $$PWD/MCPAutoServer.cpp
