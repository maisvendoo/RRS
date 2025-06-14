#include "MaterialAnimation.h"

#include "CfgReader.h"

#include <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MaterialAnimation::MaterialAnimation(vsg::ref_ptr<vsg::PbrMaterialValue> data)
    : Inherit()
    , material_value(data)
    , color(data->value().baseColorFactor)
    , emission_color(0.0f, 0.0f, 0.0f, 1.0f)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MaterialAnimation::anim_step([[maybe_unused]] float t, float dt)
{
    float delta = (pos - cur_pos);
    if (abs(delta) > 1e-5f)
    {
        cur_pos += delta * fmin(duration * dt, 1.0f);
        update();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MaterialAnimation::load_config(CfgReader &cfg)
{
    QString sec_name = "MaterialAnimation";

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
        signal_id = tmp_int;

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
        duration = tmp_dbl;

    tmp_dbl = 0.0;
    if (cfg.getDouble(sec_name, "FixedSignal", tmp_dbl))
    {
        fixed_signal = static_cast<float>(tmp_dbl);
        is_fixed_signal = true;
    }

    QString tmp_qstr = "0.0 0.0 0.0";
    if (cfg.getString(sec_name, "EmissionColor", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> emission_color.x >> emission_color.y >> emission_color.z;
    }

    tmp_qstr = "1.0 1.0 1.0";
    if (cfg.getString(sec_name, "Color", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> color.x >> color.y >> color.z;
    }

    // material_value->value().baseColorFactor = color;
    // material_value->value().emissiveFactor = emission_color;

    update();
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MaterialAnimation::update()
{
    if (keypoints.empty())
    {
        return;
    }

    vsg::vec4 new_color = color * interpolate(cur_pos);
    new_color.a = 1.0f;

    vsg::vec4 new_emission_color = emission_color * cur_pos;
    new_emission_color.a = 1.0f;

    material_value->value().baseColorFactor = new_color;
    material_value->value().emissiveFactor = new_emission_color;    
    // material_value->value().diffuseFactor = new_color;
    material_value->dirty();
}
