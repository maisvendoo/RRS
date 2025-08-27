#include "Sun.h"

#include "spa.h"

#include <vsg/maths/common.h>
#include <vsg/maths/vec3.h>

#include <cmath>

static spa_data default_spa()
{
    spa_data spa;
    spa.delta_ut1 = 0.0;
    spa.delta_t = 0.0;
    spa.elevation = 0.0;
    spa.pressure = 0.0;
    spa.temperature = 0.0;
    spa.slope = 0.0;
    spa.azm_rotation = 0.0;
    spa.atmos_refract = 0.5667;
    spa.function = SPA_ZA;

    return spa;
}

void Sun::calculate_direction(
    int year, int month, int day,
    int hour, int minute, double second,
    double timezone,
    double latitude, double longitude
)
{
    static spa_data spa = default_spa();
    spa.year = year;
    spa.month = month;
    spa.day = day;
    spa.hour = hour;
    spa.minute = minute;
    spa.second = second;
    spa.timezone = timezone;
    spa.longitude = longitude;
    spa.latitude = latitude;

    spa_calculate(&spa);

    const double azimuth_rad = vsg::radians(spa.azimuth);
    const double altitude_rad = vsg::radians(spa.e);

    direction.x = -std::cos(altitude_rad) * std::sin(azimuth_rad);
    direction.y = -std::cos(altitude_rad) * std::cos(azimuth_rad);
    direction.z = -std::sin(altitude_rad);
    direction = vsg::normalize(direction);
}
