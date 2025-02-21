#ifndef MATERIAL_RGB_ANIMATION_VISITOR_H
#define MATERIAL_RGB_ANIMATION_VISITOR_H

#include "ConfigReader.h"
#include "animations-list.h"
#include <vsg/core/Visitor.h>
#include <vsg/nodes/StateGroup.h>

class MaterialRgbAnimationVisitor : public vsg::Visitor
{
public:
    MaterialRgbAnimationVisitor(animations_t* animations, ConfigReader& cfg);

    virtual void apply(vsg::Node& node);

    virtual void apply(vsg::StateGroup& stateGroup);

private:
    animations_t* animations;
    ConfigReader* cfg;
};

#endif // MATERIAL_RGB_ANIMATION_VISITOR_H
