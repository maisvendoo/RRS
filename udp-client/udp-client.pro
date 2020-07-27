TEMPLATE = lib

QT -= gui
QT += network

DEFINES += UDP_CLIENT_LIB

TARGET = udp-client

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)
#    LIBS += -L../../../lib -lCfgReader_d

} else {
#    LIBS += -L../../../lib -lCfgReader
}

INCLUDEPATH += ./include
#INCLUDEPATH += ../../common-headers
#INCLUDEPATH += ../../CfgReader/include

HEADERS += $$files(./include/*.h)
#HEADERS += $$files(../../common-headers/*.h)
SOURCES += $$files(./src/*.cpp)
