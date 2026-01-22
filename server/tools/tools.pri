INCLUDEPATH  += $$PWD
QMAKE_INCDIR += $$PWD

HEADERS += \
    $$PWD/MCPTool.h \
    $$PWD/IMCPToolService.h \
    $$PWD/MCPToolInputSchema.h \
    $$PWD/MCPToolOutputSchema.h \
    $$PWD/MCPToolNotificationHandler.h \
    $$PWD/MCPToolService.h

SOURCES += \
    $$PWD/MCPTool.cpp \
    $$PWD/MCPToolInputSchema.cpp \
    $$PWD/MCPToolOutputSchema.cpp \
    $$PWD/MCPToolNotificationHandler.cpp \
    $$PWD/MCPToolService.cpp
