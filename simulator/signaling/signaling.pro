TEMPLATE = lib

QT -= gui
QT += xml

DEFINES += SIGNALING_LIB

TARGET = signaling

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -lCfgReader_d

} else {

    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -lCfgReader
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
