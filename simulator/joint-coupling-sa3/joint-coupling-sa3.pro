TEMPLATE = lib

QT -= gui
QT += xml

TARGET = joint-coupling-sa3

DESTDIR = ../../../modules

CONFIG(debug, debug|release) {

    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -ldevice_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -ldevice
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../device/include
INCLUDEPATH += ../solver/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
