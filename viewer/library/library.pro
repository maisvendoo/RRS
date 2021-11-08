DEFINES += LIBRARY_LIB

TEMPLATE = lib

CONFIG += qt
QT += core

TARGET = library

DESTDIR = ../../../lib

win32 {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)       

        LIBS += -L../../../lib -lfilesystem_d

    } else {

        LIBS += -L../../../lib -lfilesystem

    }

}

unix {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -L../../../lib -lfilesystem_d

    } else {

        LIBS += -L../../../lib -lfilesystem
    }
}

INCLUDEPATH += ../../common-headers
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)

