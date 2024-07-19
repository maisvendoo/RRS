TEMPLATE = app

CONFIG += qt

QT += core
QT += gui
QT += widgets
QT += network
QT += opengl

TARGET = viewer

DESTDIR = ../../../bin

win32 {

    OSG_LIB_DIRECTORY = $$(OSG_BIN_PATH)
    OSG_INCLUDE_DIRECTORY = $(OSG_INCLUDE_PATH)

    OPENAL_LIB_DIR = $$(OPENAL_BIN)
    OPENAL_INCLUDE_BIN = $$(OPENAL_INCLUDE)

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -L$$OSG_LIB_DIRECTORY -losg
        LIBS += -L$$OSG_LIB_DIRECTORY -losgViewer
        LIBS += -L$$OSG_LIB_DIRECTORY -losgDB
        LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreads
        LIBS += -L$$OSG_LIB_DIRECTORY -losgGA
        LIBS += -L$$OSG_LIB_DIRECTORY -losgUtil
        LIBS += -L$$OSG_LIB_DIRECTORY -losgText

        LIBS += -L../../../lib -lroute-loader_d
        LIBS += -L../../../lib -llibrary_d
        LIBS += -L../../../lib -lfilesystem_d
        LIBS += -L../../../lib -lTcpConnection_d
        LIBS += -L../../../lib -ldisplay_d

        LIBS += -L../../../lib -lsound-manager_d

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
        LIBS += -L../../../lib -ldisplay

        LIBS += -L../../../lib -lsound-manager
    }

    #LIBS += -lopengl32 -lglu32

    INCLUDEPATH += $$OSG_INCLUDE_DIRECTORY

    LIBS += -L$$OPENAL_LIB_DIR -lOpenAL32
    INCLUDEPATH += $$OPENAL_INCLUDE_BIN
}

unix {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -losg
        LIBS += -losgViewer
        LIBS += -losgDB
        LIBS += -lOpenThreads
        LIBS += -losgGA
        LIBS += -losgUtil
        LIBS += -losgText

        LIBS += -L../../../lib -lroute-loader_d
        LIBS += -L../../../lib -llibrary_d
        LIBS += -L../../../lib -lfilesystem_d
        LIBS += -L../../lib -lTcpConnection_d
        LIBS += -L../../lib -ldisplay_d

        LIBS += -L../../../lib -lsound-manager

    } else {

        LIBS +=  -losg
        LIBS +=  -losgViewer
        LIBS +=  -losgDB
        LIBS +=  -lOpenThreads
        LIBS +=  -losgGA
        LIBS +=  -losgUtil
        LIBS += -losgText

        LIBS += -L../../../lib -lroute-loader
        LIBS += -L../../../lib -llibrary
        LIBS += -L../../../lib -lfilesystem
        LIBS += -L../../lib -lTcpConnection
        LIBS += -L../../lib -ldisplay

        LIBS += -L../../../lib -lsound-manager
    }

    LIBS += -lopenal
    INCLUDEPATH += /usr/include/AL

    #LIBS += -lGL
}

#QMAKE_CXXFLAGS += -pg
#QMAKE_LFLAGS += -pg
#QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
#LIBS += -lgcov

#CONFIG += force_debug_info

#MAKE_CXXFLAGS_DEBUG += -pg
#QMAKE_LFLAGS_DEBUG += -pg

INCLUDEPATH += ../../common-headers
INCLUDEPATH += ../../simulator/vehicle/include
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../tcp-connection/include
INCLUDEPATH += ../route-loader/include
INCLUDEPATH += ../library/include
INCLUDEPATH += ../display/include
INCLUDEPATH += ./include

INCLUDEPATH += ../sound-manager/include
INCLUDEPATH += ../../asound/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
