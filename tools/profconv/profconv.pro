TEMPLATE = app

QT += core

TARGET = profconv

DESTDIR = ../../../bin

INCLUDEPATH += ./include

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

} else {

}

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
