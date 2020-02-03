TEMPLATE = lib

QT += core
QT += gui
QT += widgets
QT += opengl
QT += xml

TARGET = blok-display

DESTDIR = ../../../../modules/ep20

CONFIG(debug, debug|release) {

    LIBS += -L../../../../lib -ldisplay_d
    LIBS += -L../../../../lib -lCfgReader_d

} else {

    LIBS += -L../../../../lib -ldisplay
    LIBS += -L../../../../lib -lCfgReader
}

INCLUDEPATH += ./include
INCLUDEPATH += ../ep20/include
INCLUDEPATH += ../../../common-headers/
INCLUDEPATH += ../../../simulator/vehicle/include
INCLUDEPATH += ../../../CfgReader/include
INCLUDEPATH += ../../../viewer/display/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
RESOURCES += $$files(./resources/*.qrc)
