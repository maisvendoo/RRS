TEMPLATE = lib

QT -= gui

TARGET = euler2

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
