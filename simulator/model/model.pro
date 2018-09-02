TEMPLATE = lib

QT -= gui

DEFINES += MODEL_LIB

TARGET = model

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -llog_d

} else {

    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -llog
}

INCLUDEPATH += ./include
INCLUDEPATH += ../common-headers
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../log/include

HEADERS += $$files(./include/*.h)
HEADERS += $$files(../common-headers/*.h)
SOURCES += $$files(./src/*.cpp)
