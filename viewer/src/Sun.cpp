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

static double rad_to_deg(double rad)
{
    return rad * 180.0 / M_PI;
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
    ecef_to_latlong(spa.latitude, spa.longitude, spa.elevation);

    spa_calculate(&spa);

    const double azimuth_rad = vsg::radians(spa.azimuth);
    const double altitude_rad = vsg::radians(spa.e);

    direction.x = -std::cos(altitude_rad) * std::sin(azimuth_rad);
    direction.y = -std::cos(altitude_rad) * std::cos(azimuth_rad);
    direction.z = -std::sin(altitude_rad);
    direction = vsg::normalize(direction);
}

void Sun::ecef_to_latlong(double& latitude, double& longitude, double& elevation)
{
    const double x = camera_pos.x;
    const double y = camera_pos.y;
    const double z = camera_pos.z;

    constexpr double a = 6'378'137.0;
    constexpr double b = 6'356'752.3142;
    constexpr double e_sq = (a * a - b * b) / (a * a);
    constexpr double e_sq2 = (a * a - b * b) / (b * b);

    const double p = std::sqrt(x * x + y * y);
    const double F = 54.0 * b * b * z * z;
    const double G = p * p + (1.0 - e_sq) * z * z - e_sq * (a * a - b * b);
    const double c = e_sq * e_sq * F * p * p / (G * G * G);
    const double s = std::cbrt(1.0 + c + std::sqrt(c * c + 2.0 * c));
    const double k = s + 1.0 + 1.0 / s;
    const double P = F / (3.0 * k * k * G * G);
    const double Q = std::sqrt(1.0 + 2.0 * e_sq * e_sq * P);
    const double r_0 = (-P * e_sq * p) / (1.0 + Q) + std::sqrt(0.5 * a * a * (1.0 + 1.0 / Q) - (P * (1.0 - e_sq) * z * z) / (Q * (1.0 + Q)) - 0.5 * P * p * p);
    const double U = std::sqrt(std::pow((p - e_sq * r_0), 2) + z * z);
    const double V = std::sqrt(std::pow((p - e_sq * r_0), 2) + (1.0 - e_sq) * z * z);
    const double z_0 = (b * b * z) / (a * V);

    double lat_rad = std::atan((z + e_sq2 * z_0) / p);
    double lon_rad = std::atan2(y, x);

    elevation = U * (1.0 - (b * b) / (a * V));
    latitude = rad_to_deg(lat_rad);
    longitude = rad_to_deg(lon_rad);
}
