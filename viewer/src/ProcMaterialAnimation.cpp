#include "ProcMaterialAnimation.h"

#include "CfgReader.h"

#include <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcMaterialAnimation::ProcMaterialAnimation(vsg::ref_ptr<vsg::PbrMaterialValue> in_material_data)
    : Inherit()
    , material_value(in_material_data)
    , base_color(in_material_data->value().baseColorFactor)
    , emission_color(in_material_data->value().emissiveFactor)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcMaterialAnimation::update(float current_signal)
{
    if (!keypoints.empty())
    {
        float color_factor = interpolate(current_signal);
        vsg::vec4 new_color = base_color * color_factor;
        new_color.a = 1.0f;
        material_value->value().baseColorFactor = new_color;
    }

    vsg::vec4 new_emission_color = emission_color * current_signal;
    new_emission_color.a = 1.0f;
    material_value->value().emissiveFactor = new_emission_color;
    material_value->dirty();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcMaterialAnimation::load_config(CfgReader &cfg)
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
        cur_signal = static_cast<float>(tmp_dbl);
        is_fixed_signal = true;
    }

    QString tmp_qstr = "1.0 1.0 1.0";
    if (cfg.getString(sec_name, "EmissionColor", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> emission_color.r >> emission_color.g >> emission_color.b;
    }

    tmp_qstr = "1.0 1.0 1.0";
    if (cfg.getString(sec_name, "Color", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        vsg::vec4 config_color_limit = {1.0f, 1.0f, 1.0f, 1.0f};
        ss >> config_color_limit.r >> config_color_limit.g >> config_color_limit.b;
        base_color *= config_color_limit;
    }

    material_value->properties.dataVariance = vsg::DYNAMIC_DATA;
    update(cur_signal);
    return true;
}
