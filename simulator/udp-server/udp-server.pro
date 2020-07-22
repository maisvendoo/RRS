TEMPLATE = lib

QT -= gui
QT += network

DEFINES += UDP_SERVER_LIB

TARGET = udp-server

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)


} else {

}

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
