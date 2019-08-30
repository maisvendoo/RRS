TEMPLATE = lib

QT += opengl

TARGET = osgVRViewer

DESTDIR = ../../../lib

CONFIG += c++11

win32 {

    OSG_LIB_DIRECTORY = $$(OSG_BIN_PATH)
    OSG_INCLUDE_DIRECTORY = $$(OSG_INCLUDE_PATH)


    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -L$$OSG_LIB_DIRECTORY -losgd
        LIBS += -L$$OSG_LIB_DIRECTORY -losgGAd
        LIBS += -L$$OSG_LIB_DIRECTORY -losgViewerd
        LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreadsd

        LIBS += -L$$(OPENVR_LIB) -lopenvr_api

    } else {

        LIBS += -L$$OSG_LIB_DIRECTORY -losg
        LIBS += -L$$OSG_LIB_DIRECTORY -losgGA
        LIBS += -L$$OSG_LIB_DIRECTORY -losgViewer
        LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreads

        LIBS += -L$$(OPENVR_LIB) -lopenvr_api

    }

    LIBS += -lopengl32 -lglu32

    INCLUDEPATH += $$OSG_INCLUDE_DIRECTORY
    INCLUDEPATH += $$(OPENVR_INCLUDE)
}

unix {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -losgd
        LIBS += -losgViewerd
        LIBS += -lOpenThreadsd

        LIBS += -lopenvr_api

    } else {

        LIBS += -losg
        LIBS += -losgViewer
        LIBS += -lOpenThreads

        LIBS += -lopenvr_api
    }
}

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
