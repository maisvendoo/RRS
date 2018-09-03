TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += ./CfgReader
SUBDIRS += ./filesystem
SUBDIRS += ./log

SUBDIRS += ./simulator/physics
SUBDIRS += ./simulator/solver
SUBDIRS += ./simulator/vehicle
SUBDIRS += ./simulator/model
SUBDIRS += ./simulator/simulator
