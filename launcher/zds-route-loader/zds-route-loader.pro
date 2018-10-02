TEMPLATE = lib

QT += core
QT -= gui

CONFIG += c++11

TARGET = zds-route-loader

DESTDIR = ../../../lib

win32 {

    OSG_LIB_DIRECTORY = $$(OSG_BIN_PATH)
    OSG_INCLUDE_DIRECTORY = $$(OSG_INCLUDE_PATH)
}

unix {

    LIB_DIRECTORY = /opt/osg/lib64
    OSG_INCLUDE_DIRECTORY = /opt/osg/include
}

CONFIG(debug, debug|release) {    

    LIBS += -L$$OSG_LIB_DIRECTORY -losgd
    LIBS += -L$$OSG_LIB_DIRECTORY -losgDBd
    LIBS += -L$$OSG_LIB_DIRECTORY -losgUtild
    LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreadsd

    LIBS += -L../../../lib -lroute-loader_d
    LIBS += -L../../../lib -lfilesystem_d

} else {

    LIBS += -L$$OSG_LIB_DIRECTORY -losg
    LIBS += -L$$OSG_LIB_DIRECTORY -losgDB
    LIBS += -L$$OSG_LIB_DIRECTORY -losgUtil
    LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreads

    LIBS += -L../../../lib -lzds-route-loader
    LIBS += -L../../../lib -lfilesystem
}

INCLUDEPATH += $$OSG_INCLUDE_DIRECTORY
INCLUDEPATH += ../../common-headers/
INCLUDEPATH += ../../filesystem/include

INCLUDEPATH += ../route-loader/include

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)

