TEMPLATE = lib

QT -= gui
QT += xml
QT += network

DEFINES += UDP_CONNECTION_LIB

TARGET = udp-connection

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)
    LIBS += -L../../../lib -lCfgReader_d

} else {
    LIBS += -L../../../lib -lCfgReader
}

INCLUDEPATH += ./include
INCLUDEPATH += ../vehicle/include
INCLUDEPATH += ../../CfgReader/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
