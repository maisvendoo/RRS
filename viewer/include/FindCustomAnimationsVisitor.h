#pragma once
#ifndef FIND_CUSTOM_ANIMATIONS_VISITOR_H
#define FIND_CUSTOM_ANIMATIONS_VISITOR_H

#include "animations-list.h"
#include "ProcAnimation.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>

#include <string>

class ProcAnimation;

namespace vsg
{
    class Duplicate;
    class MatrixTransform;
    class Node;
    class PropagateDynamicObjects;
}

struct FindCustomAnimationsVisitorCreateInfo final
{
    vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo;
    vsg::ref_ptr<vsg::Duplicate> duplicate;
    std::string animations_dir;
    vsg::ref_ptr<animations_t> animations;
};

struct DeferredAnimation final
{
    vsg::Node* node;
    vsg::ref_ptr<ProcAnimation> animation;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FindCustomAnimationsVisitor final : public vsg::Inherit<vsg::Visitor, FindCustomAnimationsVisitor>
{
public:
    explicit FindCustomAnimationsVisitor(const FindCustomAnimationsVisitorCreateInfo& create_info);

    void apply(vsg::Node& node) override;
    void apply(vsg::Group& group) override;
    void apply(vsg::Light &light) override;

    void reconfigure_animations();

private:
    vsg::ref_ptr<ProcAnimation> create_animation(const std::string& name, vsg::Group& group);

    vsg::ref_ptr<ProcAnimation> create_animation(const std::string& name, vsg::Light& light);

    template <typename AnimationClass>
    vsg::ref_ptr<ProcAnimation> create_transform_animation(const char* type, CfgReader& cfg, vsg::Group* group_ptr);

    template <typename VisitorClass>
    vsg::ref_ptr<ProcAnimation> create_material_animation(const char* type, CfgReader& cfg, vsg::Group* group_ptr);

    vsg::ref_ptr<ProcAnimation> create_light_animation(const char* type, CfgReader& cfg, vsg::Light* light_ptr);

private:
    vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo;
    vsg::ref_ptr<vsg::Duplicate> duplicate;
    std::string animations_dir;
    vsg::ref_ptr<animations_t> animations;
    std::vector<DeferredAnimation> deferred_animations;
};

#endif // FIND_CUSTOM_ANIMATIONS_VISITOR_H
