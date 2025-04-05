#include "MyGui.h"

#include "VehiclesHandler.h"

#include <vsgImGui/imgui.h>

#include "cmake_defines.h"
#include "vsg/app/Viewer.h"
#include <vsg/io/Options.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>

std::vector<vsg::ref_ptr<vsg::Node>> GUIParams::nodes;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MyGui::MyGui(vsg::ref_ptr<GUIParams> in_params, vsg::ref_ptr<vsg::Options> options)
    : params(in_params)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF(FONT_PATH, font_size);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::compile(vsg::Context& context)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::record(vsg::CommandBuffer& cb) const
{
    bool is_modified_key = ImGui::IsKeyPressed(ImGuiKey_LeftShift) ||
                           ImGui::IsKeyPressed(ImGuiKey_RightShift) ||
                           ImGui::IsKeyPressed(ImGuiKey_LeftCtrl) ||
                           ImGui::IsKeyPressed(ImGuiKey_RightCtrl) ||
                           ImGui::IsKeyPressed(ImGuiKey_LeftAlt) ||
                           ImGui::IsKeyPressed(ImGuiKey_RightAlt);

    // Подтверждение выхода по Esc
    if (ImGui::IsKeyPressed(ImGuiKey_Escape) && !params->prev_Esc)
    {
        params->is_show_quit_dialog = !params->is_show_quit_dialog;
    }
    params->prev_Esc = ImGui::IsKeyPressed(ImGuiKey_Escape);

    if (params->is_show_quit_dialog)
    {
        showQuitDialog();
    }

    // Отображение статистики
    if (ImGui::IsKeyPressed(ImGuiKey_F11) && !params->prev_F11 && !is_modified_key)
    {
        params->is_show_statistics = !params->is_show_statistics;
    }
    params->prev_F11 = ImGui::IsKeyPressed(ImGuiKey_F11);

    if (params->is_show_statistics)
    {
        showStatistics();
    }

    if (params->vehicles_handler)
    {
        // Отображение дебаг-строки подвижного состава
        if (ImGui::IsKeyPressed(ImGuiKey_F9) && !params->prev_F9)
        {
            params->is_show_debug_msg = !params->is_show_debug_msg;
        }
        params->prev_F9 = ImGui::IsKeyPressed(ImGuiKey_F9);

        // Строка нажмите Enter для управления
        params->is_no_controlled =
                        (params->vehicles_handler->getCurrentVehicleIndex() !=
                        params->vehicles_handler->getControlledVehicleIndex());
    }
    else
    {
        params->prev_F9 = false;
        params->is_show_debug_msg = false;
        params->is_no_controlled = false;
    }

    if (params->is_show_debug_msg)
    {
        showDebugMsg();
    }

    if (params->is_no_controlled)
    {
        showNoControlled();
    }

    // Демо ImGui
    if (ImGui::IsKeyPressed(ImGuiKey_F10) && !params->prev_F10 && !is_modified_key)
    {
        params->showDemoWindow = !params->showDemoWindow;
    }
    params->prev_F10 = ImGui::IsKeyPressed(ImGuiKey_F10);
    if (params->showDemoWindow)
    {
        ImGui::ShowDemoWindow();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showQuitDialog() const
{
    int w = 400;
    int h = 150;

    int cx = w / 2;
    int cy = h / 2;

    ImGui::SetNextWindowSize(ImVec2(w, h));

    ImGuiIO &io = ImGui::GetIO();

    ImVec2 content_size = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2( (content_size.x - w) / 2, (content_size.y - h) / 2));

    ImGui::Begin(u8"Вы действительно хотите выйти?");

    int bw = w / 4;
    int bh = h / 4;

    ImGui::SetCursorPos(ImVec2(cx - 3 * bw / 2, cy - bh / 2));
    if (ImGui::Button(u8"Да", ImVec2(bw, bh)))
    {
        ImGui::SetCursorPos(ImVec2(cx, cy));
        vsg::ref_ptr<vsg::Viewer> viewer = params->viewer;
        if (viewer)
            viewer->close();
    }

    ImGui::SetCursorPos(ImVec2(cx + bw / 2, cy - bh / 2));
    if (ImGui::Button(u8"Нет", ImVec2(bw, bh)))
    {
        ImGui::SetCursorPos(ImVec2(cx, cy));
        params->is_show_quit_dialog = false;
    }

    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showStatistics() const
{
    ImGui::Begin("Statistics");

    for (const auto& node : params->nodes)
    {
        printObject(node);
    }

    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showDebugMsg() const
{
    QString debugMsg = params->vehicles_handler->getDebugMsg();
    QStringList lines = debugMsg.split('\n');
    float h = font_size * (lines.count() + 1);

    ImGuiIO &io = ImGui::GetIO();
    ImVec2 content_size = io.DisplaySize;

    ImGui::SetNextWindowSize(ImVec2(content_size.x, h));
    ImGui::SetNextWindowPos(ImVec2(0, content_size.y - h));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Консоль отладки", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::Text(u8"%s", debugMsg.toStdString().c_str());
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::showNoControlled() const
{
    const char *text = "Нажмите Enter для управления данной ПЕ";
    ImVec2 text_size = ImGui::CalcTextSize(text);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(text_size.x + 20, text_size.y + 20));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Состояние управления", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    ImGui::Text(u8"%s", text);
    ImGui::PopStyleColor();
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyGui::printObject(const vsg::ref_ptr<vsg::Object>& object) const
{
    if (!object)
    {
        return;
    }

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
