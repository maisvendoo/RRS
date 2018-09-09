TEMPLATE = lib

DEFINES += BRAKEPIPE_LIB

QT -= qui
QT += xml

TARGET = brakepipe
DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TRAGET,,,_d)

    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lphysics_d

} else {

    LIBS += -L../../../lib -lCfgReader    
    LIBS += -L../../../lib -lphysics
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../physics/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
