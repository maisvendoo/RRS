TEMPLATE = lib

QT -= gui
QT += xml

TARGET = vl60pk

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

#CONFIG += force_debug_info

INCLUDEPATH += ./include
INCLUDEPATH += ../vl60-equipment/include

INCLUDEPATH += ../../../common-headers
INCLUDEPATH += ../../../CfgReader/include
INCLUDEPATH += ../../../filesystem/include

INCLUDEPATH += ../../../simulator/solver/include
INCLUDEPATH += ../../../simulator/physics/include
INCLUDEPATH += ../../../simulator/vehicle/include
INCLUDEPATH += ../../../simulator/device/include

HEADERS += $$files(./include/*.h)
HEADERS += $$files(../vl60-equipment/include/*.h)
SOURCES += $$files(./src/*.cpp)
SOURCES += $$files(../vl60-equipment/src/*.cpp)
