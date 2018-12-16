QT += core
QT += gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = launcher
TEMPLATE = app

DESTDIR = ../../bin

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

INCLUDEPATH += ./include

SOURCES += $$files(./src/*.cpp)
HEADERS += $$files(./include/*.h)
FORMS += $$files(./forms/*.ui)
