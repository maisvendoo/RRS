#include "Sun.h"

#include "spa.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/lighting/DirectionalLight.h>

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
}
