#ifndef PROC_VISIBLE_ANIMATION_H
#define PROC_VISIBLE_ANIMATION_H

#include "ProcAnimation.h"

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/Switch.h>
//#include <mutex>

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcVisibleAnimation final : public vsg::Inherit<ProcAnimation, ProcVisibleAnimation>
{
public:
    explicit ProcVisibleAnimation(vsg::ref_ptr<vsg::Group> group);

    void setGroup(vsg::ref_ptr<vsg::Group> group);

private:

    vsg::ref_ptr<vsg::Group> group_node = nullptr;

    vsg::ref_ptr<vsg::Group> group_with_children = vsg::Group::create();
    vsg::ref_ptr<vsg::Switch> visible_switch = vsg::Switch::create();

    void anim_step(float t, float dt) override;

    void update(float current_signal) override;

    bool load_config(CfgReader &cfg) override;
};

#endif // PROC_VISIBLE_ANIMATION_H
