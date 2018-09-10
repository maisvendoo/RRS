TEMPLATE = lib

QT -= gui

TARGET = zds-profile-loader

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    LIBS += -L../../../lib -lprofile_d

} else {

    LIBS += -L../../../lib -lprofile
}

INCLUDEPATH += ./include
INCLUDEPATH += ../profile/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
