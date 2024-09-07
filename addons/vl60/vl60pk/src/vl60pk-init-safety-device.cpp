#include    "vl60pk.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initSafetyDevices(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;

    speedmap_fwd = new SpeedMap();
    speedmap_fwd->setDirection(dir * orient);
    addRailwayConnector(speedmap_fwd, length / 2.0);

    speedmap_bwd = new SpeedMap();
    speedmap_bwd->setDirection(-1 * dir * orient);
    addRailwayConnector(speedmap_bwd, -length / 2.0);

    speed_meter = new SL2M();
    speed_meter->setWheelDiameter(wheel_diameter[0]);
    speed_meter->read_config("3SL-2M", custom_cfg_dir);
}
