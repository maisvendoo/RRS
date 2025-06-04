#ifndef MATERIAL_ANIMATION_VISITOR_H
#define MATERIAL_ANIMATION_VISITOR_H

#include "ProcAnimation.h"
#include "animations-list.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>

class CfgReader;

namespace vsg
{
    class BindDescriptorSet;
    class Duplicate;
    class Node;
    class PropagateDynamicObjects;
}

struct MaterialAnimationVisitorCreateInfo
{
    vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo;
    vsg::ref_ptr<vsg::Duplicate> duplicate;
    CfgReader& cfg_reader;
};

class MaterialAnimationVisitor : public vsg::Inherit<vsg::Visitor, MaterialAnimationVisitor>
{
public:
    explicit MaterialAnimationVisitor(const MaterialAnimationVisitorCreateInfo& create_info);

    void apply(vsg::Node& node) override;
    void apply(vsg::BindDescriptorSet& bindDescriptorSet) override;

    ProcAnimation* get_animation() { return animation; }

private:
    vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo;
    vsg::ref_ptr<vsg::Duplicate> duplicate;
    CfgReader& cfg_reader;
    ProcAnimation* animation = nullptr;
};

#endif // MATERIAL_ANIMATION_VISITOR_H
