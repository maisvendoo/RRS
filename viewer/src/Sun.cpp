#include "Sun.h"

#include "spa.h"

#include <vsg/maths/common.h>
#include <vsg/maths/vec3.h>

#include <cmath>

using Meters = double;

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Sun::Sun(const vsg::dvec3& camera_pos, double ambient_intensity, double sun_intensity)
    : camera_pos(camera_pos)
    , ambient_max_intensity(ambient_intensity)
    , sun_max_intensity(sun_intensity)
{
    addChild(sun);
    addChild(ambient);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Sun::update(
    int year, int month, int day,
    int hour, int minute, double second,
    double timezone
)
{
    if (!use_gui_ambient_intensity)
    {
        // Для утренних/вечерних сумерек условно поднимаем солнце из-под горизонта на 15 градусов
        constexpr double deg_under_horizont = 15.0;
        constexpr double altitude_coeff = (90.0 - deg_under_horizont) / 90.0;
        const double ambient_altitude_deg = deg_under_horizont + altitude_coeff * altitude_deg;

        ambient->intensity = std::fmax(0.01f, calc_intensity(ambient_altitude_deg, ambient_max_intensity));
    }

    if (!use_gui_sun_intensity)
    {
        sun->intensity = calc_intensity(altitude_deg, sun_max_intensity);
    }

    if (!use_gui_sun_direction)
    {
        update_sun_direction_degrees(year, month, day, hour, minute, second, timezone);
    }

    const double azimuth_rad = vsg::radians(azimuth_deg);
    const double altitude_rad = vsg::radians(altitude_deg);

    sun->direction.x = -std::cos(altitude_rad) * std::sin(azimuth_rad);
    sun->direction.y = -std::cos(altitude_rad) * std::cos(azimuth_rad);
    sun->direction.z = -std::sin(altitude_rad);
    sun->direction = vsg::normalize(sun->direction);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Sun::update_sun_direction_degrees(int year, int month, int day, int hour, int minute, double second, double timezone)
{
    constexpr Meters ecef_x0 = 2'849'494.463'270'107;
    constexpr Meters ecef_y0 = 2'196'239.724'320'043;
    constexpr Meters ecef_z0 = 5'248'968.407'733'058;

    static spa_data spa = default_spa();
    if (use_gui_time)
    {
        spa.year = gui_time.date.year();
        spa.month = gui_time.date.month();
        spa.day = gui_time.date.day();
        spa.hour = gui_time.time.hour();
        spa.minute = gui_time.time.minute();
        spa.second = gui_time.time.sec() + gui_time.time.msec();
    }
    else
    {
        spa.year = year;
        spa.month = month;
        spa.day = day;
        spa.hour = hour;
        spa.minute = minute;
        spa.second = second;
    }
    spa.timezone = timezone;
    ecef_to_latlong(camera_pos.x + ecef_x0, camera_pos.y + ecef_y0, camera_pos.z + ecef_z0, spa.latitude, spa.longitude, spa.elevation);

    spa_calculate(&spa);

    azimuth_deg = spa.azimuth;
    altitude_deg = spa.e;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Sun::ecef_to_latlong(
    double x, double y, double z,
    double& latitude, double& longitude, double& elevation
)
{
    constexpr Meters a = 6'378'137.0;
    constexpr Meters b = 6'356'752.3142;
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Sun::calc_intensity(double altitude_deg, float max_intencity)
{
    if (altitude_deg <= 0.0)
    {
        return 0.0;
    }

    // Радиус Земли, высота атмосферы
    constexpr Meters R_e = 6'378'137.0;
    constexpr Meters y_atm = 9000.0;
    constexpr double r = R_e / y_atm;

    // Расчёт относительного увелечения толщины атмосферы
    const double z = vsg::radians(altitude_deg);
    const double AM = std::sqrt(std::pow((r * std::sin(z)), 2) + 2.0 * r + 1.0) - r * std::sin(z);

    // Расчёт относительного ослабления солнечного света
    constexpr double empiric_exponent_base = 0.7;
    constexpr double empiric_exponent_power = 0.678;
    constexpr double intensity_normalize = 1.0 / empiric_exponent_base;
    const double intensity_coeff = intensity_normalize * std::pow(empiric_exponent_base, std::pow(AM, empiric_exponent_power));

    return max_intencity * intensity_coeff;
}
