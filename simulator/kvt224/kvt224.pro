DEFINES += DEVICE_LIB

TEMPLATE = lib

QT -= gui
QT += xml

TARGET = kvt224

DESTDIR = ../../../modules

CONFIG(debug, debug|release) {

    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -ldevice_d
    LIBS += -L../../../lib -lfilesystem_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -ldevice
    LIBS += -L../../../lib -lfilesystem
}

INCLUDEPATH += ./include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../device/include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../common-headers


HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)

