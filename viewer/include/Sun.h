#ifndef SUN_H
#define SUN_H

#include <vsg/core/Inherit.h>
#include <vsg/lighting/DirectionalLight.h>

class Sun : public vsg::Inherit<vsg::DirectionalLight, Sun>
{
public:
    void calculate_direction(
        int year, int month, int day,
        int hour, int minute, double second,
        double timezone,
        double latitude, double longitude
    );
};

#endif // SUN_H
