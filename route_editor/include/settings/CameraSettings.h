#ifndef EDITOR_CAMERA_SETTINGS_H
#define EDITOR_CAMERA_SETTINGS_H

class CfgReader;

struct camera_settings_t
{
    double zNear;
    double view_distance;
    double fovy;
    double fovy_min;
    double fovy_max;
    double initial_height;
    double move_speed;
    double rotate_speed;
    double zoom_power;

    camera_settings_t();
    void read(CfgReader& cfg);
};

#endif // EDITOR_CAMERA_SETTINGS_H
