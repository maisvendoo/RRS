#include "MaterialAnimationVisitor.h"
#include "ConfigReader.h"
#include "animations-list.h"
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
   node.traverse(*this);
}

void MaterialAnimationVisitor::apply(vsg::StateGroup& stateGroup)
{
    for (auto& command : stateGroup.stateCommands)
    {
        if (auto bindDescriptorSets = command->cast<vsg::BindDescriptorSets>())
        {
            for (auto& descriptorSet : bindDescriptorSets->descriptorSets)
            {
                for (auto& descriptor : descriptorSet->descriptors)
                {
                    if (auto descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>())
                    {
                        auto data = descriptorBuffer->bufferInfoList[0]->data;
                        auto material = reinterpret_cast<vsg::material*>(data.get());
                    }
                }
            }
        }
    }
}
