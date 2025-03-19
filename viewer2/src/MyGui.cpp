#include "MyGui.h"

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
        ImGui::Begin("Parameters");
        ImGui::Text("Test text");
        ImGui::End();
    }
}
