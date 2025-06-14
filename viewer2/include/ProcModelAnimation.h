#ifndef PROC_MODEL_ANIMATION_H
#define PROC_MODEL_ANIMATION_H

#include "ProcAnimation.h"

class CfgReader;

namespace vsg
{
    class Animation;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ProcModelAnimation : public vsg::Inherit<ProcAnimation, ProcModelAnimation>
{
public:
    explicit ProcModelAnimation(vsg::ref_ptr<vsg::Animation> in_animation);

private:
    vsg::ref_ptr<vsg::Animation> animation;

    float cur_pos = 0.0;

    void anim_step(float t, float dt) override;

    bool load_config(CfgReader& cfg) override;

    void update();
};

#endif // PROC_MODEL_ANIMATION_H
