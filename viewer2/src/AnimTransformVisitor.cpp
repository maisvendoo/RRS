#include "AnimTransformVisitor.h"
#include "AnalogRotation.h"
#include "AnalogTranslation.h"
#include "CfgReader.h"
#include "MaterialAnimationVisitor.h"
#include "ProcAnimation.h"
#include "filesystem.h"
#include "helper.h"
#include <cstring>
#include <iostream>
#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
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
    std::string name = "";
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

ProcAnimation* AnimTransformVisitor::create_animation(const std::string& name, vsg::MatrixTransform& transform)
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
            animation = new AnalogRotation(&transform);
            animation->load(cfg);
            return animation;
        }

        config_section = cfg.getFirstSection("AnalogTranslation");
        if (!config_section.isNull())
        {
            animation = new AnalogTranslation(&transform);
            animation->load(cfg);
            return animation;
        }

        config_section = cfg.getFirstSection("MaterialAnimation");
        if (!config_section.isNull())
        {
            MaterialAnimationVisitor mav(animations, &cfg);
            transform.accept(mav);
            return nullptr;
        }

        config_section = cfg.getFirstSection("MaterialRGBAnimation");
        if (!config_section.isNull())
        {
            return nullptr;
        }
    }

    return nullptr;
}
