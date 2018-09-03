TEMPLATE = lib

QT -= gui
QT += xml

DEFINES += SOLVER_LIB

TARGET = vehicle

DESTDIR = ../../../lib

CONFIG(debug, debug|release) {

    TARGET = $$join(TARGET,,,_d)
    LIBS += -L../../../lib -lCfgReader_d

} else {

    LIBS += -L../../../lib -lCfgReader
}

INCLUDEPATH += ./include
INCLUDEPATH += ../solver/include
INCLUDEPATH += ../../CfgReader/include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
