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

    void update(simulator_time_t time, double timezone,
                double latitude, double longitude);

    bool use_gui_sun_direction = false;

    bool use_gui_ambient_intensity = false;

    bool use_gui_sun_intensity = false;

    /// Тени от облаков (ТЗ "Частицы", пресеты High/Ultra/Extreme):
    /// плавная модуляция интенсивности ambient лёгким детерминированным
    /// шумом (сумма синусов нескольких частот, периоды 30-60 с).
    /// Legacy/Low/Custom не включают — ambient у них не трогается.
    /// При ручной настройке ambient из GUI модуляция не применяется
    bool cloud_shadows = false;

    float azimuth_deg = 0.0;
    float altitude_deg = 0.0;

    vsg::ref_ptr<vsg::DirectionalLight> sun = vsg::DirectionalLight::create();
    vsg::ref_ptr<vsg::AmbientLight> ambient = vsg::AmbientLight::create();

private:
    void update_sun_direction_degrees(simulator_time_t time, double timezone,
                                      double latitude, double longitude);

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
