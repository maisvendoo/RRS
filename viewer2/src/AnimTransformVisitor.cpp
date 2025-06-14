#include "AnimTransformVisitor.h"

#include "filesystem.h"
#include "CfgReader.h"
#include "animations-list.h"
#include "ProcAnimation.h"
#include "AnalogRotation.h"
#include "AnalogTranslation.h"
#include "MaterialAnimationVisitor.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include <QDomNode>

#include <string>

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

        if (vsg::ref_ptr<AnalogRotation> rotation = animation.cast<AnalogRotation>())
        {
            rotation->setTransform(new_transform);
            continue;
        }
        if (vsg::ref_ptr<AnalogTranslation> translation = animation.cast<AnalogTranslation>())
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
        QDomNode config_section;
        vsg::ref_ptr<ProcAnimation> animation = nullptr;
        vsg::ref_ptr<vsg::Group> group_node = vsg::ref_ptr<vsg::Group>(&group);

        config_section = cfg.getFirstSection("AnalogRotation");
        if (!config_section.isNull())
        {
            std::scoped_lock<std::mutex> pdo_lock(pdo->mutex);
            pdo->tag(&group);

            animation = AnalogRotation::create(group_node.cast<vsg::MatrixTransform>());
            animation->load(cfg);

            deferred_animations.emplace_back(DeferredAnimation{&group, animation});

            return animation;
        }

        config_section = cfg.getFirstSection("AnalogTranslation");
        if (!config_section.isNull())
        {
            std::scoped_lock<std::mutex> pdo_lock(pdo->mutex);
            pdo->tag(&group);

            animation = AnalogTranslation::create(group_node.cast<vsg::MatrixTransform>());
            animation->load(cfg);

            deferred_animations.emplace_back(DeferredAnimation{&group, animation});

            return animation;
        }

        config_section = cfg.getFirstSection("MaterialAnimation");
        if (!config_section.isNull())
        {
            auto inner_pdo = vsg::PropagateDynamicObjects::create();
            vsg::CopyOp copyop;
            auto inner_duplicate = copyop.duplicate = new vsg::Duplicate;

            FindMaterialAnimationVisitorCreateInfo fmav_create_info = {inner_pdo, inner_duplicate, cfg};

            FindMaterialAnimationVisitor fmav(fmav_create_info);
            group.accept(fmav);
            group.accept(*inner_pdo);

            if (!inner_pdo->dynamicObjects.empty())
            {
                for (auto& object : inner_pdo->dynamicObjects)
                {
                    if (!inner_duplicate->contains(object))
                    {
                        inner_duplicate->insert(object);
                    }
                }

                std::scoped_lock<std::mutex> pdo_lock(pdo->mutex);
                pdo->tag(&group);
                duplicate->insert(&group, copyop(vsg::ref_ptr(&group)));
            }

            return fmav.get_animation();
        }
    }

    return nullptr;
}
