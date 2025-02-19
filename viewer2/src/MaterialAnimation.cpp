#include "MaterialAnimation.h"
#include "ConfigReader.h"
#include "ProcAnimation.h"
#include <sstream>
#include <vsg/maths/vec4.h>

MaterialAnimation::MaterialAnimation(vsg::PbrMaterial& material)
    : ProcAnimation()
    , material(material)
    , color(material.baseColorFactor)
    , emission_color(0.0f, 0.0f, 0.0f, 1.0f)
{
    pos = 0.0f;
    duration = 0.0f;
}

void MaterialAnimation::anim_step(float t, float dt)
{
    cur_pos += (pos - cur_pos) * duration * dt;
    update();
}

bool MaterialAnimation::load_config(ConfigReader& cfg)
{
    cfg.setSection("MaterialAnimation");
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

    std::string emission_tmp;
    cfg.getValue("EmissionColor", emission_tmp);

    std::istringstream ss(emission_tmp);
    ss >> emission_color.r >> emission_color.g >> emission_color.b;

    std::string color_tmp = "";

    try
    {
        cfg.getValue("Color", color_tmp);
        std::istringstream ss2(color_tmp);
        ss2 >> color.r >> color.g >> color.b;
    }
    catch (...)
    {
    }

    material.baseColorFactor = color;
    material.emissiveFactor = emission_color;

    return true;
}

void MaterialAnimation::update()
{
    vsg::vec4 new_color = color * interpolate(cur_pos);
    new_color.a = 1.0f;

    vsg::vec4 new_emission_color = emission_color * cur_pos;
    new_emission_color.a = 1.0f;

    material.baseColorFactor = new_color;
    material.emissiveFactor = new_emission_color;
}
