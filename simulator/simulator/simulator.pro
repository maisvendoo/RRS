TEMPLATE = app

QT -= gui
QT += core
QT += xml

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

DESTDIR += ../../../bin

TARGET = simulator

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)

    LIBS += -L../../../lib -lCfgReader_d
    LIBS += -L../../../lib -lfilesystem_d
    LIBS += -L../../../lib -llog_d
    LIBS += -L../../../lib -lphysics_d
    LIBS += -L../../../lib -lvehicle_d
    LIBS += -L../../../lib -lcoupling_d
    LIBS += -L../../../lib -lsolver_d
    LIBS += -L../../../lib -ltrain_d
    LIBS += -L../../../lib -lbrakepipe_d
    LIBS += -L../../../lib -lmodel_d
    LIBS += -L../../../lib -lprofile_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -llog
    LIBS += -L../../../lib -lphysics
    LIBS += -L../../../lib -lvehicle
    LIBS += -L../../../lib -lcoupling
    LIBS += -L../../../lib -lsolver
    LIBS += -L../../../lib -ltrain
    LIBS += -L../../../lib -lbrakepipe
    LIBS += -L../../../lib -lmodel
    LIBS += -L../../../lib -lprofile
}

INCLUDEPATH += ./include

INCLUDEPATH += ../common-headers/
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../log/include
INCLUDEPATH += ../../profile-loader/profile/include

INCLUDEPATH += ../physics/include
INCLUDEPATH += ../model/include
INCLUDEPATH += ../train/include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../vehicle/include
INCLUDEPATH += ../coupling/include
INCLUDEPATH += ../brakepipe/include

HEADERS += $$files(./include/*.h) 
SOURCES += $$files(./src/*.cpp)
