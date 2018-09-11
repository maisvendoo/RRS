TEMPLATE = lib

QT -= gui

DEFINES += FILESYSTEM_LIB

TARGET = filesystem

DESTDIR = ../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

} else {

}

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
