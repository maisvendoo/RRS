TEMPLATE = lib

QT -= gui
QT += xml

DEFINES += VEHICLE_LIB

TARGET = vehicle

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)
    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -ldevice_d
    LIBS += -L../../../lib -lJournal_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -ldevice
    LIBS += -L../../../lib -lJournal
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../common-headers
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../device/include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../libJournal/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
