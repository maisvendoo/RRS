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
    LIBS += -L../../../lib -lmodel_d

} else {

    LIBS += -L../../../lib -lCfgReader
    LIBS += -L../../../lib -lfilesystem
    LIBS += -L../../../lib -llog
    LIBS += -L../../../lib -lmodel
}

INCLUDEPATH += ./include

INCLUDEPATH += ../common-headers/
INCLUDEPATH += ../../CfgReader/include
INCLUDEPATH += ../../filesystem/include
INCLUDEPATH += ../../log/include

INCLUDEPATH += ../model/include
INCLUDEPATH += ../train/include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../vehicle/include
INCLUDEPATH += ../coupling/include

HEADERS += $$files(./include/*.h) 
SOURCES += $$files(./src/*.cpp)
