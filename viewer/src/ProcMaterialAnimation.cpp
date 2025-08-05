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
void ProcMaterialAnimation::anim_step([[maybe_unused]] float t, float dt)
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
        update(cur_signal);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcMaterialAnimation::update([[maybe_unused]] float current_signal)
{
    if (!keypoints.empty())
    {
        material_value->value().baseColorFactor.r = base_color.r * interpolate(cur_signal);
        material_value->value().baseColorFactor.g = base_color.g * interpolate(cur_signal2);
        material_value->value().baseColorFactor.b = base_color.b * interpolate(cur_signal3);
    }

    material_value->value().emissiveFactor.r = emission_color.r * cur_signal;
    material_value->value().emissiveFactor.g = emission_color.g * cur_signal2;
    material_value->value().emissiveFactor.b = emission_color.b * cur_signal3;
    material_value->dirty();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcMaterialAnimation::load_config(CfgReader &cfg)
{
    QString sec_name = "MaterialAnimation";

    int tmp_int = -1;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
        signal_id = tmp_int;

    tmp_int = -1;
    if (cfg.getInt(sec_name, "SignalID2", tmp_int))
        signal_id2 = tmp_int;
    else
        signal_id2 = signal_id;

    tmp_int = -1;
    if (cfg.getInt(sec_name, "SignalID3", tmp_int))
        signal_id3 = tmp_int;
    else
        signal_id3 = signal_id;

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
        duration = tmp_dbl;

    tmp_dbl = 0.0;
    if (cfg.getDouble(sec_name, "FixedSignal", tmp_dbl))
    {
        cur_signal = static_cast<float>(tmp_dbl);
        cur_signal2 = static_cast<float>(tmp_dbl);
        cur_signal3 = static_cast<float>(tmp_dbl);
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

    emission_color.a = 1.0f;
    base_color.a = 1.0f;
    material_value->properties.dataVariance = vsg::DYNAMIC_DATA;
    update(cur_signal);
    return true;
}
