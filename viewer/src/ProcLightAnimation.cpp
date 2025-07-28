#include    <ProcLightAnimation.h>
#include    <CfgReader.h>

#include    <vsg/lighting/SpotLight.h>
#include    <vsg/lighting/DirectionalLight.h>
#include    <vsg/lighting/PointLight.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcLightAnimation::ProcLightAnimation(vsg::ref_ptr<vsg::Light> in_light)
    : Inherit()
    , light(in_light)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcLightAnimation::update(float current_signal)
{
    if (!keypoints.empty())
    {
        double level = interpolate(current_signal);
        light->intensity = level * max_intensity;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcLightAnimation::load_config(CfgReader &cfg)
{
    load_common_settings(cfg);

    if (auto spotlight = light.cast<vsg::SpotLight>())
    {
        load_spotlight_settings(spotlight.get(), cfg);
    }

    if (auto pointLight = light.cast<vsg::PointLight>())
    {
        load_pointlight_settings(pointLight.get(), cfg);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcLightAnimation::load_common_settings(CfgReader &cfg)
{
    QString sec_name = "LightAnimation";

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
        signal_id = tmp_int;

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
        duration = tmp_dbl;

    tmp_dbl = 0.0;
    if (cfg.getDouble(sec_name, "FixedSignal", tmp_dbl))
    {
        cur_signal = static_cast<float>(tmp_dbl);
        is_fixed_signal = true;
    }

    QString tmp_qstr = "1.0 1.0 1.0";
    if (cfg.getString(sec_name, "Color", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        vsg::vec3 config_color_limit = {1.0f, 1.0f, 1.0f};
        ss >> config_color_limit.r >> config_color_limit.g >> config_color_limit.b;
        light_color *= config_color_limit;
    }

    tmp_dbl = 0.0;
    if (cfg.getDouble(sec_name, "MaxIntensity", tmp_dbl))
    {
        max_intensity = tmp_dbl;
    }

    light->color = light_color;
    light->intensity = cur_signal * max_intensity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcLightAnimation::load_spotlight_settings(vsg::SpotLight *sl,
                                                 CfgReader &cfg)
{
    QString sec_name = "LightAnimation";

    double innerAngle = vsg::degrees(sl->innerAngle);
    if (cfg.getDouble(sec_name, "InnerAngle", innerAngle))
    {
        sl->innerAngle = vsg::radians(innerAngle);
    }

    double outerAngle = vsg::degrees(sl->outerAngle);
    if (cfg.getDouble(sec_name, "OuterAngle", outerAngle))
    {
        sl->outerAngle = vsg::radians(outerAngle);
    }

    double radius = sl->radius;
    if (cfg.getDouble(sec_name, "Radius", radius))
    {
        sl->radius = radius;
    }

    vsg::dvec3 direction = sl->direction;

    double hrot = 0.0;
    double vrot = 0.0;

    cfg.getDouble(sec_name, "HRotation", hrot);
    cfg.getDouble(sec_name, "VRotation", vrot);

    hrot = vsg::radians(hrot);
    vrot = vsg::radians(vrot);

    direction.x = cos(vrot) * sin(hrot);
    direction.y = sin(vrot);
    direction.z = -cos(vrot) * cos(hrot);

    sl->direction = direction;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcLightAnimation::load_pointlight_settings(vsg::PointLight *pl,
                                                  CfgReader &cfg)
{
    QString sec_name = "LightAnimation";

    double radius = pl->radius;
    if (cfg.getDouble(sec_name, "Radius", radius))
    {
        pl->radius = radius;
    }
}
