TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += ./sound-manager \
    udp-server
SUBDIRS += ./physics
SUBDIRS += ./device
SUBDIRS += ./solver
SUBDIRS += ./rkf5
SUBDIRS += ./rk4
SUBDIRS += ./vehicle
SUBDIRS += ./coupling
SUBDIRS += ./brakepipe
SUBDIRS += ./profile
SUBDIRS += ./train
SUBDIRS += ./sim-client
SUBDIRS += ./model
SUBDIRS += ./default-coupling
SUBDIRS += ./ef-coupling
SUBDIRS += ./simulator

SUBDIRS += ./modbus

SUBDIRS += ./krm395
SUBDIRS += ./krm130
SUBDIRS += ./vr242
SUBDIRS += ./kvt224
SUBDIRS += ./kvt254
SUBDIRS += ./epk150
SUBDIRS += ./evr305
SUBDIRS += ./carbrakes-mech

