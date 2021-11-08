TEMPLATE = lib

CONFIG += qt

QT += core

DEFINES += FILESYSTEM_LIB

TARGET = filesystem

DESTDIR = ../../lib

win32 {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

    } else {

    }

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
