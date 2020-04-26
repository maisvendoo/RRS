TEMPLATE = lib

QT -= gui
QT += network

DEFINES += SIM_CLIENT_LIB

TARGET = sim-client

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)
    LIBS += -L../../../lib/ -lTcpConnection_d


} else {

    LIBS += -L../../../lib/ -lTcpConnection
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../tcp-connection/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
