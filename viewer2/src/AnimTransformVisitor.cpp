#include "AnimTransformVisitor.h"
#include "AnalogRotation.h"
#include "AnalogTranslation.h"
#include "ConfigReader.h"
#include "ProcAnimation.h"
#include "filesystem.h"
#include <iostream>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/MatrixTransform.h>

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
        std::cout << name << std::endl;
    }
    node.traverse(*this);
}

void AnimTransformVisitor::apply(vsg::Transform& transform)
{
    std::string name;
    if (transform.getValue("name", name))
    {
        std::cout << name << std::endl;
    }
    vsg::MatrixTransform* matrix_trans = static_cast<vsg::MatrixTransform*>(&transform);


    ProcAnimation* animation = create_animation(name, matrix_trans);

    if (animation)
    {
        animation->name = name;
        animations->insert(animation->getSignalID(), animation);
    }

    // traverse();
}

ProcAnimation* AnimTransformVisitor::create_animation(const std::string& name, vsg::MatrixTransform* transform)
{
    FileSystem& fs = FileSystem::getInstance();
    std::string data_dir = fs.getDataDir();
    std::string file_path = data_dir
        + fs.separator() + "animations"
        + fs.separator() + vehicle_config
        + fs.separator() + name + ".xml";

    // ConfigReader cfg(file_path);
    // auto config_section = cfg.getConfigSection();
    // for (auto child : config_section)
    // {
    //     ProcAnimation* animation = nullptr;
    //     std::string child_name = child.name();
    //     if (child_name == "AnalogRotation")
    //     {
    //         animation = new AnalogRotation(transform);
    //         animation->load(cfg);
    //         return animation;
    //     }
    //     else if (child_name == "AnalogTranslation")
    //     {
    //         animation = new AnalogTranslation(transform);
    //         animation->load(cfg);
    //         return animation;
    //     }
    //     else if (child_name == "MaterialAnimation")
    //     {

    //     }
    // }
    // return {};
    return nullptr;
}
