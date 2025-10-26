#include "ProcVisibleAnimation.h"

#include "CfgReader.h"
#include "ProcAnimation.h"

#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcVisibleAnimation::ProcVisibleAnimation(vsg::ref_ptr<vsg::Group> group)
    : Inherit()
    , group_node(group)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcVisibleAnimation::setGroup(vsg::ref_ptr<vsg::Group> group)
{
    group_node = group;

    group_with_children->children = group_node->children;

    visible_switch->children = {{vsg::MASK_OFF, group_with_children}};

    group_node->children = {visible_switch};

    if (!is_fixed_signal)
    {
        cur_signal = 0.0f;
    }

    update(cur_signal);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcVisibleAnimation::anim_step([[maybe_unused]] float t, [[maybe_unused]] float dt)
{
    if (is_fixed_signal)
    {
        return;
    }

    if (server_signals && (signal_id >= 0) && (static_cast<std::size_t>(signal_id) < server_signals->size()))
    {
        cur_signal = (*server_signals)[signal_id];
    }
    else
    {
        cur_signal = 0.0f;
    }

    update(cur_signal);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcVisibleAnimation::update(float current_signal)
{
    visible_switch->setAllChildren(current_signal > 1e-5);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcVisibleAnimation::load_config(CfgReader &cfg)
{
    QString sec_name = "VisibleAnimation";

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
    {
        signal_id = tmp_int;
    }

    duration = 1.0f;

    double tmp_dbl = 0.0;
    if (cfg.getDouble(sec_name, "FixedSignal", tmp_dbl))
    {
        cur_signal = static_cast<float>(tmp_dbl);
        is_fixed_signal = true;
    }

    // update();
    return true;
}
