#include "MaterialAnimationVisitor.h"
#include "ConfigReader.h"
#include "animations-list.h"
#include <iostream>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/material.h>

MaterialAnimationVisitor::MaterialAnimationVisitor(animations_t* animations, ConfigReader* cfg)
    :vsg::Visitor()
    , animations(animations)
    , cfg(cfg)
{
}

void MaterialAnimationVisitor::apply(vsg::Node& node)
{
//     std::string name;
//     node.getValue("name", name);
//     std::cout << node.className() << ' ' << name << std::endl;

   node.traverse(*this);
}

void MaterialAnimationVisitor::apply(vsg::StateGroup& stateGroup)
{
    std::string name;
    stateGroup.getValue("name", name);
    std::cout << stateGroup.className() << ' ' << name << std::endl;
    std::cout << "stateCommands count: " << stateGroup.stateCommands.size() << std::endl;
    for (auto& command : stateGroup.stateCommands)
    {
        command->getValue("name", name);
        std::cout << "    " << command->className() << ' ' << name << std::endl;
    }
    std::cout << std::endl;

    for (auto& command : stateGroup.stateCommands)
    {
        if (auto bindDescriptorSet = command->cast<vsg::BindDescriptorSet>())
        {
            auto& descriptorSet = bindDescriptorSet->descriptorSet;
            for (auto& descriptor : descriptorSet->descriptors)
            {
                if (auto descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>())
                {
                    auto data = descriptorBuffer->bufferInfoList[0]->data;
                    auto material = reinterpret_cast<vsg::material*>(data.get());
                    vsg::material b = *material;
                    int a = 10;
                }
            }
        }
    }

    stateGroup.traverse(*this);
}
