TEMPLATE = lib

CONFIG += c++11

CONFIG -= qt

CONFIG += plugin

CONFIG += no_plugin_name_prefix

TARGET = osgdb_dmd

win32-g++: TARGET = $$join(TARGET,,mingw_,)


win32 {

    OSG_LIB_DIRECTORY = $$(OSG_BIN_PATH)
    OSG_INCLUDE_DIRECTORY = $$(OSG_INCLUDE_PATH)
    DESTDIR = $$(OSG_PLUGINS_PATH)
}

unix {

    LIB_DIRECTORY = /opt/osg/lib64
    OSG_INCLUDE_DIRECTORY = /opt/osg/include
    DESTDIR = /opt/osg/lib64/osgPlugins-3.7.0
}

CONFIG(debug, debug|release) {    

    LIBS += -L$$OSG_LIB_DIRECTORY -losgd
    LIBS += -L$$OSG_LIB_DIRECTORY -losgDBd
    LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreadsd

    TARGET = $$join(TARGET,,,d)

} else {

    LIBS += -L$$OSG_LIB_DIRECTORY -losg
    LIBS += -L$$OSG_LIB_DIRECTORY -losgDB
    LIBS += -L$$OSG_LIB_DIRECTORY -lOpenThreads
}

INCLUDEPATH += ./include
INCLUDEPATH += $$OSG_INCLUDE_DIRECTORY

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
