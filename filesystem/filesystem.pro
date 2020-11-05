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

    } else {

    }

    INCLUDEPATH += $$OSG_INCLUDE_DIRECTORY
}

unix {    

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)        

    } else {

    }
}

INCLUDEPATH += ../common-headers
INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
