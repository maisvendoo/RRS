#include "MyGui.h"

#include "cmake_defines.h"

#include <vsgImGui/imgui.h>

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>

#include <vector>

std::vector<vsg::ref_ptr<vsg::Node>> Params::nodes;

Params::Params()
    : showGui(true)
    , showDemoWindow(false)
{
}

MyGui::MyGui(vsg::ref_ptr<Params> in_params, vsg::ref_ptr<vsg::Options> options)
    : params(in_params)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF(FONT_PATH, 16.0f);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
}

void MyGui::compile(vsg::Context& context)
{
}

void MyGui::record(vsg::CommandBuffer& cb) const
{
    if (params->showGui)
    {
        ImGui::Begin("Statistics");

        for (const auto& node : params->nodes)
        {
            printObject(node);
        }

        ImGui::End();
    }

    if (params->showDemoWindow)
    {
        ImGui::ShowDemoWindow();
    }
}

void MyGui::printObject(const vsg::ref_ptr<vsg::Object>& object) const
{
    std::string name;
    object->getValue("name", name);
    if (name.empty())
    {
        object->getValue("Name", name);
    }

    ImGuiID id = ImGui::GetID(object.get());

    ImGui::PushStyleColor(0, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
    bool show_tree_node = ImGui::TreeNodeEx(object.get(),
        ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth, "%s", object->className());
    ImGui::PopStyleColor();

    if (!name.empty())
    {
        ImGui::SameLine();
        ImGui::Text("%s", name.c_str());
    }

    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%p", object.get());

    if (show_tree_node)
    {
        ImGui::PopID();

        if (auto transform = vsg::ref_ptr(object->cast<vsg::MatrixTransform>()))
        {
            for (const auto& child : transform->children)
            {
                printObject(child);
            }
        }
        else if (auto cullNode = vsg::ref_ptr(object->cast<vsg::CullNode>()))
        {
            printObject(cullNode->child);
        }
        else if (auto stateGroup = vsg::ref_ptr(object->cast<vsg::StateGroup>()))
        {
            for (const auto& command : stateGroup->stateCommands)
            {
                printObject(command);
            }
        }
        else if (auto group = vsg::ref_ptr(object->cast<vsg::Group>()))
        {
            for (const auto& child : group->children)
            {
                printObject(child);
            }
        }
        else if (auto bindDescriptorSet = vsg::ref_ptr(object->cast<vsg::BindDescriptorSet>()))
        {
            for (const auto& descriptor : bindDescriptorSet->descriptorSet->descriptors)
            {
                printObject(descriptor);
            }
        }
        else if (auto descriptorBuffer = vsg::ref_ptr(object->cast<vsg::DescriptorBuffer>()))
        {
            for (const auto& bufferInfo : descriptorBuffer->bufferInfoList)
            {
                printObject(bufferInfo);
            }
        }
        else if (auto bufferInfo = vsg::ref_ptr(object->cast<vsg::BufferInfo>()))
        {
            printObject(bufferInfo->data);
        }

        ImGui::PushID(id);
        ImGui::TreePop();
    }
}
