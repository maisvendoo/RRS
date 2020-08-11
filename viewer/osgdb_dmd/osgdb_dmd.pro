TEMPLATE = lib

CONFIG -= qt

CONFIG += plugin
CONFIG += no_plugin_name_prefix

TARGET = osgdb_dmd

win32-g++: TARGET = $$join(TARGET,,mingw_,)

win32 {

    OSG_LIB_DIRECTORY = $$(OSG_BIN_PATH)
    OSG_INCLUDE_DIRECTORY = $$(OSG_INCLUDE_PATH)
    DESTDIR = $$(OSG_PLUGINS_PATH)

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,d)

        LIBS += -L$$OSG_LIB_DIRECTORY -losgd
        LIBS += -L$$OSG_LIB_DIRECTORY -losgViewerd
        LIBS += -L$$OSG_LIB_DIRECTORY -losgDBd
        LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreadsd
        LIBS += -L$$OSG_LIB_DIRECTORY -losgUtild

    } else {

        LIBS += -L$$OSG_LIB_DIRECTORY -losg
        LIBS += -L$$OSG_LIB_DIRECTORY -losgViewer
        LIBS += -L$$OSG_LIB_DIRECTORY -losgDB
        LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreads
        LIBS += -L$$OSG_LIB_DIRECTORY -losgUtil

    }

    INCLUDEPATH += $$OSG_INCLUDE_DIRECTORY
}

unix {

    DESTDIR = /usr/lib/osgPlugins-3.7.0

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,d)

        LIBS += -losgd
        LIBS += -losgViewerd
        LIBS += -losgDBd
        LIBS += -lOpenThreadsd
        LIBS += -losgUtild

    } else {

        LIBS +=  -losg
        LIBS +=  -losgViewer
        LIBS +=  -losgDB
        LIBS +=  -lOpenThreads
        LIBS += -losgUtil
    }
}

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
