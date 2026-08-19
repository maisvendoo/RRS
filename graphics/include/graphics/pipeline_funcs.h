#ifndef GRAPHICS_PIPELINE_FUNCS_H
#define GRAPHICS_PIPELINE_FUNCS_H

#include <vsg/core/ref_ptr.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/VertexInputState.h>

namespace vsg
{

class ColorBlendState;
class DepthStencilState;
class InputAssemblyState;
class MultisampleState;
class Options;
class RasterizationState;
class StateGroup;

}

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
);

#endif // GRAPHICS_PIPELINE_FUNCS_H
