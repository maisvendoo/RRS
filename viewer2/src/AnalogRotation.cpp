#include "AnalogRotation.h"

#include "CfgReader.h"
#include "ProcAnimation.h"

#include <iostream>
#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <algorithm>
#include <sstream>
#include <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AnalogRotation::AnalogRotation(vsg::ref_ptr<vsg::MatrixTransform> transform)
    : Inherit()
    , matrix(transform->matrix)
    , transform_node(transform)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnalogRotation::setTransform(vsg::ref_ptr<vsg::MatrixTransform> transform)
{
    transform_node = transform;
    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
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
    transform_node->matrix = matrix * rotate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnalogRotation::anim_step([[maybe_unused]] float t, float dt)
{
    float delta = pos - cur_pos;
    if (abs(delta) > 1e-5f)
    {
        cur_pos += delta * fmin(duration * dt, 1.0f);
        update();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AnalogRotation::load_config(CfgReader& cfg)
{
    QString sec_name = "AnalogRotation";

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
    {
        signal_id = tmp_int;
    }

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
    {
        duration = tmp_dbl;
    }

    tmp_dbl = 0.0;
    if (cfg.getDouble(sec_name, "FixedSignal", tmp_dbl))
    {
        fixed_signal = static_cast<float>(tmp_dbl);
        is_fixed_signal = true;
    }

    cfg.getBool(sec_name, "Infinity", infinity);

    QString tmp_qstr = "0.0 0.0 1.0";
    if (cfg.getString(sec_name, "Axis", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> axis.x >> axis.y >> axis.z;
        axis = vsg::normalize(axis);
    }

    // update();
    return true;
}
