TEMPLATE = app

QT -= gui
QT += core
QT += xml

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

DESTDIR += ../../bin

TARGET = simulator

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../lib -lCfgReader_d

} else {

    LIBS += -L../../lib -lCfgReader
}

INCLUDEPATH += ./include
INCLUDEPATH += ../CfgReader/include

HEADERS += $$files(./include/*.h) \
    include/command-line.h \
    include/global-const.h
SOURCES += $$files(./src/*.cpp)
