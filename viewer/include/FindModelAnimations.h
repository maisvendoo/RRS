#pragma once
#ifndef FIND_MODEL_ANIMATIONS_H
#define FIND_MODEL_ANIMATIONS_H

#include "animations-list.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>

#include <string>

class ProcAnimation;

namespace vsg
{
    class Node;
    class Animation;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct FindModelAnimationsCreateInfo final
{
    vsg::ref_ptr<vsg::Node> node;
    vsg::ref_ptr<animations_t> animations;
    std::string animations_dir;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FindModelAnimations final : public vsg::Inherit<vsg::Object, FindModelAnimations>
{
public:
    explicit FindModelAnimations(const FindModelAnimationsCreateInfo& create_info);

private:
    vsg::ref_ptr<vsg::Node> node;
    std::string animations_dir;
    vsg::ref_ptr<animations_t> animations;

    void find_animations();

    vsg::ref_ptr<ProcAnimation> create_animation(vsg::ref_ptr<vsg::Animation> model_animation);
};

#endif // FIND_MODEL_ANIMATIONS_H
