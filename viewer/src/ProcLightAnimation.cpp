#include    <ProcLightAnimation.h>
#include    <CfgReader.h>
#include    <sstream>

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
void ProcLightAnimation::anim_step(float t, float dt)
{
    if (is_fixed_signal)
    {
        return;
    }

    float server_signal = 0.0f;
    if (server_signals && (signal_id >= 0) && (static_cast<std::size_t>(signal_id) < server_signals->size()))
    {
        server_signal = (*server_signals)[signal_id];
    }
    const float delta = server_signal - cur_signal;

    server_signal = 0.0f;
    if (server_signals && (signal_id2 >= 0) && (static_cast<std::size_t>(signal_id2) < server_signals->size()))
    {
        server_signal = (*server_signals)[signal_id2];
    }
    const float delta2 = server_signal - cur_signal2;

    server_signal = 0.0f;
    if (server_signals && (signal_id3 >= 0) && (static_cast<std::size_t>(signal_id3) < server_signals->size()))
    {
        server_signal = (*server_signals)[signal_id3];
    }
    const float delta3 = server_signal - cur_signal3;

    if ((std::abs(delta) > 1e-5f) || (std::abs(delta2) > 1e-5f) || (std::abs(delta3) > 1e-5f))
    {
        const float delta_dt = std::min(duration * dt, 1.0f);
        cur_signal += delta * delta_dt;
        cur_signal2 += delta2 * delta_dt;
        cur_signal3 += delta3 * delta_dt;
        update(std::min(cur_signal + cur_signal2 + cur_signal3, 1.0f));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcLightAnimation::update(float current_signal)
{
    if (keypoints.empty())
    {
        light->intensity = current_signal * max_intensity;
    }
    else
    {
        light->intensity = interpolate(current_signal) * max_intensity;
    }

    auto sum_by_signal = [&](float& c1, float& c2, float& c3) -> double {
        return std::min(c1 * cur_signal + c2 * cur_signal2 + c3 * cur_signal3, 1.0f);
    };

    light->color.r = sum_by_signal(light_color.r, light_color2.r, light_color3.r);
    light->color.g = sum_by_signal(light_color.g, light_color2.g, light_color3.g);
    light->color.b = sum_by_signal(light_color.b, light_color2.b, light_color3.b);
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

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
        duration = tmp_dbl;

    tmp_dbl = 0.0;
    if (cfg.getDouble(sec_name, "FixedSignal", tmp_dbl))
    {
        cur_signal = static_cast<float>(tmp_dbl);
        is_fixed_signal = true;
    }

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
        signal_id = tmp_int;

    QString tmp_qstr = "1.0 1.0 1.0";
    if (cfg.getString(sec_name, "Color", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> light_color.r >> light_color.g >> light_color.b;
        light_color.r = std::clamp(light_color.r, 0.0f, 1.0f);
        light_color.g = std::clamp(light_color.g, 0.0f, 1.0f);
        light_color.b = std::clamp(light_color.b, 0.0f, 1.0f);
    }

    tmp_int = -1;
    tmp_qstr = "0.0 0.0 0.0";
    if (cfg.getInt(sec_name, "SignalID2", tmp_int) &&
        cfg.getString(sec_name, "Color2", tmp_qstr))
    {
        signal_id2 = tmp_int;

        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> light_color2.r >> light_color2.g >> light_color2.b;
        light_color2.r = std::clamp(light_color2.r, 0.0f, 1.0f);
        light_color2.g = std::clamp(light_color2.g, 0.0f, 1.0f);
        light_color2.b = std::clamp(light_color2.b, 0.0f, 1.0f);
    }

    tmp_int = -1;
    tmp_qstr = "0.0 0.0 0.0";
    if (cfg.getInt(sec_name, "SignalID3", tmp_int) &&
        cfg.getString(sec_name, "Color3", tmp_qstr))
    {
        signal_id3 = tmp_int;

        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> light_color3.r >> light_color3.g >> light_color3.b;
        light_color3.r = std::clamp(light_color3.r, 0.0f, 1.0f);
        light_color3.g = std::clamp(light_color3.g, 0.0f, 1.0f);
        light_color3.b = std::clamp(light_color3.b, 0.0f, 1.0f);
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
