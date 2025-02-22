#pragma once

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>

#include <iostream>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorBuffer.h>

inline void print_node(vsg::ref_ptr<vsg::Node> node, int indentation)
{
    std::string name;
    node->getValue("name", name);
    for (int i = 0; i < indentation; ++i)
    {
        std::cout << "    ";
    }
    std::cout << node->className() << ' ' << name << ' ' << node.get() << std::endl;

    if (auto* cull = node->cast<vsg::CullNode>())
    {
        print_node(cull->child, indentation + 1);
    }
    else if (auto* transform = node->cast<vsg::MatrixTransform>())
    {
        for (auto& child : transform->children)
        {
            print_node(child, indentation + 1);
        }
    }
    else if (auto* state_group = node->cast<vsg::StateGroup>())
    {
        for (auto& commands : state_group->stateCommands)
        {
            print_node(commands, indentation + 1);
            if (auto* bindDescriptorSet = commands->cast<vsg::BindDescriptorSet>())
            {
                auto& descriptorSet = bindDescriptorSet->descriptorSet;
                for (auto& descriptor : descriptorSet->descriptors)
                {
                    descriptor->getValue("name", name);
                    for (int i = 0; i < indentation + 2; ++i)
                    {
                        std::cout << "    ";
                    }
                    std::cout << descriptor->className() << ' ' << name << ' ' << descriptor.get() << std::endl;

                    if (auto* descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>())
                    {
                        for (auto& child : descriptorBuffer->bufferInfoList)
                        {
                            child->getValue("name", name);
                            for (int i = 0; i < indentation + 3; ++i)
                            {
                                std::cout << "    ";
                            }
                            std::cout << child->className() << ' ' << name << ' ' << child.get() << std::endl;
                            auto& data = child->data;
                            data->getValue("name", name);
                            for (int i = 0; i < indentation + 4; ++i)
                            {
                                std::cout << "    ";
                            }
                            std::cout << data->className() << ' ' << name << ' ' << data.get() << std::endl;
                            // auto& buffer = child->buffer;
                            // for (int i = 0; i < indentation + 4; ++i)
                            // {
                                // std::cout << "    ";
                            // }
                            // std::cout << buffer->className();
                            // std::cout << ' ' << name << ' ' << buffer.get() << std::endl;
                        }
                    }
                }
            }
        }
    }
    else if (auto* group = node->cast<vsg::Group>())
    {
        for (auto& child : group->children)
        {
            print_node(child, indentation + 1);
        }
    }
}

