#include "FindCustomAnimationsVisitor.h"

#include "CfgReader.h"
#include "filesystem.h"
#include "FindDisplayAnimationVisitor.h"
#include "FindMaterialAnimationVisitor.h"
#include "ProcAnimation.h"
#include "ProcRotationAnimation.h"
#include "ProcTranslationAnimation.h"
#include "ProcLightAnimation.h"

#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include <string>
#include <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FindCustomAnimationsVisitor::FindCustomAnimationsVisitor(const FindCustomAnimationsVisitorCreateInfo& create_info)
    : pdo(create_info.pdo)
    , duplicate(create_info.duplicate)
    , animations(create_info.animations)
{
    FileSystem& fs = FileSystem::getInstance();
    std::string animations_dir_path = fs.getDataDir() + fs.separator()
                                      + "animations" + fs.separator()
                                      + create_info.animations_dir + fs.separator();
    animations_dir = animations_dir_path;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FindCustomAnimationsVisitor::apply(vsg::Node& node)
{
    node.traverse(*this);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FindCustomAnimationsVisitor::apply(vsg::Group& group)
{
    std::string name;
    group.getValue("name", name);

    if (name.empty())
    {
        group.getValue("Name", name);
    }

    if (name.empty())
    {
        group.traverse(*this);
        return;
    }

    vsg::ref_ptr<ProcAnimation> animation = create_animation(name, group);
    if (animation)
    {
        animation->name = name;
        animations->thread_safe_insert({animation->getSignalID(), animation});
    }

    group.traverse(*this);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FindCustomAnimationsVisitor::reconfigure_animations()
{
    for (const auto& deferred_animation : deferred_animations)
    {
        vsg::ref_ptr<ProcAnimation> animation = deferred_animation.animation;

        vsg::ref_ptr<vsg::Object> new_object = duplicate->duplicates[deferred_animation.node];
        vsg::ref_ptr<vsg::MatrixTransform> new_transform = new_object.cast<vsg::MatrixTransform>();

        if (vsg::ref_ptr<ProcRotationAnimation> rotation = animation.cast<ProcRotationAnimation>())
        {
            rotation->setTransform(new_transform);
            continue;
        }
        if (vsg::ref_ptr<ProcTranslationAnimation> translation = animation.cast<ProcTranslationAnimation>())
        {
            translation->setTransform(new_transform);
            continue;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<ProcAnimation> FindCustomAnimationsVisitor::create_animation(const std::string& name, vsg::Group& group)
{
    std::string file_path = animations_dir + name + ".xml";

    CfgReader cfg;
    if (cfg.load(file_path.c_str()))
    {
        auto animation = create_transform_animation<ProcRotationAnimation>("AnalogRotation", cfg, &group);
        if (animation)
        {
            return animation;
        }

        animation = create_transform_animation<ProcTranslationAnimation>("AnalogTranslation", cfg, &group);
        if (animation)
        {
            return animation;
        }

        animation = create_material_animation<FindMaterialAnimationVisitor>("MaterialAnimation", cfg, &group);
        if (animation)
        {
            return animation;
        }

        animation = create_material_animation<FindDisplayAnimationVisitor>("Display", cfg, &group);
        if (animation)
        {
            return animation;
        }

        animation = create_light_animation<ProcLightAnimation>("LightAnimation", cfg, &group);
        if (animation)
        {
            return animation;
        }
    }

    return nullptr;
}

template <typename AnimationClass>
vsg::ref_ptr<ProcAnimation> FindCustomAnimationsVisitor::create_transform_animation(const char* type, CfgReader& cfg, vsg::Group* group_ptr)
{
    const auto group_node = vsg::ref_ptr(group_ptr);

    const auto config_section = cfg.getFirstSection(type);
    if (!config_section.isNull())
    {
        if (const auto transform_node = group_node.template cast<vsg::MatrixTransform>())
        {
            const auto animation = AnimationClass::create(transform_node);
            if (animation && animation->load(cfg))
            {
                const std::scoped_lock pdo_lock(pdo->mutex);
                pdo->tag(group_ptr);
                deferred_animations.emplace_back(DeferredAnimation{group_ptr, animation});

                return animation;
            }
        }
    }

    return nullptr;
}

template <typename VisitorClass>
vsg::ref_ptr<ProcAnimation> FindCustomAnimationsVisitor::create_material_animation(const char* type, CfgReader& cfg, vsg::Group* group_ptr)
{
    const auto config_section = cfg.getFirstSection(type);
    if (!config_section.isNull())
    {
        const auto inner_pdo = vsg::PropagateDynamicObjects::create();
        const vsg::CopyOp inner_copyop;
        inner_copyop.duplicate = new vsg::Duplicate;

        VisitorClass animation_visitor(inner_pdo, inner_copyop.duplicate);
        group_ptr->accept(animation_visitor);

        const auto animation = animation_visitor.get_animation();
        if (animation && animation->load(cfg))
        {
            group_ptr->accept(*inner_pdo);
            if (!inner_pdo->dynamicObjects.empty())
            {
                for (auto& object : inner_pdo->dynamicObjects)
                {
                    if (!inner_copyop.duplicate->contains(object))
                    {
                        inner_copyop.duplicate->insert(object);
                    }
                }

                const std::scoped_lock pdo_lock(pdo->mutex);
                pdo->tag(group_ptr);
                duplicate->insert(group_ptr, inner_copyop(vsg::ref_ptr(group_ptr)));
            }

            return animation;
        }
    }

    return nullptr;
}

template<typename AnimationClass>
vsg::ref_ptr<ProcAnimation> FindCustomAnimationsVisitor::create_light_animation(const char *type, CfgReader &cfg, vsg::Group *group_ptr)
{
    std::cout << "Find config " << type << std::endl;
    const auto group_node = vsg::ref_ptr(group_ptr);

    const auto config_section = cfg.getFirstSection(type);
    if (!config_section.isNull())
    {
        if (const auto light_node = group_node.template cast<vsg::Light>())
        {
            const auto animation = AnimationClass::create(light_node);
            if (animation && animation->load(cfg))
            {
                const std::scoped_lock pdo_lock(pdo->mutex);
                pdo->tag(group_ptr);
                deferred_animations.emplace_back(DeferredAnimation{group_ptr, animation});

                return animation;
            }
        }
        else
        {
            std::cout << "EE: Can't convert Group node to Light node!!!" << std::endl;
        }
    }

    return nullptr;
}
