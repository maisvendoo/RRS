TEMPLATE = subdirs

CONFIG += ordered

CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS -= O1
    QMAKE_CXXFLAGS -= O2
    QMAKE_CXXFLAGS += O4
}


SUBDIRS += ./CfgReader
SUBDIRS += ./filesystem
SUBDIRS += ./log
#SUBDIRS += ./tcp
SUBDIRS += ./tcp-connection
SUBDIRS += ./simulator

SUBDIRS += ./addons/default-vehicle
SUBDIRS += ./addons/test-loco
SUBDIRS += ./addons/sapsan-motor
SUBDIRS += ./addons/sapsan-non-motor

SUBDIRS += ./launcher

SUBDIRS += ./viewer

SUBDIRS += ./tools/profconv
