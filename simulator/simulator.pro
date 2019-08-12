TEMPLATE = subdirs

CONFIG += ordered
CONFIG += force_debug_info

SUBDIRS += ./modbus
SUBDIRS += ./sound-manager
SUBDIRS += ./physics
SUBDIRS += ./device
SUBDIRS += ./solver
SUBDIRS += ./rkf5
SUBDIRS += ./vehicle
SUBDIRS += ./coupling
SUBDIRS += ./brakepipe
SUBDIRS += ./profile
SUBDIRS += ./train
SUBDIRS += ./model
SUBDIRS += ./default-coupling
SUBDIRS += ./ef-coupling
SUBDIRS += ./simulator

SUBDIRS += ./krm395
SUBDIRS += ./vr242
SUBDIRS += ./kvt254
SUBDIRS += ./epk150
SUBDIRS += ./km2202
SUBDIRS += ./carbrakes-mech

