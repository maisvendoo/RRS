#ifndef     MATERIAL_ANIMATION_VISITOR_H
#define     MATERIAL_ANIMATION_VISITOR_H

#include    <osg/NodeVisitor>
#include    "animations-list.h"

class ConfigReader;

class MaterialAnimationVisitor : public osg::NodeVisitor
{
public:

    MaterialAnimationVisitor(animations_t *animations, ConfigReader *cfg);

    virtual void apply(osg::Geode &node);

private:
    animations_t *animations;
    ConfigReader *cfg_reader;
};

#endif // MATERIAL_ANIMATION_VISITOR_H
