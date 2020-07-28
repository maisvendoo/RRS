TEMPLATE = lib

QT -= gui
QT += xml
QT += network

DEFINES += UDP_SERVER_LIB

TARGET = udp-server

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
INCLUDEPATH += ../../udp-client/include

HEADERS += $$files(./include/*.h)
HEADERS += $$files(../../common-headers/*.h)
SOURCES += $$files(./src/*.cpp)
