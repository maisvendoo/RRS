TEMPLATE = lib

QT -= qui
QT += xml

CONFIG += staticlib

TARGET = CfgReader

DESTDIR = ../../lib

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
