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
    Sun(const vsg::dvec3& camera_pos, double ambient_intensity, double sun_intensity);

    void update(simulator_time_t time, double timezone);

    void getSunDirection(double& azimuth_degrees, double& altitude_degrees);

    bool use_gui_sun_direction = false;
    float azimuth_deg = 0.0;
    float altitude_deg = 0.0;

    bool use_gui_ambient_intensity = false;

    bool use_gui_sun_intensity = false;

    vsg::ref_ptr<vsg::DirectionalLight> sun = vsg::DirectionalLight::create();
    vsg::ref_ptr<vsg::AmbientLight>     ambient = vsg::AmbientLight::create();

private:
    void update_sun_direction_degrees(simulator_time_t time, double timezone);

    void ecef_to_latlong(
        double x, double y, double z,
        double& latitude, double& longitude, double& elevation
    );

    float calc_intensity(double altitude_deg, float max_intencity);

private:
    const vsg::dvec3& camera_pos;
    double ambient_max_intensity = 0.5;
    double sun_max_intensity = 5.0;
};

#endif // SUN_H
