#include "editor/states/BoxSelectionState.h"

#include "editor/Mouse.h"
#include "editor/StateManager.h"

#include <graphics/pipeline_funcs.h>

#include <filesystem.h>

#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/core/Value.h>
#include <vsg/io/Options.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/ui/PointerEvent.h>
#include <vsgImGui/imgui.h>

#include <vulkan/vulkan_core.h>

#include <string>

BoxSelectionState::BoxSelectionState(
    const vsg::ref_ptr<Mouse>& mouse,
    StateManager& state_manager,
    const vsg::ref_ptr<vsg::Options>& vsg_options
)
    : mouse(mouse)
    , state_manager(state_manager)
    , vsg_options(vsg_options)
{
    const FileSystem& fs = FileSystem::getInstance();
    const std::string shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto translation = vsg::vec2Value::create(0.0f, 0.0f);
    translation->properties.dataVariance = vsg::DYNAMIC_DATA;
    const auto scale = vsg::vec2Value::create(1.0f, 1.0f);
    scale->properties.dataVariance = vsg::DYNAMIC_DATA;

    state_group = create_state_group_with_custom_pipeline(
        shaders_dir.c_str(),
        "box_selection.vert",
        "box_selection.frag",
        vsg_options,
        vsg::VertexInputState::Bindings{
            VkVertexInputBindingDescription{0, sizeof(vsg::vec2),
                VK_VERTEX_INPUT_RATE_VERTEX}
        },
        vsg::VertexInputState::Attributes{
            VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32_SFLOAT, 0}
        },
        vsg::DescriptorSetLayoutBindings{
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                VK_SHADER_STAGE_VERTEX_BIT, nullptr}
        },
        vsg::Descriptors{
            vsg::DescriptorBuffer::create(vsg::DataList{translation, scale},
                0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        },
        vsg::InputAssemblyState::create(),
        vsg::RasterizationState::create(),
        vsg::MultisampleState::create(),
        vsg::ColorBlendState::create(),
        vsg::DepthStencilState::create()
    );

    const auto vertices = vsg::vec2Array::create(4);
    vertices->at(0) = {-0.5, -0.5};
    vertices->at(1) = {-0.5,  0.5};
    vertices->at(2) = { 0.5, -0.5};
    vertices->at(3) = { 0.5,  0.5};

    const auto geometry = vsg::Geometry::create();
    geometry->assignArrays(vsg::DataList{vertices});
    geometry->commands.push_back(vsg::Draw::create(4, 1, 0, 0));

    state_group->addChild(geometry);
}

BoxSelectionState::~BoxSelectionState() = default;

void BoxSelectionState::fill_status_bar() const
{
    ImGui::Text("Box selection state");
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::Text("Begin pos: %dx%d\n", begin_x, begin_y);
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::Text("End pos: %dx%d\n", end_x, end_y);
}

void BoxSelectionState::handle_button_release() const
{
    if (!(mouse->get_button_mask() & vsg::BUTTON_MASK_1))
    {
        state_manager.defer_switch_to_basic_editor_state();
    }
}

void BoxSelectionState::handle_mouse_move()
{
    end_x = mouse->get_pos_x();
    end_y = mouse->get_pos_y();
}
