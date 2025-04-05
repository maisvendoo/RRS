#ifndef MATERIAL_RGB_ANIMATION_VISITOR_H
#define MATERIAL_RGB_ANIMATION_VISITOR_H

#include "animations-list.h"
#include <vsg/core/Visitor.h>
#include <vsg/nodes/StateGroup.h>

class CfgReader;

class MaterialRgbAnimationVisitor : public vsg::Visitor
{
public:
    MaterialRgbAnimationVisitor(animations_t* animations, CfgReader& cfg);

    void apply(vsg::Node& node) override;

    void apply(vsg::StateGroup& stateGroup) override;

private:
    animations_t* animations;
    CfgReader* cfg;
};

#endif // MATERIAL_RGB_ANIMATION_VISITOR_H
