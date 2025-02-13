#include "AnalogTranslation.h"
#include "ConfigReader.h"
#include "ProcAnimation.h"
#include <algorithm>
#include <sstream>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/MatrixTransform.h>

AnalogTranslation::AnalogTranslation(vsg::MatrixTransform* transform)
    : ProcAnimation(transform)
    , matrix(transform->matrix)
{
}

void AnalogTranslation::anim_step(float t, float dt)
{
    cur_pos += (pos - cur_pos) * duration * dt;
    motion = interpolate(cur_pos);
    update();
}

bool AnalogTranslation::load_config(ConfigReader& cfg)
{
    cfg.setSection("AnalogTranslation");
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
    cfg.getValue("Axis", tmp);

    std::istringstream ss(tmp);
    ss >> axis.x >> axis.y >> axis.z;

    return true;
}

void AnalogTranslation::update()
{
    if (keypoints.empty())
    {
        return;
    }

    motion = std::clamp(motion, keypoints.front().value, keypoints.back().value);

    vsg::dmat4 translate = vsg::translate(axis * static_cast<double>(motion));
    transform->matrix = translate * matrix;
}
