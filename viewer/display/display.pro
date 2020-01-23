DEFINES += DISPLAY_LIB

TEMPLATE = lib

QT += core
QT += gui
QT += widgets

DESTDIR = ../../../lib

TARGET = display

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

} else {


}

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
