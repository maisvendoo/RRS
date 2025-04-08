#ifndef ANIM_TRANSFORM_VISITOR_H
#define ANIM_TRANSFORM_VISITOR_H

#include "animations-list.h"
#include "ProcAnimation.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/Node.h>

#include <string>

namespace vsg
{
    class MatrixTransform;
}

class AnimTransformVisitor : public vsg::Inherit<vsg::Visitor, AnimTransformVisitor>
{
public:
    AnimTransformVisitor(animations_t* animations, const std::string& vehicle_config, vsg::ref_ptr<vsg::MatrixTransform>& root_node, vsg::ref_ptr<vsg::Options> options);

    void apply(vsg::Node& node) override;

    void apply(vsg::MatrixTransform& transform) override;

private:
    vsg::ref_ptr<vsg::Options> options;

    animations_t* animations;
    std::string vehicle_config;
    vsg::ref_ptr<vsg::MatrixTransform>& root_node;

    ProcAnimation* create_animation(const std::string& name, vsg::ref_ptr<vsg::MatrixTransform> transform);

    // TODO: change name
    void copy_nodes(vsg::ref_ptr<vsg::MatrixTransform>& transform);
};

#endif // ANIM_TRANSFORM_VISITOR_H
