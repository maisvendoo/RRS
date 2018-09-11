TEMPLATE = lib

QT -= gui

DEFINES += PHYSICS_LIB

TARGET = physics

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

} else {

}

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
