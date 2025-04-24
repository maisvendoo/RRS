#pragma once
#ifndef ANIM_TRANSFORM_VISITOR_H
#define ANIM_TRANSFORM_VISITOR_H

#include "animations-list.h"

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

class AnimTransformVisitor : public vsg::Inherit<vsg::Visitor, AnimTransformVisitor>
{
public:
    explicit AnimTransformVisitor(const AnimTransformVisitorCreateInfo& create_info);

    void apply(vsg::Node& node) override;
    void apply(vsg::MatrixTransform& transform) override;

private:
    ProcAnimation* create_animation(const std::string& name, vsg::MatrixTransform& transform);

private:
    vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo;
    vsg::ref_ptr<vsg::Duplicate> duplicate;
    std::string animations_dir;
    animations_t* animations;
};

#endif // ANIM_TRANSFORM_VISITOR_H
