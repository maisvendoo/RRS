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
    spa.pressure = 0.0;
    spa.temperature = 0.0;
    spa.slope = 0.0;
    spa.azm_rotation = 0.0;
    spa.atmos_refract = 0.5667;
    spa.function = SPA_ZA;

    return spa;
}

Sun::Sun(const vsg::dvec3& camera_pos)
    : camera_pos(camera_pos)
{
}

void Sun::calculate_direction(
    int year, int month, int day,
    int hour, int minute, double second,
    double timezone
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
    decart_to_geo(spa.latitude, spa.longitude, spa.elevation);

    spa_calculate(&spa);

    const double azimuth_rad = vsg::radians(spa.azimuth);
    const double altitude_rad = vsg::radians(spa.e);

    direction.x = -std::cos(altitude_rad) * std::sin(azimuth_rad);
    direction.y = -std::cos(altitude_rad) * std::cos(azimuth_rad);
    direction.z = -std::sin(altitude_rad);
    direction = vsg::normalize(direction);
}

void Sun::decart_to_geo(double& latitude, double& longitude, double& elevation)
{
    // const double x = camera_pos.x;
    // const double y = camera_pos.y;
    // const double z = camera_pos.z;

    // /// Большая полуось (экваториальный радиус) (в метрах)
    // constexpr double a = 6'378'137.0;

    // /// Малая полуось (полярный радиус) (в метрах)
    // constexpr double b = 6'356'752.3142;

    // /// Сжатие эллипсоида
    // constexpr double f = (a - b) / a;

    // /// Первый эксцентриситет
    // constexpr double e_sq = 2.0 * f - f * f;

    // longitude = std::atan2(y, x);

    // /// Расстояние от точки до оси Z
    // const double p = std::sqrt(x * x + y * z);

    // double latitude_prev = std::atan2(z, (p * (1.0 - e_sq)));
    // latitude = latitude_prev + 1.0;

    // while (std::abs(latitude - latitude_prev) > 1.0e-12)
    // {
    //     /// Радиус кривизны первого вертикала
    //     const double N_i = a / std::sqrt(1.0 - e_sq * std::pow(std::sin(latitude_prev), 2));

    //     elevation = p / std::cos(latitude_prev) - N_i;

    //     latitude = std::atan((z / p) / (1.0 - e_sq * (N_i / (N_i + elevation))));
    // }
}
