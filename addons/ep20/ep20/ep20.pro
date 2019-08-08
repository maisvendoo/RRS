TEMPLATE = lib

QT -= gui
QT += xml

TARGET = ep20

DESTDIR = ../../../../modules/$$join(TARGET,,,)

CONFIG(debug, debug|release) {

    LIBS += -L../../../../lib -lCfgReader_d
    LIBS += -L../../../../lib -lphysics_d
    LIBS += -L../../../../lib -lvehicle_d
    LIBS += -L../../../../lib -ldevice_d
    LIBS += -L../../../lib -lfilesystem_d

} else {

    LIBS += -L../../../../lib -lCfgReader
    LIBS += -L../../../../lib -lphysics
    LIBS += -L../../../../lib -lvehicle
    LIBS += -L../../../../lib -ldevice
    LIBS += -L../../../lib -lfilesystem
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../../CfgReader/include

INCLUDEPATH += ../../../simulator/solver/include
INCLUDEPATH += ../../../simulator/physics/include
INCLUDEPATH += ../../../simulator/vehicle/include
INCLUDEPATH += ../../../simulator/device/include
INCLUDEPATH += ../../../filesystem/include

HEADERS += $$files(./include/*.h) \
    include/task-pant-state-down.h \
    include/current-kind-switch.h
SOURCES += $$files(./src/*.cpp) \
    src/current-kind-switch.cpp
