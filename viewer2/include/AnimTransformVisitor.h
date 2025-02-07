#ifndef ANIM_TRANSFORM_VISITOR_H
#define ANIM_TRANSFORM_VISITOR_H

#include "ProcAnimation.h"
#include "animations-list.h"

#include <vsg/core/Visitor.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Transform.h>

class AnimTransformVisitor : public vsg::Visitor
{
public:
    AnimTransformVisitor(animations_t* animations, const std::string& vehicle_config);

    virtual void apply(vsg::Transform& transform);

private:
    animations_t* animations;
    std::string vehicle_config;

    ProcAnimation* create_animation(const std::string& name, vsg::MatrixTransform* transform);
};

#endif // ANIM_TRANSFORM_VISITOR_H
