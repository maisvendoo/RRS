DEFINES += DEVICE_LIB

TEMPLATE = lib

QT -= gui
QT += xml

TARGET = freejoy

DESTDIR = ../../../plugins

CONFIG(debug, debug|release) {

    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -ldevice_d
    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -lJournal_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -ldevice
    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -lJournal
}

LIBS += -L$$(SFML_ROOT)/lib -lsfml-window-s
LIBS += -L$$(SFML_ROOT)/lib -lsfml-system-s

win32 {

    LIBS += -lwinmm
}

INCLUDEPATH += ./include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../device/include
INCLUDEPATH += ../../common-headers
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../libJournal/include
INCLUDEPATH += $$(SFML_ROOT)/include/

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
