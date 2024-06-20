TEMPLATE = lib

QT -= gui

DEFINES += PROFILE_LIB

TARGET = profile

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -lJournal_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -lvehicle_d

} else {

    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -lJournal
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -lvehicle
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../common-headers
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../libJournal/include
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../vehicle/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
