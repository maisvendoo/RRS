#include "AnalogRotation.h"
#include "ConfigReader.h"
#include <algorithm>
#include <sstream>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>

AnalogRotation::AnalogRotation(vsg::MatrixTransform* transform)
    : matrix(transform->matrix)
{
}

void AnalogRotation::anim_step(float t, float dt)
{
    cur_pos += (pos - cur_pos) * duration * dt;
    angle = interpolate(cur_pos);
    update();
}

bool AnalogRotation::load_config(ConfigReader& cfg)
{
    cfg.setSection("AnalogRotation");
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

    int inf = 0;
    cfg.getValue("Infinity", inf);

    infinity = static_cast<bool>(inf);

    std::string tmp;
    cfg.getValue("Axis", tmp);

    std::istringstream ss(tmp);
    ss >> axis.x >> axis.y >> axis.z;

    return true;
}

void AnalogRotation::update()
{
    if (keypoints.empty())
    {
        return;
    }

    if (!infinity)
    {
        angle = std::clamp(angle, keypoints.front().value, keypoints.back().value);
    }

    vsg::dmat4 rotate = vsg::rotate(static_cast<double>(vsg::radians(angle)), axis);
    transform->matrix = rotate * matrix;
}
