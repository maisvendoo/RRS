TEMPLATE = lib

QT -= gui
QT += xml

TARGET = chs2t

DESTDIR = ../../../../modules/$$join(TARGET,,,)

CONFIG(debug, debug|release) {

    LIBS += -L../../../../lib -lCfgReader_d
    LIBS += -L../../../../lib -lphysics_d
    LIBS += -L../../../../lib -lvehicle_d
    LIBS += -L../../../../lib -ldevice_d
    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -lJournal_d

} else {

    LIBS += -L../../../../lib -lCfgReader
    LIBS += -L../../../../lib -lphysics
    LIBS += -L../../../../lib -lvehicle
    LIBS += -L../../../../lib -ldevice
    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -lJournal
}

INCLUDEPATH += ./include
INCLUDEPATH += ../chs2t-equipment/include

INCLUDEPATH += ../../../CfgReader/include
INCLUDEPATH += ../../../simulator/solver/include
INCLUDEPATH += ../../../simulator/physics/include
INCLUDEPATH += ../../../simulator/vehicle/include
INCLUDEPATH += ../../../simulator/device/include
INCLUDEPATH += ../../../filesystem/include
INCLUDEPATH += ../../../libJournal/include

HEADERS += $$files(./include/*.h)
HEADERS += $$files(../chs2t-equipment/include/*.h)
SOURCES += $$files(./src/*.cpp)
SOURCES += $$files(../chs2t-equipment/src/*.cpp)

#CONFIG += force_debug_info
#QMAKE_CXXFLAGS += -Werror -pedantic-errors -Wall -Wextra -Wpedantic -Wmaybe-uninitialized -Wreturn-type -Warray-bounds=1
