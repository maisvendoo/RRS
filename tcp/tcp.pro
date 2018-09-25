TEMPLATE = lib

QT -= gui
QT += network

DESTDIR = ../../lib
TARGET = tcp

DEFINES += TCP_LIB

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)    

} else {

}

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)

