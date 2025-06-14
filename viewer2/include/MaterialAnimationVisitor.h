#ifndef FIND_MATERIAL_ANIMATION_H
#define FIND_MATERIAL_ANIMATION_H

#include "ProcAnimation.h"

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

struct FindMaterialAnimationVisitorCreateInfo
{
    vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo;
    vsg::ref_ptr<vsg::Duplicate> duplicate;
    CfgReader& cfg_reader;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FindMaterialAnimationVisitor : public vsg::Inherit<vsg::Visitor, FindMaterialAnimationVisitor>
{
public:
    explicit FindMaterialAnimationVisitor(const FindMaterialAnimationVisitorCreateInfo& create_info);

    void apply(vsg::Node& node) override;
    void apply(vsg::BindDescriptorSet& bindDescriptorSet) override;

    vsg::ref_ptr<ProcAnimation> get_animation() { return animation; }

private:
    vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo;
    vsg::ref_ptr<vsg::Duplicate> duplicate;
    CfgReader& cfg_reader;
    vsg::ref_ptr<ProcAnimation> animation = nullptr;
};

#endif // MATERIAL_ANIMATION_VISITOR_H
