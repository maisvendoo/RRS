QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = routeconv
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

DESTDIR = ../../../bin

win32 {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -L../../../lib -lfilesystem_d
        LIBS += -L../../../lib -lCfgEditor_d

    } else {


        LIBS += -L../../../lib -lfilesystem
        LIBS += -L../../../lib -lCfgEditor
    }
}

unix {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -L../../../lib -lfilesystem_d
        LIBS += -L../../../lib -lCfgEditor_d

    } else {

        LIBS += -L../../../lib -lfilesystem
        LIBS += -L../../../lib -lCfgEditor
    }
}

INCLUDEPATH += ../../common-headers
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../CfgEditor/include
INCLUDEPATH += ./include

SOURCES += $$files(./src/*.cpp)
HEADERS += $$files(./include/*.h)
FORMS += $$files(./forms/*.ui)

TRANSLATIONS += ./translations/routeconv.ru_RU.ts

RESOURCES += $$files(./resources/*.qrc)

