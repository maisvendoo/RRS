#ifndef MATERIAL_ANIMATION_VISITOR_H
#define MATERIAL_ANIMATION_VISITOR_H

#include "animations-list.h"
#include <vsg/core/Visitor.h>
#include <vsg/nodes/StateGroup.h>

class CfgReader;

class MaterialAnimationVisitor : public vsg::Visitor
{
public:
    MaterialAnimationVisitor(animations_t* animations, CfgReader* cfg);

    void apply(vsg::Node& node) override;

    void apply(vsg::StateGroup& stateGroup) override;

private:
    animations_t* animations;
    CfgReader* cfg;
};

#endif // MATERIAL_ANIMATION_VISITOR_H
