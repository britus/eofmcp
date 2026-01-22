INCLUDEPATH  += $$PWD
QMAKE_INCDIR += $$PWD

SUBDIRS += \
    $$PWD/http

HEADERS += \
    $$PWD/IMCPTransport.h \
    $$PWD/MCPHttpTransport.h \
    $$PWD/MCPHttpTransportAdapter.h

SOURCES += \
    $$PWD/MCPHttpTransport.cpp \
    $$PWD/MCPHttpTransportAdapter.cpp
