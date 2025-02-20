#include "AnimTransformVisitor.h"
#include "AnalogRotation.h"
#include "AnalogTranslation.h"
#include "ConfigReader.h"
#include "MaterialAnimationVisitor.h"
#include "ProcAnimation.h"
#include "filesystem.h"
#include <cstring>
#include <iostream>
#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>

AnimTransformVisitor::AnimTransformVisitor(animations_t* animations, const std::string& vehicle_config)
    : animations(animations)
    , vehicle_config(vehicle_config)
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

    ProcAnimation* animation = create_animation(name, transform);
    if (animation)
    {
    }

    transform.traverse(*this);
}

ProcAnimation* AnimTransformVisitor::create_animation(const std::string& name, vsg::MatrixTransform& transform)
{
    FileSystem& fs = FileSystem::getInstance();
    std::string data_dir = fs.getDataDir();
    std::string file_path = data_dir
        + fs.separator() + "animations"
        + fs.separator() + vehicle_config
        + fs.separator() + name + ".xml";

    try
    {
        ConfigReader cfg(file_path);
        auto config_section = cfg.getConfigSection();
        ProcAnimation* animation = nullptr;
        for (auto config_child : config_section.children())
        {
            std::string child_name = config_child.name();

            if (child_name == "AnalogRotation")
            {
                animation = new AnalogRotation(&transform);
                animation->load(cfg);
                return animation;
            }

            if (child_name == "AnalogTranslation")
            {
                animation = new AnalogTranslation(&transform);
                animation->load(cfg);
                return animation;
            }

            if (child_name == "MaterialAnimation")
            {
                std::string name;
                transform.getValue("name", name);
                std::cout << name << std::endl;
                MaterialAnimationVisitor mav(animations, &cfg);
                transform.accept(mav);
                std::cout << std::endl;
            }

            if (child_name == "MaterialRGBAnimation")
            {

            }
        }
    }
    catch (...)
    {
    }

    return nullptr;
}
