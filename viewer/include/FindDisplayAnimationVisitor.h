#pragma once
#ifndef FIND_DISPLAY_ANIMATION_H
#define FIND_DISPLAY_ANIMATION_H

#include "ProcAnimation.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>

namespace vsg
{
    class BindDescriptorSet;
    class Duplicate;
    class Node;
    class PropagateDynamicObjects;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FindDisplayAnimationVisitor final : public vsg::Inherit<vsg::Visitor, FindDisplayAnimationVisitor>
{
public:
    FindDisplayAnimationVisitor(vsg::ref_ptr<vsg::PropagateDynamicObjects> in_pdo,
                                vsg::ref_ptr<vsg::Duplicate> in_duplicate,
                                const std::string &in_node_name);

    void apply(vsg::Node& node) override;
    void apply(vsg::Group& group) override;
    void apply(vsg::BindDescriptorSet& bindDescriptorSet) override;

    vsg::ref_ptr<ProcAnimation> get_animation() { return animation; }

private:
    vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo;
    vsg::ref_ptr<vsg::Duplicate> duplicate;
    std::string node_name;
    vsg::ref_ptr<ProcAnimation> animation = nullptr;
};

#endif // FIND_DISPLAY_ANIMATION_H
