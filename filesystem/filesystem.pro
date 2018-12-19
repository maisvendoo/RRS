TEMPLATE = lib

CONFIG += qt

QT += core

DEFINES += FILESYSTEM_LIB

TARGET = filesystem

DESTDIR = ../../lib

win32 {

    OSG_LIB_DIRECTORY = $$(OSG_BIN_PATH)
    OSG_INCLUDE_DIRECTORY = $$(OSG_INCLUDE_PATH)    

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -L$$OSG_LIB_DIRECTORY -losgd
        LIBS += -L$$OSG_LIB_DIRECTORY -losgDBd

    } else {

        LIBS += -L$$OSG_LIB_DIRECTORY -losg
        LIBS += -L$$OSG_LIB_DIRECTORY -losgDB

    }

    INCLUDEPATH += $$OSG_INCLUDE_DIRECTORY
}

unix {    

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -losgd
        LIBS += -losgDBd

    } else {

        LIBS +=  -losg
        LIBS +=  -losgDB
    }
}

INCLUDEPATH += ../common-headers
INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
