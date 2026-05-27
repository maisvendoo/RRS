#include "editor/settings/CameraSettings.h"

#include <CfgReader.h>

#include <QString>

camera_settings_t::camera_settings_t()
    : zNear(0.1)
    , view_distance(2000.0)
    , fovy(60.0)
    , fovy_min(2.0)
    , fovy_max(100.0)
    , initial_height(5.0)
    , move_speed(100.0)
    , rotate_speed(0.3)
    , zoom_power(25.0)
{
}

void camera_settings_t::read(CfgReader& cfg)
{
    const QString section = "Camera";

    cfg.getDouble(section, "zNear", zNear);
    cfg.getDouble(section, "ViewDistance", view_distance);
    cfg.getDouble(section, "FovY", fovy);
    cfg.getDouble(section, "FovYMin", fovy_min);
    cfg.getDouble(section, "FovYMax", fovy_max);
    cfg.getDouble(section, "InitialHeight", initial_height);
    cfg.getDouble(section, "MoveSpeed", move_speed);
    cfg.getDouble(section, "RotateSpeed", rotate_speed);
    cfg.getDouble(section, "ZoomPower", zoom_power);
}
