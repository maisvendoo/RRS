#include "AnalogRotation.h"

#include "CfgReader.h"
#include "ProcAnimation.h"

#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <algorithm>
#include <sstream>
#include <string>

AnalogRotation::AnalogRotation(vsg::MatrixTransform* transform)
    : ProcAnimation(transform)
    , matrix(transform->matrix)
{
}

void AnalogRotation::anim_step(float t, float dt)
{
    float delta = (pos - cur_pos);
    if (abs(delta) > 1e-5f)
    {
        cur_pos += delta * duration * dt;
        update();
    }
}

bool AnalogRotation::load_config(CfgReader& cfg)
{
    QString sec_name = "AnalogRotation";

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
        signal_id = tmp_int;

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
        duration = tmp_dbl;

    cfg.getBool(sec_name, "FixedSignal", is_fixed_signal);

    cfg.getBool(sec_name, "Infinity", infinity);

    QString tmp_qstr = "0.0 0.0 1.0";
    if (cfg.getString(sec_name, "Axis", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> axis.x >> axis.y >> axis.z;
    }

    return true;
}

void AnalogRotation::update()
{
    if (keypoints.empty())
    {
        return;
    }

    angle = interpolate(cur_pos);

    if (!infinity)
    {
        angle = std::clamp(angle, keypoints.front().value, keypoints.back().value);
    }

    vsg::dmat4 rotate = vsg::rotate(static_cast<double>(vsg::radians(angle)), axis);
    transform->matrix = rotate * matrix;
}
