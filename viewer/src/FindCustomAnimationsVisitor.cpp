#include "FindCustomAnimationsVisitor.h"

#include "CfgReader.h"
#include "filesystem.h"
#include "FindDisplayAnimationVisitor.h"
#include "FindMaterialAnimationVisitor.h"
#include "ProcAnimation.h"
#include "ProcRotationAnimation.h"
#include "ProcTranslationAnimation.h"
#include "ProcVisibleAnimation.h"
#include "ProcLightAnimation.h"

#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/lighting/SpotLight.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include <string>
#include <Logger.h>

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

    // Некоторые варианты экспорта в .gltf присваивают имя источника света
    // родительскому узлу, а сам свет оставляют безымянным, исправляем это
    for (vsg::ref_ptr<vsg::Node>& child : group.children)
    {
        if (auto light = child.cast<vsg::Light>())
        {
            if (light->name.empty())
            {
                light->name = name;
            }
        }
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
void FindCustomAnimationsVisitor::apply(vsg::Light &light)
{
    if (light.name.empty())
    {
        light.traverse(*this);
        return;
    }

    vsg::ref_ptr<ProcAnimation> animation = create_animation(light.name, light);
    if (animation)
    {
        animation->name = light.name;
        animations->thread_safe_insert({animation->getSignalID(), animation});
    }

    light.traverse(*this);
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


        if (vsg::ref_ptr<ProcVisibleAnimation> visible = animation.cast<ProcVisibleAnimation>())
        {
            vsg::ref_ptr<vsg::Group> new_group = new_object.cast<vsg::Group>();
            visible->setGroup(new_group);
            continue;
        }

        if (vsg::ref_ptr<ProcRotationAnimation> rotation = animation.cast<ProcRotationAnimation>())
        {
            vsg::ref_ptr<vsg::MatrixTransform> new_transform = new_object.cast<vsg::MatrixTransform>();
            rotation->setTransform(new_transform);
            continue;
        }

        if (vsg::ref_ptr<ProcTranslationAnimation> translation = animation.cast<ProcTranslationAnimation>())
        {
            vsg::ref_ptr<vsg::MatrixTransform> new_transform = new_object.cast<vsg::MatrixTransform>();
            translation->setTransform(new_transform);
            continue;
        }

        if (vsg::ref_ptr<ProcLightAnimation> light_anim = animation.cast<ProcLightAnimation>())
        {
            vsg::ref_ptr<vsg::Light> new_light = new_object.cast<vsg::Light>();
            light_anim->setLight(new_light);
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
        auto animation = create_visible_animation("VisibleAnimation", cfg, &group);
        if (animation)
        {
            return animation;
        }

        animation = create_transform_animation<ProcRotationAnimation>("AnalogRotation", cfg, &group);
        if (animation)
        {
            return animation;
        }

        animation = create_transform_animation<ProcTranslationAnimation>("AnalogTranslation", cfg, &group);
        if (animation)
        {
            return animation;
        }

        animation = create_material_animation<FindMaterialAnimationVisitor>("MaterialAnimation", cfg, &group, name);
        if (animation)
        {
            return animation;
        }

        animation = create_material_animation<FindDisplayAnimationVisitor>("Display", cfg, &group, name);
        if (animation)
        {
            return animation;
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<ProcAnimation> FindCustomAnimationsVisitor::create_animation(const std::string &name, vsg::Light &light)
{
    std::string file_path = animations_dir + name + ".xml";

    CfgReader cfg;
    if (cfg.load(file_path.c_str()))
    {
        auto animation = create_light_animation("LightAnimation", cfg, &light);
        if (animation)
        {
            return animation;
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<ProcAnimation> FindCustomAnimationsVisitor::create_visible_animation(const char *type, CfgReader &cfg, vsg::Group *group_ptr)
{
    const auto config_section = cfg.getFirstSection(type);
    if (!config_section.isNull())
    {
        const auto animation = ProcVisibleAnimation::create(vsg::ref_ptr(group_ptr));
        if (animation && animation->load(cfg))
        {
            const std::scoped_lock pdo_lock(pdo->mutex);
            pdo->tag(group_ptr);
            deferred_animations.emplace_back(DeferredAnimation{group_ptr, animation});

            return animation;
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <typename AnimationClass>
vsg::ref_ptr<ProcAnimation> FindCustomAnimationsVisitor::create_transform_animation(const char* type, CfgReader& cfg, vsg::Group* group_ptr)
{
    const auto config_section = cfg.getFirstSection(type);
    if (!config_section.isNull())
    {
        if (const auto transform_node = vsg::ref_ptr(group_ptr).template cast<vsg::MatrixTransform>())
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <typename VisitorClass>
vsg::ref_ptr<ProcAnimation> FindCustomAnimationsVisitor::create_material_animation(const char* type, CfgReader& cfg, vsg::Group* group_ptr, const std::string& name)
{
    const auto config_section = cfg.getFirstSection(type);
    if (!config_section.isNull())
    {
        const auto inner_pdo = vsg::PropagateDynamicObjects::create();
        const vsg::CopyOp inner_copyop;
        inner_copyop.duplicate = new vsg::Duplicate;

        VisitorClass animation_visitor(inner_pdo, inner_copyop.duplicate, name);
        group_ptr->traverse(animation_visitor);

        const auto animation = animation_visitor.get_animation();
        if (animation && animation->load(cfg))
        {
            group_ptr->accept(*inner_pdo);
            for (auto& object : inner_pdo->dynamicObjects)
            {
                if (!inner_copyop.duplicate->contains(object))
                {
                    inner_copyop.duplicate->insert(object);
                }
            }

            if (!inner_copyop.duplicate->duplicates.empty())
            {
                const std::scoped_lock pdo_lock(pdo->mutex);
                pdo->tag(group_ptr);
                duplicate->insert(group_ptr, inner_copyop(vsg::ref_ptr(group_ptr)));
            }

            return animation;
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<ProcAnimation> FindCustomAnimationsVisitor::create_light_animation(const char *type, CfgReader &cfg, vsg::Light *light_ptr)
{
    const auto config_section = cfg.getFirstSection(type);
    if (!config_section.isNull())
    {
        const auto animation = ProcLightAnimation::create(vsg::ref_ptr(light_ptr));
        if (animation && animation->load(cfg))
        {
            const std::scoped_lock pdo_lock(pdo->mutex);
            pdo->tag(light_ptr);
            deferred_animations.emplace_back(DeferredAnimation{light_ptr, animation});

            return animation;
        }
    }
    else
    {
        LOG_ERROR("Section LightAnimation is't found");
    }

    return nullptr;
}
