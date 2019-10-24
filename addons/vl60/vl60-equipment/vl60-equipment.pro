TEMPLATE = lib

QT -= gui
QT += xml

TARGET = vl60-equipment

CONFIG += static

DESTDIR = ../../../../modules/vl60-common

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

INCLUDEPATH += ./include
INCLUDEPATH += ../../../CfgReader/include
INCLUDEPATH += ../../../filesystem/include

INCLUDEPATH += ../../../simulator/solver/include
INCLUDEPATH += ../../../simulator/physics/include
INCLUDEPATH += ../../../simulator/vehicle/include
INCLUDEPATH += ../../../simulator/device/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
