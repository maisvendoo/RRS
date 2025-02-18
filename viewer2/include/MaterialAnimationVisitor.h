#ifndef MATERIAL_ANIMATION_VISITOR_H
#define MATERIAL_ANIMATION_VISITOR_H

#include "ConfigReader.h"
#include "animations-list.h"
#include <vsg/core/Visitor.h>
#include <vsg/nodes/StateGroup.h>

class MaterialAnimationVisitor : public vsg::Visitor
{
public:
    MaterialAnimationVisitor(animations_t* animations, ConfigReader* cfg);

    virtual void apply(vsg::Node& node);

    virtual void apply(vsg::StateGroup& stateGroup);

private:
    animations_t* animations;
    ConfigReader* cfg;
};

#endif // MATERIAL_ANIMATION_VISITOR_H
