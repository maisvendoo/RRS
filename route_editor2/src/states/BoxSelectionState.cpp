#include "editor/states/BoxSelectionState.h"

#include "editor/Mouse.h"
#include "editor/ObjectManager.h"
#include "editor/StateManager.h"
#include "editor/states/EditorState.h"

#include <filesystem.h>
#include <graphics/pipeline_funcs.h>

#include <vsg/app/Window.h>
#include <vsg/commands/Draw.h>
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/core/Mask.h>
#include <vsg/core/Value.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/maths/vec2.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/Switch.h>
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

#include <memory>
#include <string>

BoxSelectionState::BoxSelectionState(
    const vsg::ref_ptr<vsg::Window>& window,
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager,
    const vsg::ref_ptr<vsg::Options>& vsg_options,
    const vsg::ref_ptr<vsg::Group>& gui_group,
    const std::unique_ptr<ObjectManager>& object_manager,
    const vsg::ref_ptr<Camera>& camera
)
    : EditorState(window, mouse, keyboard, state_manager)
    , object_manager(object_manager)
    , camera(camera)
{
    name = "BoxSelectionState";

    const FileSystem& fs = FileSystem::getInstance();
    const std::string shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    transform_value = vsg::Value<Transform>::create();
    transform_value->set({{0.0, 0.0}, {1.0, 1.0}});
    transform_value->properties.dataVariance = vsg::DYNAMIC_DATA;

    const auto input_assembly_state = vsg::InputAssemblyState::create();
    input_assembly_state->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    const auto color_blend_state = vsg::ColorBlendState::create();
    color_blend_state->attachments = {{
        true,                                   // blending enabled
        VK_BLEND_FACTOR_SRC_ALPHA,              // srcColorBlendFactor
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,    // dstColorBlendFactor
        VK_BLEND_OP_ADD,                        // colorBlendOp
        VK_BLEND_FACTOR_ONE,                    // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ZERO,                   // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                        // alphaBlendOp
        VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
    }};

    state_group = create_state_group_with_custom_pipeline(
        shaders_dir.c_str(),
        "box_selection.vert",
        "box_selection.frag",
        vsg_options,
        vsg::VertexInputState::Bindings{
            VkVertexInputBindingDescription{
                0, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX
            }
        },
        vsg::VertexInputState::Attributes{
            VkVertexInputAttributeDescription{
                0, 0, VK_FORMAT_R32G32_SFLOAT, 0
            }
        },
        vsg::DescriptorSetLayoutBindings{{
            0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1, VK_SHADER_STAGE_VERTEX_BIT, nullptr
        }},
        vsg::Descriptors{
            vsg::DescriptorBuffer::create(
                transform_value, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
            )
        },
        input_assembly_state,
        vsg::RasterizationState::create(),
        vsg::MultisampleState::create(),
        color_blend_state,
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

    switch_node = vsg::Switch::create();
    switch_node->addChild(vsg::MASK_OFF, state_group);

    gui_group->addChild(switch_node);
}

BoxSelectionState::~BoxSelectionState() = default;

void BoxSelectionState::on_activate()
{
    begin_x = end_x = mouse->get_pos_x();
    begin_y = end_y = mouse->get_pos_y();

    update_selection();

    switch_node->setAllChildren(true);
}

void BoxSelectionState::on_deactivate()
{
    int min_x, max_x;
    int min_y, max_y;

    if (begin_x < end_x)
    {
        min_x = begin_x;
        max_x = end_x;
    }
    else
    {
        min_x = end_x;
        max_x = begin_x;
    }

    if (begin_y < end_y)
    {
        min_y = begin_y;
        max_y = end_y;
    }
    else
    {
        min_y = end_y;
        max_y = begin_y;
    }

    object_manager->check_intersections_and_select_objects(camera, min_x, min_y, max_x, max_y);

    switch_node->setAllChildren(false);
}

void BoxSelectionState::handle_window_resize() const
{
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

    update_selection();
}

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

void BoxSelectionState::update_selection()
{
    int min_x, max_x;
    int min_y, max_y;

    if (begin_x < end_x)
    {
        min_x = begin_x;
        max_x = end_x;
    }
    else
    {
        min_x = end_x;
        max_x = begin_x;
    }

    if (begin_y < end_y)
    {
        min_y = begin_y;
        max_y = end_y;
    }
    else
    {
        min_y = end_y;
        max_y = begin_y;
    }

    const float inverted_window_width = 1.0f / window->extent2D().width;
    const float inverted_window_height = 1.0f / window->extent2D().height;

    const float x1 = min_x * inverted_window_width * 2.0f - 1.0f;
    const float y1 = min_y * inverted_window_height * 2.0f - 1.0f;
    const float x2 = max_x * inverted_window_width * 2.0f - 1.0f;
    const float y2 = max_y * inverted_window_height * 2.0f - 1.0f;

    const float sx = x2 - x1;
    const float sy = y2 - y1;
    const float tx = (x1 + x2) * 0.5;
    const float ty = (y1 + y2) * 0.5;

    transform_value->set({{tx, ty}, {sx, sy}});
    transform_value->dirty();
}
