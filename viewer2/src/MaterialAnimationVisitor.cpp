#include "MaterialAnimationVisitor.h"
#include "CfgReader.h"
#include "MaterialAnimation.h"
#include "ProcAnimation.h"
#include "animations-list.h"

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
#include <vsg/utils/PropagateDynamicObjects.h>

MaterialAnimationVisitor::MaterialAnimationVisitor(animations_t* animations, CfgReader* cfg, vsg::ref_ptr<vsg::Options> options, vsg::ref_ptr<vsg::MatrixTransform>& root_node)
    : vsg::Visitor()
    , options(options)
    , root_node(root_node)
    , animations(animations)
    , cfg(cfg)
{
}

void MaterialAnimationVisitor::apply(vsg::Node& node)
{
    node.traverse(*this);
}

// void MaterialAnimationVisitor::apply(vsg::BindDescriptorSet& bindDescriptorSet)
// {
//     for (auto& descriptor : bindDescriptorSet.descriptorSet->descriptors)
//     {
//         if (auto descriptorBuffer = descriptor.cast<vsg::DescriptorBuffer>())
//         {
//             for (auto& bufferInfo : descriptorBuffer->bufferInfoList)
//             {
//                 if (bufferInfo->data->cast<vsg::PbrMaterialValue>())
//                 {
//                     auto pbrValue = (vsg::ref_ptr<vsg::PbrMaterialValue>*)(&bufferInfo->data);
//                     (*pbrValue)->properties.dataVariance = vsg::DYNAMIC_DATA_TRANSFER_AFTER_RECORD;

//                     std::scoped_lock<std::mutex> pdo_lock(options->propagateDynamicObjects->mutex);
//                     options->propagateDynamicObjects->dynamicObjects.clear();
//                     options->propagateDynamicObjects->tag((*pbrValue));
//                     root_node->accept(*options->propagateDynamicObjects);
                
//                     vsg::CopyOp copyop;
//                     auto duplicate = copyop.duplicate = new vsg::Duplicate;
//                     for (auto& object : options->propagateDynamicObjects->dynamicObjects)
//                     {
//                         duplicate->insert(object);
//                     }
                
//                     *pbrValue = copyop(*pbrValue);
//                     root_node = copyop(root_node);

//                     ProcAnimation *animation = new MaterialAnimation((*pbrValue));
//                     animation->load(*cfg);
//                     animations->insert({animation->getSignalID(), animation});
//                 }
//             }
//         }
//     }
// }

void MaterialAnimationVisitor::apply(vsg::MatrixTransform& transform)
{
    auto transform_ptr = vsg::ref_ptr(&transform);

    if (auto group = vsg::ref_ptr(transform_ptr->children[0]->clone()->cast<vsg::Group>()))
    {
        transform_ptr->children = {group};
        if (auto state_group = vsg::ref_ptr(group->children[0]->clone()->cast<vsg::StateGroup>()))
        {
            group->children = {state_group};
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
                            material->properties.dataVariance = vsg::DYNAMIC_DATA_TRANSFER_AFTER_RECORD;

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

    transform.traverse(*this);
}
