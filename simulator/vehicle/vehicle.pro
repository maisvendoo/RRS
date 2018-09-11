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

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lphysics
}

INCLUDEPATH += ./include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../../CfgReader/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
