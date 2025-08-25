#include "ProcTranslationAnimation.h"

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
ProcTranslationAnimation::ProcTranslationAnimation(vsg::ref_ptr<vsg::MatrixTransform> transform)
    : Inherit()
    , matrix(transform->matrix)
    , transform_node(transform)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcTranslationAnimation::setTransform(vsg::ref_ptr<vsg::MatrixTransform> transform)
{
    transform_node = transform;

    if (!is_fixed_signal)
    {
        cur_signal = 0.0f;
    }

    update(cur_signal);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcTranslationAnimation::update(float current_signal)
{
    if (keypoints.empty())
    {
        return;
    }

    motion = interpolate(current_signal);

    //--------------------------------------------------------------------------
    // Костыль на случай, если keypoints.front.value() > keypoints.back.value()
    // (в std::clamp тогда провалится assert)
    //--------------------------------------------------------------------------
    // float min, max;
    // if (keypoints.front().value > keypoints.back().value)
    // {
    //     max = keypoints.front().value;
    //     min = keypoints.back().value;
    // }
    // else
    // {
    //     max = keypoints.back().value;
    //     min = keypoints.front().value;
    // }

    // motion = std::clamp(motion, min, max);
    //--------------------------------------------------------------------------

    motion = std::clamp(motion, keypoints.front().value, keypoints.back().value);

    vsg::dmat4 translate = vsg::translate(axis * static_cast<double>(motion));
    transform_node->matrix = matrix * translate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcTranslationAnimation::load_config(CfgReader &cfg)
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
        cur_signal = static_cast<float>(tmp_dbl);
        is_fixed_signal = true;
    }

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
