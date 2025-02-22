#include "MaterialAnimationVisitor.h"
#include "CfgReader.h"
#include "MaterialAnimation.h"
#include "ProcAnimation.h"
#include "animations-list.h"
#include "helper.h"
#include <iostream>
#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/Buffer.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/StateCommand.h>
#include <vsg/state/material.h>
#include <vsg/vk/State.h>

MaterialAnimationVisitor::MaterialAnimationVisitor(animations_t* animations, CfgReader* cfg)
    : vsg::Visitor()
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
        if (auto* bindDescriptorSet = command->cast<vsg::BindDescriptorSet>())
        {
            command = bindDescriptorSet->clone()->cast<vsg::BindDescriptorSet>();
            bindDescriptorSet = command->cast<vsg::BindDescriptorSet>();

            vsg::ref_ptr<vsg::DescriptorSet> descriptorSet(bindDescriptorSet->descriptorSet->clone()->cast<vsg::DescriptorSet>());
            bindDescriptorSet->descriptorSet = descriptorSet;

            for (auto& descriptor : descriptorSet->descriptors)
            {
                if (auto* descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>())
                {
                    descriptor = descriptorBuffer->clone()->cast<vsg::DescriptorBuffer>();
                    descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>();

                    auto material_value = vsg::PbrMaterialValue::create();
                    auto bufferInfo = vsg::BufferInfo::create(material_value);
                    descriptorBuffer->bufferInfoList.clear();
                    descriptorBuffer->bufferInfoList.push_back(bufferInfo);

                    auto& material = material_value->value();

                    ProcAnimation *animation = new MaterialAnimation(material);
                    animation->load(*cfg);
                    animations->insert(animation->getSignalID(), animation);
                }
            }
        }
    }
    // print_node(vsg::ref_ptr(&stateGroup), 0);
    stateGroup.traverse(*this);
}
