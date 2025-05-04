#include "AnimTransformVisitor.h"

#include "AnalogRotation.h"
#include "AnalogTranslation.h"
#include "animations-list.h"
#include "CfgReader.h"
#include "filesystem.h"
#include "MaterialAnimationVisitor.h"
#include "ProcAnimation.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include <QDomNode>
#include <QString>

#include <string>
#include <vulkan/vulkan_core.h>

AnimTransformVisitor::AnimTransformVisitor(const AnimTransformVisitorCreateInfo& create_info)
    : pdo(create_info.pdo)
    , duplicate(create_info.duplicate)
    , animations_dir(create_info.animations_dir)
    , animations(create_info.animations)
{
}

void AnimTransformVisitor::apply(vsg::Node& node)
{
    node.traverse(*this);
}

void AnimTransformVisitor::apply(vsg::MatrixTransform& transform)
{
    std::string name;
    transform.getValue("name", name);

    if (name.empty())
    {
        transform.getValue("Name", name);
    }

    if (name.empty())
    {
        transform.traverse(*this);
        return;
    }

    ProcAnimation* animation = create_animation(name, transform);
    if (animation)
    {
        animation->name = name;
        animations->insert({animation->getSignalID(), animation});
    }

    transform.traverse(*this);
}

void AnimTransformVisitor::apply(vsg::StateGroup& stateGroup)
{
    // vsg::RasterizationState* rasterState = nullptr;
    // for (auto& stateCommand : stateGroup.stateCommands)
    // {
    //     rasterState = dynamic_cast<vsg::RasterizationState*>(stateCommand.get());
    //     if (rasterState)
    //     {
    //         break;
    //     }
    // }

    // if (rasterState)
    // {
    //     rasterState->cullMode = VK_CULL_MODE_NONE;
    // }
    // else
    // {
    //     auto newRasterState = vsg::RasterizationState::create();
    //     newRasterState->cullMode = VK_CULL_MODE_NONE;
    //     stateGroup.add(newRasterState);
    // }

    stateGroup.traverse(*this);
}

ProcAnimation* AnimTransformVisitor::create_animation(const std::string& name, vsg::MatrixTransform& transform)
{
    FileSystem& fs = FileSystem::getInstance();
    std::string data_dir = fs.getDataDir();
    std::string file_path = data_dir
        + fs.separator() + "animations"
        + fs.separator() + animations_dir
        + fs.separator() + name + ".xml";

    CfgReader cfg;
    if (cfg.load(file_path.c_str()))
    {
        QDomNode config_section;
        ProcAnimation* animation = nullptr;

        config_section = cfg.getFirstSection("AnalogRotation");
        if (!config_section.isNull())
        {
            std::scoped_lock<std::mutex> pdo_lock(pdo->mutex);
            pdo->tag(&transform);
            auto new_transform = vsg::ref_ptr(transform.clone()->cast<vsg::MatrixTransform>());
            duplicate->insert(&transform, new_transform);

            animation = new AnalogRotation(new_transform.get());
            animation->load(cfg);

            return animation;
        }

        config_section = cfg.getFirstSection("AnalogTranslation");
        if (!config_section.isNull())
        {
            std::scoped_lock<std::mutex> pdo_lock(pdo->mutex);
            pdo->tag(&transform);
            auto new_transform = vsg::ref_ptr(transform.clone()->cast<vsg::MatrixTransform>());
            duplicate->insert(&transform, new_transform);

            animation = new AnalogTranslation(new_transform.get());
            animation->load(cfg);

            return animation;
        }

        config_section = cfg.getFirstSection("MaterialAnimation");
        if (!config_section.isNull())
        {
            auto inner_pdo = vsg::PropagateDynamicObjects::create();
            vsg::CopyOp copyop;
            auto inner_duplicate = copyop.duplicate = new vsg::Duplicate;

            MaterialAnimationVisitorCreateInfo mav_create_info = {
                .pdo = inner_pdo,
                .duplicate = inner_duplicate,
                .animations = animations,
                .cfg_reader = &cfg
            };

            MaterialAnimationVisitor mav(mav_create_info);
            transform.accept(mav);
            transform.accept(*inner_pdo);

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
                pdo->tag(&transform);
                duplicate->insert(&transform, copyop(vsg::ref_ptr(&transform)));
            }

            return nullptr;
        }
        
        // config_section = cfg.getFirstSection("MaterialRGBAnimation");
        // if (!config_section.isNull())
        // {
        //     return nullptr;
        // }
    }

    return nullptr;
}
