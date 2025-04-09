#ifndef MATERIAL_ANIMATION_VISITOR_H
#define MATERIAL_ANIMATION_VISITOR_H

#include "animations-list.h"
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/state/BindDescriptorSet.h>

class CfgReader;

class MaterialAnimationVisitor : public vsg::Visitor
{
public:
    MaterialAnimationVisitor(animations_t* animations, CfgReader* cfg, vsg::ref_ptr<vsg::Options> options, vsg::ref_ptr<vsg::MatrixTransform>& root_node);

    void apply(vsg::Node& node) override;
    void apply(vsg::BindDescriptorSet& bindDescriptorSet) override;
    // void apply(vsg::MatrixTransform& transform) override;

private:
    vsg::ref_ptr<vsg::Options> options;
    vsg::ref_ptr<vsg::MatrixTransform>& root_node;

    animations_t* animations;
    CfgReader* cfg;
};

#endif // MATERIAL_ANIMATION_VISITOR_H
