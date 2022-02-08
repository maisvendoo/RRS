TEMPLATE = lib

DEFINES += CFGEDITOR_LIB

QT -= qui
QT += xml

CONFIG(debug, debug|release) {

    TARGET = CfgEditor_d
    DESTDIR = ../../lib

} else {

    TARGET = CfgEditor
    DESTDIR = ../../lib

}

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
