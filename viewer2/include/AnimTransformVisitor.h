#ifndef ANIM_TRANSFORM_VISITOR_H
#define ANIM_TRANSFORM_VISITOR_H

#include "ProcAnimation.h"
#include "animations-list.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Transform.h>

class AnimTransformVisitor : public vsg::Inherit<vsg::Visitor, AnimTransformVisitor>
{
public:
    AnimTransformVisitor(animations_t* animations, const std::string& vehicle_config, vsg::ref_ptr<vsg::Node> main_node);

    void apply(vsg::Node& node) override;

    void apply(vsg::MatrixTransform& transform) override;

private:
    animations_t* animations;
    std::string vehicle_config;
    vsg::ref_ptr<vsg::Node> main_node;

    ProcAnimation* create_animation(const std::string& name, vsg::MatrixTransform& transform);

    // TODO: change name
    void copy_nodes();
};

#endif // ANIM_TRANSFORM_VISITOR_H
