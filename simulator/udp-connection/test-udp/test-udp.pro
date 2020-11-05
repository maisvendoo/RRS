TEMPLATE = app

QT -= gui
QT += core
QT += xml
QT += network

CONFIG += c++11
CONFIG += console


DESTDIR = ../../../../bin

TARGET = test-udp

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../../lib -lCfgReader_d
    LIBS += -L../../../../lib -ludp-connection_d

} else {

    LIBS += -L../../../../lib -lCfgReader
    LIBS += -L../../../../lib -ludp-connection
}

INCLUDEPATH += ./include

INCLUDEPATH += ../../../common-headers/
INCLUDEPATH += ../../../CfgReader/include
INCLUDEPATH += ../../vehicle/include
INCLUDEPATH += ../udp-connection/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
