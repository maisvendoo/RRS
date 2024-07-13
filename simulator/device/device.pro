DEFINES += DEVICE_LIB

TEMPLATE = lib

QT -= gui
QT += xml

TARGET = device

DESTDIR = ../../../lib

#CONFIG += force_debug_info

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)
    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -lJournal_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -lJournal
}

INCLUDEPATH += ./include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../libJournal/include
INCLUDEPATH += ../../common-headers


HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
