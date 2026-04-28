#include "graphics/pipeline_funcs.h"

#include "graphics/shader_funcs.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/VertexInputState.h>

#include <vulkan/vulkan_core.h>

vsg::ref_ptr<vsg::StateGroup> create_state_group_with_custom_pipeline(
    const char* shaders_dir,
    const char* vert_shader_filename,
    const char* frag_shader_filename,
    vsg::ref_ptr<const vsg::Options> options,
    const vsg::VertexInputState::Bindings& vertex_bindings_descriptions,
    const vsg::VertexInputState::Attributes& vertex_attribute_descriptions,
    const vsg::DescriptorSetLayoutBindings& descriptor_set_layout_bindings,
    const vsg::Descriptors& descriptors,
    vsg::ref_ptr<vsg::InputAssemblyState> input_assembly_state,
    vsg::ref_ptr<vsg::RasterizationState> rasterization_state,
    vsg::ref_ptr<vsg::MultisampleState> multisample_state,
    vsg::ref_ptr<vsg::ColorBlendState> color_blend_state,
    vsg::ref_ptr<vsg::DepthStencilState> depth_stencil_state
)
{
    const auto vertex_shader{read_shader(shaders_dir, vert_shader_filename, options)};
    const auto fragment_shader{read_shader(shaders_dir, frag_shader_filename, options)};

    const auto descriptor_set_layout{vsg::DescriptorSetLayout::create(descriptor_set_layout_bindings)};

    const vsg::PushConstantRanges push_constant_ranges{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128}
    };

    const vsg::GraphicsPipelineStates pipeline_states{
        vsg::VertexInputState::create(vertex_bindings_descriptions, vertex_attribute_descriptions),
        input_assembly_state,
        rasterization_state,
        multisample_state,
        color_blend_state,
        depth_stencil_state
    };

    const auto pipeline_layout{vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{descriptor_set_layout}, push_constant_ranges)};
    const auto graphics_pipeline{vsg::GraphicsPipeline::create(pipeline_layout, vsg::ShaderStages{vertex_shader, fragment_shader}, pipeline_states)};
    const auto bind_graphics_pipeline{vsg::BindGraphicsPipeline::create(graphics_pipeline)};

    const auto descriptor_set{vsg::DescriptorSet::create(descriptor_set_layout, descriptors)};
    const auto bind_descriptor_set{vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, descriptor_set)};

    const auto state_group{vsg::StateGroup::create()};
    state_group->add(bind_graphics_pipeline);
    state_group->add(bind_descriptor_set);

    return state_group;
}
