#include "ProcModelAnimation.h"

#include "CfgReader.h"

#include <vsg/animation/Animation.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcModelAnimation::ProcModelAnimation(vsg::ref_ptr<vsg::Animation> in_animation)
    : Inherit()
    , animation(in_animation)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcModelAnimation::update(float current_signal)
{
    // Применяем управлящий сигнал ко всем анимируемым элементам
    for (auto& sampler : animation->samplers)
    {
        sampler->update(static_cast<double>(current_signal));
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
        cur_signal = static_cast<float>(tmp_dbl);
        is_fixed_signal = true;
    }

    update(cur_signal);
    return true;
}
