#ifndef SUN_H
#define SUN_H

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

private:
    void ecef_to_latlong(
        double x, double y, double z,
        double& latitude, double& longitude, double& elevation
    );

private:
    const vsg::dvec3& camera_pos;
};

#endif // SUN_H
