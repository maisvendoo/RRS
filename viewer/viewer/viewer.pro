TEMPLATE = app

CONFIG += qt

QT += core
QT += gui
QT += widgets
QT += network

TARGET = viewer

DESTDIR = ../../../bin

win32 {

    OSG_LIB_DIRECTORY = $$(OSG_BIN_PATH)
    OSG_INCLUDE_DIRECTORY = $(OSG_INCLUDE_PATH)


    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -L$$OSG_LIB_DIRECTORY -losgd
        LIBS += -L$$OSG_LIB_DIRECTORY -losgViewerd
        LIBS += -L$$OSG_LIB_DIRECTORY -losgDBd
        LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreadsd
        LIBS += -L$$OSG_LIB_DIRECTORY -losgGAd
        LIBS += -L$$OSG_LIB_DIRECTORY -losgUtild
        LIBS += -L$$OSG_LIB_DIRECTORY -losgTextd

        LIBS += -L../../../lib -lroute-loader_d
        LIBS += -L../../../lib -llibrary_d
        LIBS += -L../../../lib -lfilesystem_d
        LIBS += -L../../../lib -lTcpConnection_d

    } else {

        LIBS += -L$$OSG_LIB_DIRECTORY -losg
        LIBS += -L$$OSG_LIB_DIRECTORY -losgViewer
        LIBS += -L$$OSG_LIB_DIRECTORY -losgDB
        LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreads
        LIBS += -L$$OSG_LIB_DIRECTORY -losgGA
        LIBS += -L$$OSG_LIB_DIRECTORY -losgUtil
        LIBS += -L$$OSG_LIB_DIRECTORY -losgText

        LIBS += -L../../../lib -lroute-loader
        LIBS += -L../../../lib -llibrary
        LIBS += -L../../../lib -lfilesystem
        LIBS += -L../../../lib -lTcpConnection
    }

    LIBS += -lopengl32 -lglu32

    INCLUDEPATH += $$OSG_INCLUDE_DIRECTORY
}

unix {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -losgd
        LIBS += -losgViewerd
        LIBS += -losgDBd
        LIBS += -lOpenThreadsd
        LIBS += -losgGAd
        LIBS += -losgUtild
        LIBS += -losgTextd

        LIBS += -lopenvr_api

        LIBS += -L../../lib -losgVRViewer_d

        LIBS += -L../../../lib -lroute-loader_d
        LIBS += -L../../../lib -llibrary_d
        LIBS += -L../../../lib -lfilesystem_d
        LIBS += -L../../lib -lTcpConnection_d

    } else {

        LIBS +=  -losg
        LIBS +=  -losgViewer
        LIBS +=  -losgDB
        LIBS +=  -lOpenThreads
        LIBS +=  -losgGA
        LIBS +=  -losgUtil
        LIBS += -losgText

        LIBS += -lopenvr_api

        LIBS += -L../../lib -losgVRViewer

        LIBS += -L../../../lib -lroute-loader
        LIBS += -L../../../lib -llibrary
        LIBS += -L../../../lib -lfilesystem
        LIBS += -L../../lib -lTcpConnection
    }

    LIBS += -lGL
}

#QMAKE_CXXFLAGS += -pg
#QMAKE_LFLAGS += -pg
#QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
#LIBS += -lgcov

#CONFIG += force_debug_info

INCLUDEPATH += ../../common-headers
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../tcp-connection/include
INCLUDEPATH += ../route-loader/include
INCLUDEPATH += ../library/include
INCLUDEPATH += ../osgvrviewer/include
INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
