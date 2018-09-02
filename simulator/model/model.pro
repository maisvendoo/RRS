TEMPLATE = lib

QT -= gui

DEFINES += MODEL_LIB

TARGET = model

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

} else {

}

INCLUDEPATH += ./include
INCLUDEPATH += ../common-headers/

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
