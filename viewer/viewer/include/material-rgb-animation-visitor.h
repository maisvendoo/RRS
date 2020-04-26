#ifndef     MATERIAL_RGB_ANIMATION_VISITOR_H
#define     MATERIAL_RGB_ANIMATION_VISITOR_H

#include    <osg/NodeVisitor>
#include    "animations-list.h"

class MaterialRGBAnimationVisitor : public osg::NodeVisitor
{
public:

    MaterialRGBAnimationVisitor(animations_t *animations, ConfigReader *cfg);

    virtual void apply(osg::Geode &node);

private:

    animations_t *animations;
    ConfigReader *cfg;
};

#endif // MATERIAL_RGB_ANIMATION_VISITOR_H
