TEMPLATE = app

QT += core
QT += gui
QT += widgets

CONFIG += c++11

TARGET = routeLODgen

DESTDIR = ../../../bin

win32 {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -L$$(OSG_BIN_PATH) -losg
        LIBS += -L$$(OSG_BIN_PATH) -losgViewer
        LIBS += -L$$(OSG_BIN_PATH) -losgDB
        LIBS += -L$$(OSG_BIN_PATH) -lOpenThreads
        LIBS += -L$$(OSG_BIN_PATH) -losgUtil

    } else {

        LIBS += -L$$(OSG_BIN_PATH) -losg
        LIBS += -L$$(OSG_BIN_PATH) -losgViewer
        LIBS += -L$$(OSG_BIN_PATH) -losgDB
        LIBS += -L$$(OSG_BIN_PATH) -lOpenThreads
        LIBS += -L$$(OSG_BIN_PATH) -losgUtil

    }

    INCLUDEPATH += $$(OSG_INCLUDE_PATH)
}

unix {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)

        LIBS += -losg
        LIBS += -losgViewer
        LIBS += -losgDB
        LIBS += -lOpenThreads
        LIBS += -losgUtil

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
FORMS += $$files(./forms/*.ui)
TRANSLATIONS += $$files(./translations/*.ts)
