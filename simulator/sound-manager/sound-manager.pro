TEMPLATE = lib

QT -= gui

DEFINES += SOUND_MANAGER_LIB

TARGET = sound-manager

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../lib -lasound_d

} else {

    LIBS += -L../../../lib -lasound

}

win32{

    OPENAL_LIB_DIR = $$(OPENAL_BIN)
    OPENAL_INCLUDE_BIN = $$(OPENAL_INCLUDE)

    LIBS += -L$$OPENAL_LIB_DIR -lOpenAL32
    INCLUDEPATH += $$OPENAL_INCLUDE_BIN
}

unix{

    LIBS += -lopenal
}

INCLUDEPATH += ./include
INCLUDEPATH += ../../asound/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
