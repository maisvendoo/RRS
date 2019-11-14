TEMPLATE = lib

QT -= gui
QT += xml
QT += network

DEFINES += MODEL_LIB

TARGET = model

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -lJournal_d
    LIBS += -L../../../lib -llog_d
    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -lvehicle_d
    LIBS += -L../../../lib -lcoupling_d
    LIBS += -L../../../lib -lsolver_d
    LIBS += -L../../../lib -ltrain_d
    LIBS += -L../../../lib -lbrakepipe_d
    LIBS += -L../../../lib -lTcpConnection_d
    LIBS += -L../../../lib -lprofile_d
    LIBS += -L../../../lib -ldevice_d

    LIBS += -L../../../lib -lasound_d
    LIBS += -L../../../lib -lsound-manager_d

} else {

    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -lJournal
    LIBS += -L../../../lib -llog
    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -lvehicle
    LIBS += -L../../../lib -lcoupling
    LIBS += -L../../../lib -lsolver
    LIBS += -L../../../lib -ltrain
    LIBS += -L../../../lib -lbrakepipe    
    LIBS += -L../../../lib -lTcpConnection
    LIBS += -L../../../lib -lprofile
    LIBS += -L../../../lib -ldevice

    LIBS += -L../../../lib -lasound
    LIBS += -L../../../lib -lsound-manager
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
INCLUDEPATH += ../../common-headers
INCLUDEPATH += ../physics/include
INCLUDEPATH += ../train/include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../vehicle/include
INCLUDEPATH += ../coupling/include
INCLUDEPATH += ../brakepipe/include
INCLUDEPATH += ../profile/include
INCLUDEPATH += ../device/include
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../libJournal/include
INCLUDEPATH += ../../log/include
INCLUDEPATH += ../../tcp-connection/include
INCLUDEPATH += ../../asound/include
INCLUDEPATH += ../sound-manager/include



HEADERS += $$files(./include/*.h)
HEADERS += $$files(../../common-headers/*.h)
SOURCES += $$files(./src/*.cpp)
