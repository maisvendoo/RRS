#pragma once
#ifndef FIND_MATERIAL_ANIMATION_H
#define FIND_MATERIAL_ANIMATION_H

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
class FindMaterialAnimationVisitor final : public vsg::Inherit<vsg::Visitor, FindMaterialAnimationVisitor>
{
public:
    FindMaterialAnimationVisitor(
        vsg::ref_ptr<vsg::PropagateDynamicObjects> in_pdo,
        vsg::ref_ptr<vsg::Duplicate> in_duplicate
    );

    void apply(vsg::Node& node) override;
    void apply(vsg::BindDescriptorSet& bindDescriptorSet) override;

    vsg::ref_ptr<ProcAnimation> get_animation() { return animation; }

private:
    vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo;
    vsg::ref_ptr<vsg::Duplicate> duplicate;
    vsg::ref_ptr<ProcAnimation> animation = nullptr;
};

#endif // MATERIAL_ANIMATION_VISITOR_H
