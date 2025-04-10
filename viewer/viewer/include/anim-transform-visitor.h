#ifndef     ANIM_TRANSFORM_VISITOR_H
#define     ANIM_TRANSFORM_VISITOR_H

#include    "animations-list.h"

#include    <osg/NodeVisitor>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AnimTransformVisitor : public osg::NodeVisitor
{
public:

    AnimTransformVisitor(animations_t *animations, const std::string &vehicle_config);

    virtual void apply(osg::Transform &transform);

private:

    animations_t *animations;
    std::string animations_dir;

    ProcAnimation *create_animation(const std::string &name, osg::MatrixTransform *transform);
};

#endif // ANIM_TRANSFORM_VISITOR_H
