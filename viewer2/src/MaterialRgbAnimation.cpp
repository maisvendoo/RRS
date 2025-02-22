#include "CfgReader.h"
#include "MaterialRgbAnimation.h"
#include "ProcAnimation.h"
#include <sstream>

MaterialRgbAnimation::MaterialRgbAnimation(vsg::PbrMaterial& material)
    : ProcAnimation()
    , material(material)
    , color(material.baseColorFactor)
    , emission_color(0.0f, 0.0f, 0.0f, 1.0f)
{
}

void MaterialRgbAnimation::anim_step(float t, float dt)
{
    cur_pos_r += (pos_r - cur_pos_r) * duration * dt;
    cur_pos_g += (pos_g - cur_pos_g) * duration * dt;
    cur_pos_b += (pos_b - cur_pos_b) * duration * dt;
    update();
}

bool MaterialRgbAnimation::load_config(CfgReader &cfg)
{
    QString sec_name = "MaterialRGBAnimation";

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
        signal_id = tmp_int;

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
        duration = tmp_dbl;

    cfg.getBool(sec_name, "FixedSignal", is_fixed_signal);

    QString tmp_qstr = "0.0 0.0 0.0";
    if (cfg.getString(sec_name, "EmissionColor", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> emission_color.x >> emission_color.y >> emission_color.z;
    }

    return true;
}

float MaterialRgbAnimation::getChannelState(float pos, unsigned char channel)
{
    float channel_state = 0.0f;
    unsigned char mask = static_cast<unsigned char>(pos);
    channel_state = static_cast<float>(mask & (1 << channel));
    return channel_state;
}

void MaterialRgbAnimation::update()
{
    pos_r = getChannelState(pos, 0);
    pos_g = getChannelState(pos, 1);
    pos_b = getChannelState(pos, 2);

    vsg::vec4 new_color;
    new_color.r = color.r * interpolate(cur_pos_r);
    new_color.g = color.g * interpolate(cur_pos_g);
    new_color.b = color.b * interpolate(cur_pos_b);
    new_color.a = 1.0f;

    vsg::vec4 new_emission_color;
    new_emission_color.r = emission_color.r * cur_pos_r;
    new_emission_color.g = emission_color.g * cur_pos_g;
    new_emission_color.b = emission_color.b * cur_pos_b;
    new_emission_color.a = 1.0f;

    material.baseColorFactor = new_color;
    material.emissiveFactor = new_emission_color;
}
