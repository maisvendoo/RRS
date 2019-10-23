TEMPLATE = lib

QT -= gui
QT += xml

TARGET = vl60k

DESTDIR = ../../../../modules/$$join(TARGET,,,)

CONFIG(debug, debug|release) {

    LIBS += -L../../../../lib -lCfgReader_d
    LIBS += -L../../../../lib -lphysics_d
    LIBS += -L../../../../lib -lvehicle_d
    LIBS += -L../../../../lib -ldevice_d
    LIBS += -L../../../../lib -lfilesystem_d

} else {

    LIBS += -L../../../../lib -lCfgReader
    LIBS += -L../../../../lib -lphysics
    LIBS += -L../../../../lib -lvehicle
    LIBS += -L../../../../lib -ldevice
    LIBS += -L../../../../lib -lfilesystem
}

LIBS += -L../../../../modules/vl60-common -lvl60-equipment

INCLUDEPATH += ./include
INCLUDEPATH += ../vl60-equipment/include
INCLUDEPATH += ../../../CfgReader/include
INCLUDEPATH += ../../../filesystem/include

INCLUDEPATH += ../../../simulator/solver/include
INCLUDEPATH += ../../../simulator/physics/include
INCLUDEPATH += ../../../simulator/vehicle/include
INCLUDEPATH += ../../../simulator/device/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
