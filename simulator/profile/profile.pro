TEMPLATE = lib

QT -= gui

DEFINES += PROFILE_LIB

TARGET = profile

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../lib -lfilesystem_d

} else {


    LIBS += -L../../../lib -lfilesystem

}

INCLUDEPATH += ./include
INCLUDEPATH += ../../common-headers
INCLUDEPATH += ../../filesystem/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
