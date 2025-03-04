#pragma once

#include "Logger.h"
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorBuffer.h>

inline void print_node(vsg::ref_ptr<vsg::Node> node, int indentation = 0)
{
    std::string indent_symbol = "\t";
    std::string indent = "";
    for (int i = 0; i < indentation; ++i)
        indent += indent_symbol;

    std::string name;
    node->getValue("name", name);
    if (name.empty())
        node->getValue("Name", name);
    if (name.empty())
        name = "<EMPTY_NAME>";
    LOG_INFO("%s%s %s %x", indent.c_str(), node->className(), name.c_str(), node.get());

    if (auto* cull = node->cast<vsg::CullNode>())
    {
        print_node(cull->child, indentation + 1);
    }
    else if (auto* transform = node->cast<vsg::MatrixTransform>())
    {
        vsg::dmat4 m = transform->matrix;
        vsg::dvec3 t = vsg::dvec3(m(3, 0), m(3, 1), m(3, 2));
        vsg::dvec3 s = vsg::dvec3(vsg::length(vsg::dvec3(m(0, 0),   m(0, 1),    m(0, 2))),
                                  vsg::length(vsg::dvec3(m(1, 0),   m(1, 1),    m(1, 2))),
                                  vsg::length(vsg::dvec3(m(2, 0),   m(2, 1),    m(2, 2))));
        vsg::dmat3 r = vsg::dmat3(vsg::dvec3(m(0, 0) / s.x,   m(0, 1) / s.x,   m(0, 2) / s.x),
                                  vsg::dvec3(m(1, 0) / s.y,   m(1, 1) / s.y,   m(1, 2) / s.y),
                                  vsg::dvec3(m(2, 0) / s.z,   m(2, 1) / s.z,   m(2, 2) / s.z));
        vsg::dvec3 v = vsg::dvec3(0.0, 1.0, 0.0) * r;
        LOG_INFO("%stranslation {%.6f %.6f %.6f}"
                 , indent.c_str(),      t.x, t.y, t.z);
        LOG_INFO("%srotation&scale {0.0 1.0 0.0}&{1.0 1.0 1.0} -> {%.6f %.6f %.6f}&{%.6f %.6f %.6f})"
                 , indent.c_str(),      v.x, v.y, v.z,      s.x, s.y, s.z);
        LOG_INFO("%s{%.6f %.6f %.6f %.6f}", indent.c_str(), m(0, 0), m(1, 0), m(2, 0), m(3, 0));
        LOG_INFO("%s{%.6f %.6f %.6f %.6f}", indent.c_str(), m(0, 1), m(1, 1), m(2, 1), m(3, 1));
        LOG_INFO("%s{%.6f %.6f %.6f %.6f}", indent.c_str(), m(0, 2), m(1, 2), m(2, 2), m(3, 2));
        LOG_INFO("%s{%.6f %.6f %.6f %.6f}", indent.c_str(), m(0, 3), m(1, 3), m(2, 3), m(3, 3));
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
                    indent += indent_symbol + indent_symbol; // +2

                    descriptor->getValue("name", name);
                    if (name.empty())
                        descriptor->getValue("Name", name);
                    if (name.empty())
                        name = "<EMPTY_NAME>";
                    LOG_INFO("%s%s %s %x", indent.c_str(), descriptor->className(), name.c_str(), descriptor.get());

                    if (auto* descriptorBuffer = descriptor->cast<vsg::DescriptorBuffer>())
                    {
                        for (auto& child : descriptorBuffer->bufferInfoList)
                        {
                            indent += indent_symbol; // +3

                            child->getValue("name", name);
                            if (name.empty())
                                child->getValue("Name", name);
                            if (name.empty())
                                name = "<EMPTY_NAME>";
                            LOG_INFO("%s%s %s %x", indent.c_str(), child->className(), name.c_str(), child.get());

                            auto& data = child->data;
                            data->getValue("name", name);
                            if (name.empty())
                                data->getValue("Name", name);
                            if (name.empty())
                                name = "<EMPTY_NAME>";
                            LOG_INFO("%s%s %s %x", indent.c_str(), data->className(), name.c_str(), data.get());

                            // auto& buffer = child->buffer;
                            // buffer->getValue("name", name);
                            // if (name.empty())
                                // buffer->getValue("Name", name);
                            // if (name.empty())
                                // name = "<EMPTY_NAME>";
                            // LOG_INFO("%s%s %s %x", indent.c_str(), buffer->className(), name.c_str(), buffer.get());
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

