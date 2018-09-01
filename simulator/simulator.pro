TEMPLATE = app

QT -= gui
QT += core

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

DESTDIR += ../../bin

TARGET = simulator

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

} else {


}

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h) \
    include/command-line.h
SOURCES += $$files(./src/*.cpp)
