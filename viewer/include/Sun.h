#ifndef SUN_H
#define SUN_H

#include <vsg/core/ref_ptr.h>

namespace vsg
{

class DirectionalLight;

}

class Sun
{
public:
    void calculate_direction(
        int year, int month, int day,
        int hour, int minute, double second,
        double timezone,
        double latitude, double longitude
    );

private:
    vsg::ref_ptr<vsg::DirectionalLight> light;
};

#endif // SUN_H
