#ifndef SUN_H
#define SUN_H

#include "datetime.h"

#include <vsg/core/Inherit.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/maths/vec3.h>

class Sun : public vsg::Inherit<vsg::DirectionalLight, Sun>
{
public:
    Sun(const vsg::dvec3& camera_pos);

    void update(
        int year, int month, int day,
        int hour, int minute, double second,
        double timezone
    );

    double get_azimuth_deg() const { return azimuth_deg; }
    double get_altitude_deg() const { return altitude_deg; }

    bool use_gui_intensity = false;
    float gui_intensity = 0.0f;

    bool use_gui_time = false;
    simulator_time_t gui_time;

private:
    void ecef_to_latlong(
        double x, double y, double z,
        double& latitude, double& longitude, double& elevation
    );

private:
    const vsg::dvec3& camera_pos;

    double azimuth_deg = 0.0;
    double altitude_deg = 0.0;
};

#endif // SUN_H
