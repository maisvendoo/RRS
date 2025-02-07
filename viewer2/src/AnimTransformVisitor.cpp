#include "AnimTransformVisitor.h"
#include "ConfigReader.h"
#include "ProcAnimation.h"
#include "filesystem.h"
#include <vsg/core/Visitor.h>
#include <vsg/nodes/MatrixTransform.h>

AnimTransformVisitor::AnimTransformVisitor(animations_t* animations, const std::string& vehicle_config)
    : vsg::Visitor()
    , animations(animations)
    , vehicle_config(vehicle_config)
{
}

void AnimTransformVisitor::apply(vsg::Transform& transform)
{
    vsg::MatrixTransform* matrix_trans = static_cast<vsg::MatrixTransform*>(&transform);
    std::string name;
    matrix_trans->getValue("name", name);

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

    ConfigReader cfg(file_path);
    auto config_section = cfg.getConfigSection();
    for (auto child : config_section)
    {

    }
    return {};
}
