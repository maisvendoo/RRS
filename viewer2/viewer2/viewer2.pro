TEMPLATE = app

CONFIG += qt
CONFIG += c++14

QT += core
QT += gui
QT += widgets
QT += network
QT += opengl
QT += xml

TARGET = viewer2

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
        LIBS += -L$$OSG_LIB_DIRECTORY -losgQt5d

        LIBS += -L../../../lib -lCfgReader_d

    } else {

        LIBS += -L$$OSG_LIB_DIRECTORY -losg
        LIBS += -L$$OSG_LIB_DIRECTORY -losgViewer
        LIBS += -L$$OSG_LIB_DIRECTORY -losgDB
        LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreads
        LIBS += -L$$OSG_LIB_DIRECTORY -losgGA
        LIBS += -L$$OSG_LIB_DIRECTORY -losgUtil
        LIBS += -L$$OSG_LIB_DIRECTORY -losgText
        LIBS += -L$$OSG_LIB_DIRECTORY -losgQt5        

        LIBS += -L../../../lib -lCfgReader
    }

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
        LIBS += -losgQt5d

        LIBS += -L../../../lib -lCfgReader_d

    } else {

        LIBS += -losg
        LIBS += -losgViewer
        LIBS += -losgDB
        LIBS += -lOpenThreads
        LIBS += -losgGA
        LIBS += -losgUtil
        LIBS += -losgText
        LIBS += -losgQt5

        LIBS += -L../../../lib -lCfgReader
    }
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../CfgReader/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)

