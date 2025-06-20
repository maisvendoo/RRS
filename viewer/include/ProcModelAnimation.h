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
class ProcModelAnimation final : public vsg::Inherit<ProcAnimation, ProcModelAnimation>
{
public:
    explicit ProcModelAnimation(vsg::ref_ptr<vsg::Animation> in_animation);

private:
    vsg::ref_ptr<vsg::Animation> animation;

    void update(float current_signal) override;

    bool load_config(CfgReader& cfg) override;
};

#endif // PROC_MODEL_ANIMATION_H
