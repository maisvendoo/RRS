#include "MyGui.h"

#include <vsg/nodes/PagedLOD.h>
#include <vsgImGui/imgui.h>

#include <vsg/commands/Command.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>


MyGui::MyGui(vsg::ref_ptr<Params> in_params, vsg::ref_ptr<vsg::Options> options)
    : params(in_params)
{
}

void MyGui::compile(vsg::Context& context)
{
}

void MyGui::record(vsg::CommandBuffer& cb) const
{
    if (params->showGui)
    {
        if (ImGui::Begin("Statistics"))
        {
            // if (ImGui::TreeNode("Route PagedLODs"))
            // {
            //     if (params->route_models.empty())
            //     {
            //         ImGui::Text("Empty");
            //     }
            //     else
            //     {
            //         if (ImGui::BeginTable("PagedLODs", 2))
            //         {
            //             for (const auto node : params->route_models)
            //             {
            //                 vsg::ref_ptr<vsg::PagedLOD> pagedLod = vsg::ref_ptr(node->cast<vsg::PagedLOD>());
            //                 ImGui::TableNextRow();
            //                 ImGui::TableNextColumn();
            //                 ImGui::Text("%s", pagedLod->filename.c_str());
            //                 ImGui::TableNextColumn();
            //                 // ImGui::Text("%s", std::to_string(pagedLod->requestStatus).c_str());
            //             }

            //             ImGui::EndTable();
            //         }
            //     }

            //     ImGui::TreePop();
            // }

            ImGui::End();
        }
    }

    if (params->showDemoWindow)
    {
        ImGui::ShowDemoWindow();
    }
}
