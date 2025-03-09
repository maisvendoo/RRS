#pragma once

#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>

#include <iostream>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/ViewDependentState.h>

inline void print_object(vsg::ref_ptr<vsg::Object> object, int indentation = 0)
{
    std::string name;
    object->getValue("name", name);
    if (name.empty())
        object->getValue("Name", name);
    if (name.empty())
        name = "<EMPTY_NAME>";

    for (int i = 0; i < indentation; ++i)
    {
        std::cout << "  ";
    }
    std::cout << "--> " << object->className() << ' ' << name << ' ' << object.get() << std::endl;

    if (auto* cull = object->cast<vsg::CullNode>())
    {
        print_object(cull->child, indentation + 1);
    }
    else if (auto* transform = object->cast<vsg::MatrixTransform>())
    {
        for (auto& child : transform->children)
        {
            print_object(child, indentation + 1);
        }
    }
    else if (auto* bind_graphics_pipeline = object->cast<vsg::BindGraphicsPipeline>())
    {
        for (auto& state : bind_graphics_pipeline->pipeline->pipelineStates)
        {
            print_object(state, indentation + 1);
        }
    }
    else if (auto* bind_descriptor_set = object->cast<vsg::BindDescriptorSet>())
    {
        for (auto& descriptor : bind_descriptor_set->descriptorSet->descriptors)
        {
            print_object(descriptor, indentation + 1);
        }
    }
    else if (auto* buffer_info = object->cast<vsg::BufferInfo>())
    {
        print_object(buffer_info->data, indentation + 1);
    }
    else if (auto* descriptor_buffer = object->cast<vsg::DescriptorBuffer>())
    {
        for (auto& bufferInfo : descriptor_buffer->bufferInfoList)
        {
            print_object(bufferInfo, indentation + 1);
        }
    }
    else if (auto* descriptor = object->cast<vsg::Descriptor>())
    {
        // if (auto* descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>())
        // {
        //     for (auto& child : descriptorBuffer->bufferInfoList)
        //     {
        //         child->getValue("name", name);
        //         for (int i = 0; i < indentation + 3; ++i)
        //         {
        //             std::cout << "  ";
        //         }
        //         std::cout << child->className() << ' ' << name << ' ' << child.get() << std::endl;
        //         auto& data = child->data;
        //         data->getValue("name", name);
        //         for (int i = 0; i < indentation + 4; ++i)
        //         {
        //             std::cout << "  ";
        //         }
        //         std::cout << data->className() << ' ' << name << ' ' << data.get() << std::endl;
        //         // auto& buffer = child->buffer;
        //         // for (int i = 0; i < indentation + 4; ++i)
        //         // {
        //             // std::cout << "\t";
        //         // }
        //         // std::cout << buffer->className();
        //         // std::cout << ' ' << name << ' ' << buffer.get() << std::endl;
        //     }
        // }
    }
    else if (auto* state_group = object->cast<vsg::StateGroup>())
    {
        for (auto& commands : state_group->stateCommands)
        {
            print_object(commands, indentation + 1);
        }
    }
    else if (auto* group = object->cast<vsg::Group>())
    {
        for (auto& child : group->children)
        {
            print_object(child, indentation + 1);
        }
    }
}

