TEMPLATE = lib

QT -= gui

DEFINES += PROFILE_LIB

TARGET = profile

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -lJournal_d

} else {

    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -lJournal
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../common-headers
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../libJournal/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
