TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += ./CfgReader
SUBDIRS += ./filesystem
SUBDIRS += ./log
#SUBDIRS += ./tcp
SUBDIRS += ./tcp-connection
SUBDIRS += ./simulator

SUBDIRS += ./addons/default-vehicle
SUBDIRS += ./addons/test-loco

SUBDIRS += ./launcher

SUBDIRS += ./viewer

SUBDIRS += ./tools/profconv
