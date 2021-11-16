TEMPLATE = app

QT -= gui
QT += core
QT += xml
QT += network

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

DESTDIR += ../../../bin

TARGET = simulator

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -lJournal_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -lvehicle_d
    LIBS += -L../../../lib -lcoupling_d
    LIBS += -L../../../lib -lsolver_d
    LIBS += -L../../../lib -ltrain_d
    LIBS += -L../../../lib -lbrakepipe_d
    LIBS += -L../../../lib -lmodel_d    
    LIBS += -L../../../lib -lTcpConnection_d
    LIBS += -L../../../lib -lsim-client_d
    LIBS += -L../../../lib -lprofile_d
    LIBS += -L../../../lib -ldevice_d

    LIBS += -L../../../lib -lasound_d
    LIBS += -L../../../lib -lsound-manager_d

    LIBS += -L../../../lib -lsignaling_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -lJournal
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -lvehicle
    LIBS += -L../../../lib -lcoupling
    LIBS += -L../../../lib -lsolver
    LIBS += -L../../../lib -ltrain
    LIBS += -L../../../lib -lbrakepipe
    LIBS += -L../../../lib -lmodel    
    LIBS += -L../../../lib -lTcpConnection
    LIBS += -L../../../lib -lsim-client
    LIBS += -L../../../lib -lprofile
    LIBS += -L../../../lib -ldevice

    LIBS += -L../../../lib -lasound
    LIBS += -L../../../lib -lsound-manager

    LIBS += -L../../../lib -lsignaling
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

INCLUDEPATH += ../../common-headers/
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../libJournal/include
INCLUDEPATH += ../../tcp-connection/include

INCLUDEPATH += ../physics/include
INCLUDEPATH += ../model/include
INCLUDEPATH += ../train/include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../vehicle/include
INCLUDEPATH += ../coupling/include
INCLUDEPATH += ../brakepipe/include
INCLUDEPATH += ../profile/include
INCLUDEPATH += ../device/include
INCLUDEPATH += ../sim-client/include

INCLUDEPATH += ../../asound/include
INCLUDEPATH += ../sound-manager/include

INCLUDEPATH += ../signaling/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
