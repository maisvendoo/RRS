TEMPLATE = lib

QT -= gui
QT += xml

TARGET = default-vehicle

DESTDIR = ../../../modules/$$join(TARGET,,,)

CONFIG(debug, debug|release) {

    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -lvehicle_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -lvehicle
}

INCLUDEPATH += ./include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../vehicle/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
