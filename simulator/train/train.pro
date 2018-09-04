TEMPLATE = lib

QT -= gui
QT += xml

DEFINES += TRAIN_LIB

TARGET = train

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)
    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -lvehicle_d
    LIBS += -L../../../lib -lsolver_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -lvehicle
    LIBS += -L../../../lib -lsolver
}

INCLUDEPATH += ./include
INCLUDEPATH += ../common-headers
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../vehicle/include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
