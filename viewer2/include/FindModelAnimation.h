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
struct FindModelAnimationsCreateInfo
{
    vsg::ref_ptr<vsg::Node> node;
    animations_t* animations;
    std::string animations_dir;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FindModelAnimations : public vsg::Inherit<vsg::Object, FindModelAnimations>
{
public:

    FindModelAnimations(const FindModelAnimationsCreateInfo& create_info);

private:

    vsg::ref_ptr<vsg::Node> node;
    std::string animations_dir;
    animations_t* animations;

    void find_animations();
    ProcAnimation* create_animation(vsg::ref_ptr<vsg::Animation> model_animation);
};

#endif // FIND_MODEL_ANIMATIONS_H
