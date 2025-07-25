#include    <ProcLightAnimation.h>
#include    <CfgReader.h>

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

    return true;
}
