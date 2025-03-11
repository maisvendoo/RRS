#include "MaterialAnimationVisitor.h"
#include "CfgReader.h"
#include "MaterialAnimation.h"
#include "ProcAnimation.h"
#include "animations-list.h"
#include <iostream>
#include <vsg/core/Data.h>
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

MaterialAnimationVisitor::MaterialAnimationVisitor(animations_t* animations, CfgReader* cfg, vsg::ref_ptr<vsg::Node> parent_node)
    : vsg::Visitor()
    , animations(animations)
    , cfg(cfg)
    , parent_node(parent_node)
{
}

void MaterialAnimationVisitor::apply(vsg::Node& node)
{
    node.traverse(*this);
}

void MaterialAnimationVisitor::apply(vsg::MatrixTransform& mtransform)
{
    auto parent_group = vsg::ref_ptr(parent_node->cast<vsg::Group>());

    // if (auto transform = vsg::ref_ptr())
    if (auto transform = vsg::MatrixTransform::create(mtransform))
    {
        // parent_group->children = {transform};
        parent_group->children[0] = transform;
        if (auto group = vsg::ref_ptr(transform->children[0]->clone()->cast<vsg::Group>()))
        {
            // transform->children = {group};
            transform->children[0] = group;
            if (auto state_group = vsg::ref_ptr(group->children[0]->clone()->cast<vsg::StateGroup>()))
            {
                std::cout << parent_node << std::endl;
                // group->children = {state_group};
                group->children[0] = state_group;
                for (auto& commands : state_group->stateCommands)
                {
                    if (auto bind_descriptor_set = vsg::ref_ptr(commands->clone()->cast<vsg::BindDescriptorSet>()))
                    {
                        commands = bind_descriptor_set;
                        bind_descriptor_set->descriptorSet = vsg::ref_ptr(bind_descriptor_set->descriptorSet->clone()->cast<vsg::DescriptorSet>());
                        for (auto& descriptor : bind_descriptor_set->descriptorSet->descriptors)
                        {
                            if (auto descriptor_buffer = vsg::ref_ptr(descriptor->clone()->cast<vsg::DescriptorBuffer>()))
                            {
                                descriptor = descriptor_buffer;

                                auto material = vsg::ref_ptr(descriptor_buffer->bufferInfoList[0]->data->clone()->cast<vsg::PbrMaterialValue>());
                                auto buffer_info = vsg::BufferInfo::create(material);
                                descriptor_buffer->bufferInfoList = {buffer_info};

                                ProcAnimation *animation = new MaterialAnimation(material);
                                animation->load(*cfg);
                                animations->insert({animation->getSignalID(), animation});
                            }
                        }
                    }
                }
            }
        }
    }

    mtransform.traverse(*this);
}
