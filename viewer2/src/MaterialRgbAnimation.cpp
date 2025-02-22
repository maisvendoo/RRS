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

bool MaterialRgbAnimation::load_config(ConfigReader& cfg)
{
    cfg.setSection("MaterialRGBAnimation");
    cfg.getValue("SignalID", signal_id);
    cfg.getValue("Duration", duration);

    try
    {
        cfg.getValue("FixedSignal", fixed_signal);
        is_fixed_signal = true;
    }
    catch (...)
    {
        is_fixed_signal = false;
    }

    std::string tmp;
    cfg.getValue("EmissionColor", tmp);

    std::istringstream ss(tmp);
    ss >> emission_color.r >> emission_color.g >> emission_color.b;
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
