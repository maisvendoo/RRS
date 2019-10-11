TEMPLATE = lib

QT -= gui

CONFIG += ordered
CONFIG += c++11

TARGET = Journal

DESTDIR = ../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

} else {

}

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)

win32 {

    HEADERS += ./include/wsyslog.h
    SOURCES += ./src/wsyslog.c

    LIBS += -lws2_32
}
