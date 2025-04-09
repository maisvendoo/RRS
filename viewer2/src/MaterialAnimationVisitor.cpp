#include "MaterialAnimationVisitor.h"
#include "CfgReader.h"
#include "MaterialAnimation.h"
#include "ProcAnimation.h"
#include "animations-list.h"

#include <iostream>
#include <mutex>
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

void MaterialAnimationVisitor::apply(vsg::BindDescriptorSet& bindDescriptorSet)
{
    for (auto& descriptor : bindDescriptorSet.descriptorSet->descriptors)
    {
        if (auto* descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>())
        {
            for (auto& bufferInfo : descriptorBuffer->bufferInfoList)
            {
                if (auto* pbrMaterialValue = bufferInfo->data->cast<vsg::PbrMaterialValue>())
                {
                    pbrMaterialValue->properties.dataVariance = vsg::DYNAMIC_DATA_TRANSFER_AFTER_RECORD;

                    std::scoped_lock<std::mutex> pdo_lock(options->propagateDynamicObjects->mutex);
                    options->propagateDynamicObjects->dynamicObjects.clear();
                    options->propagateDynamicObjects->tag(pbrMaterialValue);
                    root_node->accept(*options->propagateDynamicObjects);

                    std::cout << "\nDynamic:\n";
                    for (auto& object : options->propagateDynamicObjects->dynamicObjects)
                    {
                        std::cout << object->className() << std::endl;
                    }

                    vsg::CopyOp copyop;
                    auto duplicate = copyop.duplicate = new vsg::Duplicate;
                    for (auto& object : options->propagateDynamicObjects->dynamicObjects)
                    {
                        duplicate->insert(object);
                    }

                    root_node = copyop(root_node);

                    std::cout << "\n\n";
                    for (auto& [oldo, newo] : duplicate->duplicates)
                    {
                        std::cout << "Old: " << oldo << std::endl;
                        if (newo)
                        {
                            std::cout << "New: " << newo;
                            std::string name;
                            newo->getValue("name", name);
                            if (!name.empty()) std::cout << ": " << name;
                            std::cout << std::endl;
                        }
                    }
                    std::cout << "Material ptr: " << pbrMaterialValue << std::endl;

                    vsg::ref_ptr<vsg::PbrMaterialValue> a;
                    if (duplicate->duplicates[pbrMaterialValue])
                    {
                        a = vsg::ref_ptr(duplicate->duplicates[pbrMaterialValue]->cast<vsg::PbrMaterialValue>());
                    }
                    else
                    {
                        a = vsg::ref_ptr(pbrMaterialValue);
                    }
                    std::cout << "a = " << a << std::endl;
                    ProcAnimation *animation = new MaterialAnimation(a);
                    animation->load(*cfg);
                    animations->insert({animation->getSignalID(), animation});
                }
            }
        }
    }
    // for (auto& descriptor : bindDescriptorSet.descriptorSet->descriptors)
    // {
    //     if (auto descriptorBuffer = descriptor.cast<vsg::DescriptorBuffer>())
    //     {
    //         for (auto& bufferInfo : descriptorBuffer->bufferInfoList)
    //         {
    //             if (bufferInfo->data->cast<vsg::PbrMaterialValue>())
    //             {
    //                 auto* pbrValue = (vsg::ref_ptr<vsg::PbrMaterialValue>*)(&bufferInfo->data);
    //                 (*pbrValue)->properties.dataVariance = vsg::DYNAMIC_DATA_TRANSFER_AFTER_RECORD;

    //                 std::scoped_lock<std::mutex> pdo_lock(options->propagateDynamicObjects->mutex);
    //                 options->propagateDynamicObjects->dynamicObjects.clear();
    //                 options->propagateDynamicObjects->tag(*pbrValue);
    //                 root_node->accept(*options->propagateDynamicObjects);
                
    //                 vsg::CopyOp copyop;
    //                 auto duplicate = copyop.duplicate = new vsg::Duplicate;
    //                 for (auto& object : options->propagateDynamicObjects->dynamicObjects)
    //                 {
    //                     duplicate->insert(object);
    //                 }
                
    //                 *pbrValue = copyop(*pbrValue);
    //                 root_node = copyop(root_node);

    //                 ProcAnimation *animation = new MaterialAnimation(*pbrValue);
    //                 animation->load(*cfg);
    //                 animations->insert({animation->getSignalID(), animation});
    //             }
    //         }
    //     }
    // }
}

// void MaterialAnimationVisitor::apply(vsg::MatrixTransform& transform)
// {
//     auto transform_ptr = vsg::ref_ptr(&transform);

//     if (auto group = vsg::ref_ptr(transform_ptr->children[0]->clone()->cast<vsg::Group>()))
//     {
//         transform_ptr->children = {group};
//         if (auto state_group = vsg::ref_ptr(group->children[0]->clone()->cast<vsg::StateGroup>()))
//         {
//             group->children = {state_group};
//             for (auto& commands : state_group->stateCommands)
//             {
//                 if (auto bind_descriptor_set = vsg::ref_ptr(commands->clone()->cast<vsg::BindDescriptorSet>()))
//                 {
//                     commands = bind_descriptor_set;
//                     bind_descriptor_set->descriptorSet = vsg::ref_ptr(bind_descriptor_set->descriptorSet->clone()->cast<vsg::DescriptorSet>());
//                     for (auto& descriptor : bind_descriptor_set->descriptorSet->descriptors)
//                     {
//                         if (auto descriptor_buffer = vsg::ref_ptr(descriptor->clone()->cast<vsg::DescriptorBuffer>()))
//                         {
//                             descriptor = descriptor_buffer;

//                             auto material = vsg::ref_ptr(descriptor_buffer->bufferInfoList[0]->data->clone()->cast<vsg::PbrMaterialValue>());
//                             material->properties.dataVariance = vsg::DYNAMIC_DATA_TRANSFER_AFTER_RECORD;

//                             auto buffer_info = vsg::BufferInfo::create(material);
//                             descriptor_buffer->bufferInfoList = {buffer_info};

//                             ProcAnimation *animation = new MaterialAnimation(material);
//                             animation->load(*cfg);
//                             animations->insert({animation->getSignalID(), animation});
//                         }
//                     }
//                 }
//             }
//         }
//     }

//     transform.traverse(*this);
// }
