DEFINES += LIBRARY_LIB

TEMPLATE = lib

CONFIG -= qt

TARGET = library

DESTDIR = ../../../lib

win32 {

    OSG_LIB_DIRECTORY = $$(OSG_BIN_PATH)
    OSG_INCLUDE_DIRECTORY = $$(OSG_INCLUDE_PATH)

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -L$$OSG_LIB_DIRECTORY -losgd
        LIBS += -L$$OSG_LIB_DIRECTORY -losgDBd

        LIBS += -L../../../lib -lfilesystem_d

    } else {

        LIBS += -L$$OSG_LIB_DIRECTORY -losg
        LIBS += -L$$OSG_LIB_DIRECTORY -losgDB

        LIBS += -L../../../lib -lfilesystem

    }

    INCLUDEPATH += $$OSG_INCLUDE_DIRECTORY
}

unix {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -losgd
        LIBS += -losgDBd

        LIBS += -L../../../lib -lfilesystem_d

    } else {

        LIBS +=  -losg
        LIBS +=  -losgDB

        LIBS += -L../../../lib -lfilesystem_d
    }
}

INCLUDEPATH += ../../common-headers
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)

