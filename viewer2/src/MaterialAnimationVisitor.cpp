#include "MaterialAnimationVisitor.h"
#include "ConfigReader.h"
#include "MaterialAnimation.h"
#include "ProcAnimation.h"
#include "animations-list.h"
#include <iostream>
#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/Buffer.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/material.h>

MaterialAnimationVisitor::MaterialAnimationVisitor(animations_t* animations, ConfigReader* cfg)
    :vsg::Visitor()
    , animations(animations)
    , cfg(cfg)
{
}

void MaterialAnimationVisitor::apply(vsg::Node& node)
{
   node.traverse(*this);
}

void MaterialAnimationVisitor::apply(vsg::StateGroup& stateGroup)
{
    std::cout << vsg::ref_ptr(&stateGroup) << std::endl;
    for (auto& command : stateGroup.stateCommands)
    {
        if (auto* bindDescriptorSet = command->cast<vsg::BindDescriptorSet>())
        {
            auto& descriptorSet = bindDescriptorSet->descriptorSet;
            for (auto& descriptor : descriptorSet->descriptors)
            {
                if (auto descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>())
                {
                    auto data = descriptorBuffer->bufferInfoList[0]->data;
                    auto& material = data->cast<vsg::PbrMaterialValue>()->value();

                    ProcAnimation* animation = new MaterialAnimation(material);
                    animation->load(*cfg);
                    animations->insert(animation->getSignalID(), animation);
                }
            }
        }
    }

    for (auto& command : stateGroup.stateCommands)
    {
        if (auto* bindDescriptorSet = command->cast<vsg::BindDescriptorSet>())
        {
            std::cout << "    " << command << std::endl;

            auto& descriptorSet = bindDescriptorSet->descriptorSet;
            std::cout << "        " << descriptorSet << std::endl;
            for (auto& descriptor : descriptorSet->descriptors)
            {
                std::cout << "            " << descriptor << std::endl;
                if (auto descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>())
                {
                    auto data = descriptorBuffer->bufferInfoList[0]->data;
                    std::cout << "                " << data << std::endl;

                    auto* v1 = data->cast<vsg::PbrMaterialValue>();

                    auto& material = data->cast<vsg::PbrMaterialValue>()->value();

                    // ProcAnimation* animation = new MaterialAnimation(material);
                    // animation->load(*cfg);
                    // animations->insert(animation->getSignalID(), animation);
                }
            }
        }
    }

    stateGroup.traverse(*this);
}
