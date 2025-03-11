#ifndef MATERIAL_ANIMATION_VISITOR_H
#define MATERIAL_ANIMATION_VISITOR_H

#include "animations-list.h"
#include <vsg/core/Visitor.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>

class CfgReader;

class MaterialAnimationVisitor : public vsg::Visitor
{
public:
    MaterialAnimationVisitor(animations_t* animations, CfgReader* cfg, vsg::ref_ptr<vsg::Node> parent_node);

    void apply(vsg::Node& node) override;

    void apply(vsg::MatrixTransform& mtransform) override;

private:
    animations_t* animations;
    CfgReader* cfg;
    vsg::ref_ptr<vsg::Node> parent_node;
};

#endif // MATERIAL_ANIMATION_VISITOR_H
