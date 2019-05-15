DEFINES += DEVICE_LIB

TEMPLATE = lib

QT -= gui
QT += xml

TARGET = device

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)
    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -lfilesystem_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lphysics    
    LIBS += -L../../../lib -lfilesystem
}

INCLUDEPATH += ./include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../common-headers/include


HEADERS += $$files(./include/*.h) \
    include/device-export.h
SOURCES += $$files(./src/*.cpp)
