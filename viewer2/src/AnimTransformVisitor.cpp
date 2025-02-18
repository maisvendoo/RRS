#include "AnimTransformVisitor.h"
#include "AnalogRotation.h"
#include "AnalogTranslation.h"
#include "ConfigReader.h"
#include "ProcAnimation.h"
#include "filesystem.h"
#include <iostream>
#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>
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
    std::string name;
    if (node.getValue("name", name))
    {
        // std::cout << "Node: " << name << "    Class: " << node.className() << std::endl;
    }

    node.traverse(*this);
}

void AnimTransformVisitor::apply(vsg::MatrixTransform& transform)
{
    std::cout << "Class: " << transform.className() << std::endl;
    for (auto child : transform.children)
    {
        std::cout << "    " << child->className() << std::endl;
    }

    // std::string name;
    // transform.children[0]->getValue("name", name);
    // transform.setValue("name", name);
    // std::cout << "Node: " << name << "    Class: " << transform.className() << std::endl;

    transform.traverse(*this);
}

void AnimTransformVisitor::apply(vsg::Group& group)
{
    std::string name;
    if (group.getValue("name", name))
    {
        // std::cout << "Node: " << name << "    Class: " << group.className() << std::endl;
    }

    group.traverse(*this);
}

ProcAnimation* AnimTransformVisitor::create_animation(const std::string& name, vsg::Node& node)
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
                // animation = new AnalogRotation(node);
            }
        }
    }
    catch (...)
    {
    }

    return nullptr;
}
