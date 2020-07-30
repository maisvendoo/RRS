TEMPLATE = app

QT -= gui
QT += core
QT += xml
QT += network

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

DESTDIR += ../../../../bin

TARGET = simulator

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../../lib -lCfgReader_d
    LIBS += -L../../../../lib -ludp-connection_d

} else {

    LIBS += -L../../../../lib -lCfgReader
    LIBS += -L../../../../lib -ludp-connection
}

win32{

    OPENAL_LIB_DIR = $$(OPENAL_BIN)
    OPENAL_INCLUDE_BIN = $$(OPENAL_INCLUDE)

    LIBS += -L$$OPENAL_LIB_DIR -lOpenAL32
    INCLUDEPATH += $$OPENAL_INCLUDE_BIN
}

unix{

    LIBS += -lopenal
}

INCLUDEPATH += ./include

INCLUDEPATH += ../../../common-headers/
INCLUDEPATH += ../../../CfgReader/include
INCLUDEPATH += ../../vehicle/include
INCLUDEPATH += ../udp-connection/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
