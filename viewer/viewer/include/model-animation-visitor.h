#ifndef     MODEL_ANIMATION_VISITOR_H
#define     MODEL_ANIMATION_VISITOR_H

#include    <osg/NodeVisitor>
#include    "model-parts-list.h"

class ModelAnimationVisitor : public osg::NodeVisitor
{
public:

    ModelAnimationVisitor(model_parts_list_t *parts, const std::string &anim_name);

    virtual void apply(osg::Transform &transform);

private:

    model_parts_list_t  *parts;
    std::string         anim_name;
};


#endif // MODEL_ANIMATION_VISITOR_H
