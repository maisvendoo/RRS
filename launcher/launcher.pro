QT += core
QT += gui
QT += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = launcher
TEMPLATE = app

DESTDIR = ../../bin

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

win32 {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)        

        LIBS += -L../../lib -lfilesystem_d
        LIBS += -L../../lib -lCfgReader_d
        LIBS += -L../../lib -lCfgEditor_d

    } else {

        LIBS += -L../../lib -lfilesystem
        LIBS += -L../../lib -lCfgReader
        LIBS += -L../../lib -lCfgEditor
    }    
}

unix {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)        

        LIBS += -L../../lib -lfilesystem_d
        LIBS += -L../../lib -lCfgReader_d
        LIBS += -L../../lib -lCfgEditor_d

    } else {

        LIBS += -L../../lib -lfilesystem
        LIBS += -L../../lib -lCfgReader
        LIBS += -L../../lib -lCfgEditor
    }
}

INCLUDEPATH += ../common-headers
INCLUDEPATH += ../filesystem/include
INCLUDEPATH += ../CfgReader/include
INCLUDEPATH += ../CfgEditor/include
INCLUDEPATH += ./include

SOURCES += $$files(./src/*.cpp)
HEADERS += $$files(./include/*.h)
FORMS += $$files(./forms/*.ui)

TRANSLATIONS += ./translations/launcher.ru_RU.ts

RESOURCES += $$files(./resources/*.qrc)
