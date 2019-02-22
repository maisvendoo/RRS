TEMPLATE = lib

QT -= gui
QT += core
QT +=  xml

DEFINES += SOUND_MANAGER_LIB

TARGET = sound-manager

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../lib -lasound_d
    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lfilesystem_d

} else {

    LIBS += -L../../../lib -lasound
    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lfilesystem

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
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
