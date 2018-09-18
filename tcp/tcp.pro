TEMPLATE = lib

QT -= gui

DESTDIR = ../../lib
TARGET = tcp

DEFINES += TCP_LIB

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

} else {


}

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)

