TEMPLATE = lib

QT += core
QT += gui
QT += widgets
QT += opengl

TARGET = blok-display

DESTDIR = ../../../../modules/ep20

CONFIG(debug, debug|release) {

    LIBS += -L../../../../lib -ldisplay_d


} else {

    LIBS += -L../../../../lib -ldisplay
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../../common-headers/
INCLUDEPATH += ../../../viewer/display/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
