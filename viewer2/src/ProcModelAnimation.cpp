#include "ProcModelAnimation.h"

#include "CfgReader.h"
#include "ProcAnimation.h"

#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcModelAnimation::ProcModelAnimation(vsg::ref_ptr<vsg::Animation> in_animation)
    : ProcAnimation(in_animation->name)
    , animation(in_animation)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcModelAnimation::anim_step([[maybe_unused]] float t, float dt)
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
bool ProcModelAnimation::load_config(CfgReader &cfg)
{
    QString sec_name = "ModelAnimation";

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

    update();
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcModelAnimation::update()
{
    // Применяем управлящий сигнал ко всем анимируемым элементам
    for (auto& sampler : animation->samplers)
    {
        sampler->update(static_cast<double>(cur_pos));
    }
}
