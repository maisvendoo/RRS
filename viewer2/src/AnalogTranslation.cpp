#include "AnalogTranslation.h"

#include "CfgReader.h"
#include "ProcAnimation.h"

#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <algorithm>
#include <string>
#include <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AnalogTranslation::AnalogTranslation(vsg::MatrixTransform* transform)
    : ProcAnimation(transform)
    , matrix(transform->matrix)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnalogTranslation::update()
{
    if (keypoints.empty())
    {
        return;
    }

    motion = interpolate(cur_pos);
    motion = std::clamp(motion, keypoints.front().value, keypoints.back().value);

    vsg::dmat4 translate = vsg::translate(axis * static_cast<double>(motion));
    transform->matrix = matrix * translate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnalogTranslation::anim_step([[maybe_unused]] float t, float dt)
{
    float delta = (pos - cur_pos);
    if (abs(delta) > 1e-5f)
    {
        cur_pos += delta * duration * dt;
        update();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AnalogTranslation::load_config(CfgReader &cfg)
{
    QString sec_name = "AnalogTranslation";

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
    {
        signal_id = tmp_int;
    }

    double tmp_dbl = 0.0;
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

    QString tmp_qstr = "0.0 0.0 1.0";
    if (cfg.getString(sec_name, "Axis", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> axis.x >> axis.y >> axis.z;

        // TODO: Возможно нужно раскомментировать
        // axis = vsg::normalize(axis);
    }

    // update();
    return true;
}
