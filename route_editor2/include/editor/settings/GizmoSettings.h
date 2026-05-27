#ifndef EDITOR_GIZMO_SETTINGS_H
#define EDITOR_GIZMO_SETTINGS_H

#include <vsg/maths/vec3.h>

class CfgReader;

struct gizmo_settings_t
{
    vsg::vec3 axis_x_color;
    vsg::vec3 axis_y_color;
    vsg::vec3 axis_z_color;
    float arrow_length;
    float arrow_thickness;
    float opacity;
    bool to_center;

    gizmo_settings_t();
    void read(CfgReader& cfg);
};

#endif // EDITOR_GIZMO_SETTINGS_H
