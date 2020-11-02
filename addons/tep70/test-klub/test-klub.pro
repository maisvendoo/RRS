TEMPLATE = app

QT += core
QT += gui
QT += widgets
QT += opengl
QT += xml

TARGET = test-klub

DESTDIR = ../../../../bin

CONFIG(debug, debug|release) {

    LIBS += -L../../../../lib -ldisplay_d
    LIBS += -L../../../../lib -lCfgReader_d

} else {

    LIBS += -L../../../../lib -ldisplay
    LIBS += -L../../../../lib -lCfgReader
}

INCLUDEPATH += ./include
INCLUDEPATH += ../tep70bs/include
INCLUDEPATH += ../../../simulator/vehicle/include
INCLUDEPATH += ../../../CfgReader/include
INCLUDEPATH += ../../../viewer/display/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
