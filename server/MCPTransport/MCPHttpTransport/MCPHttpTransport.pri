INCLUDEPATH += $$PWD

include($$PWD/impl/impl.pri)

SOURCES = \
    MCPHttpTransport.cpp \
    MCPHttpTransportAdapter.cpp

HEADERS = \
    MCPHttpTransport.h \
    MCPHttpTransportAdapter.h
