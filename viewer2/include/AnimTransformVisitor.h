#pragma once

#ifndef ANIM_TRANSFORM_VISITOR_H
#define ANIM_TRANSFORM_VISITOR_H

#include "animations-list.h"
#include "ProcAnimation.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>

#include <string>

class ProcAnimation;

namespace vsg
{
    class Duplicate;
    class MatrixTransform;
    class Node;
    class PropagateDynamicObjects;
}

struct AnimTransformVisitorCreateInfo
{
    vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo;
    vsg::ref_ptr<vsg::Duplicate> duplicate;
    std::string animations_dir;
    animations_t* animations;
};

struct DeferredAnimation
{
    vsg::Node* node;
    ProcAnimation* animation;
};

class AnimTransformVisitor : public vsg::Inherit<vsg::Visitor, AnimTransformVisitor>
{
public:
    explicit AnimTransformVisitor(const AnimTransformVisitorCreateInfo& create_info);

    void apply(vsg::Node& node) override;
    void apply(vsg::MatrixTransform& transform) override;
    void apply(vsg::Group& group) override;

    void reconfigure_animations();

private:
    ProcAnimation* create_animation(const std::string& name, vsg::MatrixTransform& transform);
    ProcAnimation* create_animation(const std::string& name, vsg::Group& group);

private:
    vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo;
    vsg::ref_ptr<vsg::Duplicate> duplicate;
    std::string animations_dir;
    animations_t* animations;
    std::vector<DeferredAnimation> deferred_animations;
};

#endif // ANIM_TRANSFORM_VISITOR_H
