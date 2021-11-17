TEMPLATE = lib

QT -= gui
QT += xml

CONFIG += force_debug_info

DEFINES += SIGNALING_LIB

TARGET = signaling

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -lJournal_d
    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -ldevice_d

} else {

    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -lJournal
    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -ldevice
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../libJournal/include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../device/include
INCLUDEPATH += ../vehicle/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
