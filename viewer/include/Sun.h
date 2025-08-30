#ifndef SUN_H
#define SUN_H

#include "datetime.h"

#include <vsg/core/Inherit.h>
#include <vsg/nodes/Group.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/maths/vec3.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Sun : public vsg::Inherit<vsg::Group, Sun>
{
public:
    Sun(const vsg::dvec3& camera_pos);

    void update(
        int year, int month, int day,
        int hour, int minute, double second,
        double timezone
    );

    bool use_gui_sun_direction = false;
    float azimuth_deg = 0.0;
    float altitude_deg = 0.0;

    bool use_gui_sun_intensity = false;

    bool use_gui_ambient_intensity = false;

    bool use_gui_time = false;
    simulator_time_t gui_time;

    vsg::ref_ptr<vsg::DirectionalLight> sun = vsg::DirectionalLight::create();
    vsg::ref_ptr<vsg::AmbientLight>     ambient = vsg::AmbientLight::create();

private:
    void update_sun_direction_degrees(
        int year, int month, int day,
        int hour, int minute, double second,
        double timezone
    );

    void ecef_to_latlong(
        double x, double y, double z,
        double& latitude, double& longitude, double& elevation
    );

    float calc_intensity(double altitude_deg, float max_intencity);

private:
    const vsg::dvec3& camera_pos;
};

#endif // SUN_H
