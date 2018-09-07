TEMPLATE = lib

QT -= gui

TARGET = verlet

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    LIBS += -L../../../lib -lsolver_d

} else {

    LIBS += -L../../../lib -lsolver
}

INCLUDEPATH += ./include
INCLUDEPATH += ../solver/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)

