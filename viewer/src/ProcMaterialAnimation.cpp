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
        update(std::min(cur_signal + cur_signal2 + cur_signal3, 1.0f));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcMaterialAnimation::update([[maybe_unused]] float current_signal)
{
    if (!keypoints.empty())
    {
        material_value->value().baseColorFactor.r = base_color.r * interpolate(current_signal);
        material_value->value().baseColorFactor.g = base_color.g * interpolate(current_signal);
        material_value->value().baseColorFactor.b = base_color.b * interpolate(current_signal);
    }

    auto sum_by_signal = [&](float& c1, float& c2, float& c3) -> double {
        return std::min(c1 * cur_signal + c2 * cur_signal2 + c3 * cur_signal3, 1.0f);
    };

    material_value->value().emissiveFactor.r =
        sum_by_signal(emission_color.r, emission_color2.r, emission_color3.r);
    material_value->value().emissiveFactor.g =
        sum_by_signal(emission_color.g, emission_color2.g, emission_color3.g);
    material_value->value().emissiveFactor.b =
        sum_by_signal(emission_color.b, emission_color2.b, emission_color3.b);

    material_value->dirty();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcMaterialAnimation::load_config(CfgReader &cfg)
{
    QString sec_name = "MaterialAnimation";

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
    {
        duration = tmp_dbl;
    }

    tmp_dbl = 0.0;
    if (cfg.getDouble(sec_name, "FixedSignal", tmp_dbl))
    {
        cur_signal = static_cast<float>(tmp_dbl);
        cur_signal2 = static_cast<float>(tmp_dbl);
        cur_signal3 = static_cast<float>(tmp_dbl);
        is_fixed_signal = true;
    }

    int tmp_int = -1;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
        signal_id = tmp_int;

    QString tmp_qstr = "1.0 1.0 1.0";
    if (cfg.getString(sec_name, "Color", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        vsg::vec4 config_color_limit = {1.0f, 1.0f, 1.0f, 1.0f};
        ss >> config_color_limit.r >> config_color_limit.g >> config_color_limit.b;
        config_color_limit.r = std::clamp(config_color_limit.r, 0.0f, 1.0f);
        config_color_limit.g = std::clamp(config_color_limit.g, 0.0f, 1.0f);
        config_color_limit.b = std::clamp(config_color_limit.b, 0.0f, 1.0f);
        base_color *= config_color_limit;
    }

    tmp_qstr = "1.0 1.0 1.0";
    if (cfg.getString(sec_name, "EmissionColor", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> emission_color.r >> emission_color.g >> emission_color.b;
        emission_color.r = std::clamp(emission_color.r, 0.0f, 1.0f);
        emission_color.g = std::clamp(emission_color.g, 0.0f, 1.0f);
        emission_color.b = std::clamp(emission_color.b, 0.0f, 1.0f);
    }

    tmp_int = -1;
    tmp_qstr = "0.0 0.0 0.0";
    if (cfg.getInt(sec_name, "SignalID2", tmp_int) &&
        cfg.getString(sec_name, "EmissionColor2", tmp_qstr))
    {
        signal_id2 = tmp_int;

        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> emission_color2.r >> emission_color2.g >> emission_color2.b;
        emission_color2.r = std::clamp(emission_color2.r, 0.0f, 1.0f);
        emission_color2.g = std::clamp(emission_color2.g, 0.0f, 1.0f);
        emission_color2.b = std::clamp(emission_color2.b, 0.0f, 1.0f);
    }

    tmp_int = -1;
    tmp_qstr = "0.0 0.0 0.0";
    if (cfg.getInt(sec_name, "SignalID3", tmp_int) &&
        cfg.getString(sec_name, "EmissionColor3", tmp_qstr))
    {
        signal_id3 = tmp_int;

        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> emission_color3.r >> emission_color3.g >> emission_color3.b;
        emission_color3.r = std::clamp(emission_color3.r, 0.0f, 1.0f);
        emission_color3.g = std::clamp(emission_color3.g, 0.0f, 1.0f);
        emission_color3.b = std::clamp(emission_color3.b, 0.0f, 1.0f);
    }

    emission_color.a = 1.0f;
    base_color.a = 1.0f;
    material_value->properties.dataVariance = vsg::DYNAMIC_DATA;
    update(cur_signal);
    return true;
}
