#include "AnimTransformVisitor.h"

#include "AnalogRotation.h"
#include "AnalogTranslation.h"
#include "MaterialAnimationVisitor.h"
#include "animations-list.h"
#include "CfgReader.h"
#include "filesystem.h"
#include "ProcAnimation.h"

#include <vsg/core/Auxiliary.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>

#include <string>
#include <vsg/utils/PropagateDynamicObjects.h>
#include <vsg/utils/SharedObjects.h>

AnimTransformVisitor::AnimTransformVisitor(animations_t* animations, const std::string& vehicle_config, vsg::ref_ptr<vsg::MatrixTransform>& root_node, vsg::ref_ptr<vsg::Options> options)
    : animations(animations)
    , vehicle_config(vehicle_config)
    , root_node(root_node)
    , options(options)
{
}

void AnimTransformVisitor::apply(vsg::Node& node)
{
    node.traverse(*this);
}

void AnimTransformVisitor::apply(vsg::MatrixTransform& transform)
{
    auto transform_ptr = vsg::ref_ptr(&transform);

    std::string name = "";
    transform_ptr->getValue("name", name);

    if (name.empty())
    {
        transform_ptr->getValue("Name", name);
    }

    if (name.empty())
    {
        transform.traverse(*this);
        return;
    }

    ProcAnimation* animation = create_animation(name, transform_ptr);
    if (animation)
    {
        animation->name = name;
        animations->insert({animation->getSignalID(), animation});
    }

    transform.traverse(*this);
}

ProcAnimation* AnimTransformVisitor::create_animation(const std::string& name, vsg::ref_ptr<vsg::MatrixTransform> transform)
{
    FileSystem& fs = FileSystem::getInstance();
    std::string data_dir = fs.getDataDir();
    std::string file_path = data_dir
        + fs.separator() + "animations"
        + fs.separator() + vehicle_config
        + fs.separator() + name + ".xml";

    QString tmp_qstr = file_path.c_str();
    CfgReader cfg;
    if (cfg.load(tmp_qstr))
    {
        QDomNode config_section;
        ProcAnimation* animation = nullptr;

        config_section = cfg.getFirstSection("AnalogRotation");
        if (!config_section.isNull())
        {
            copy_nodes(transform);
            animation = new AnalogRotation(transform);
            animation->load(cfg);
            return animation;
        }

        config_section = cfg.getFirstSection("AnalogTranslation");
        if (!config_section.isNull())
        {
            copy_nodes(transform);
            animation = new AnalogTranslation(transform);
            animation->load(cfg);
            return animation;
        }

        config_section = cfg.getFirstSection("MaterialAnimation");
        if (!config_section.isNull())
        {
            copy_nodes(transform);
            MaterialAnimationVisitor mav(animations, &cfg);
            transform->accept(mav);
            return nullptr;
        }

        config_section = cfg.getFirstSection("MaterialRGBAnimation");
        if (!config_section.isNull())
        {
            copy_nodes(transform);
            return nullptr;
        }
    }

    return nullptr;
}

void AnimTransformVisitor::copy_nodes(vsg::ref_ptr<vsg::MatrixTransform>& transform)
{
    options->propagateDynamicObjects->dynamicObjects.clear();
    options->propagateDynamicObjects->tag(transform);
    root_node->accept(*options->propagateDynamicObjects);

    vsg::CopyOp copyop;
    auto duplicate = copyop.duplicate = new vsg::Duplicate;
    for (auto& object : options->propagateDynamicObjects->dynamicObjects)
    {
        duplicate->insert(object);
    }

    transform = copyop(transform);
    root_node = copyop(root_node);
}
