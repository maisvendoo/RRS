TEMPLATE = lib

QT -= gui
QT += xml

TARGET = default-coupling

DESTDIR = ../../../modules

CONFIG(debug, debug|release) {

    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -lcoupling_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -lcoupling
}

INCLUDEPATH += ./include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../coupling/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)

