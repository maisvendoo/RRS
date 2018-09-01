TEMPLATE = lib

QT -= qui
QT += xml

TARGET = CfgReader

DESTDIR = ../../lib

INCLUDEPATH += ./include

HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
