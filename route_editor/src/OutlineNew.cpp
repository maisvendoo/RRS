#include "OutlineNew.h"

#include "Settings.h"
#include "filesystem.h"
#include "shader_funcs.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/PagedLOD.h>
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
#include <vsg/utils/Builder.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>
#include <vsgXchange/all.h>

#include <vulkan/vulkan_core.h>

#include <cassert>

struct OutlineStaticNew
{
    OutlineStaticNew();

    vsg::ref_ptr<vsg::Options> options;
    vsg::Builder builder;
    vsg::ref_ptr<vsg::BindGraphicsPipeline> bind_graphics_pipeline;
    vsg::ref_ptr<vsg::BindDescriptorSet> bind_descriptor_set;
};

OutlineNew::OutlineNew(const settings_t& settings,
    vsg::ref_ptr<vsg::PagedLOD> paged_lod)
{
    assert(paged_lod);

    static OutlineStaticNew outline_static;

    this->add(outline_static.bind_graphics_pipeline);
    this->add(outline_static.bind_descriptor_set);

    this->addChild(paged_lod);
}

OutlineStaticNew::OutlineStaticNew()
{
    options = vsg::Options::create();
    options->sharedObjects = vsg::SharedObjects::create();
    options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->add(vsgXchange::all::create());

    const auto flat_shader = vsg::createFlatShadedShaderSet(options);
    const auto pbr_shader = vsg::createPhysicsBasedRenderingShaderSet(options);
    const auto phong_shader = vsg::createPhongShaderSet(options);

    const FileSystem& fs = FileSystem::getInstance();
    const auto shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto vert_shader = read_shader(VK_SHADER_STAGE_VERTEX_BIT,
        shaders_dir.c_str(), "standard.vert", options);

    const auto frag_shader = read_shader(VK_SHADER_STAGE_FRAGMENT_BIT,
        shaders_dir.c_str(), "outline.frag", options);

    configure_shader_set(vert_shader, frag_shader, "flat", flat_shader);
    configure_shader_set(vert_shader, frag_shader, "pbr", pbr_shader);
    configure_shader_set(vert_shader, frag_shader, "phong", phong_shader);

    const auto rasterization_state = vsg::RasterizationState::create();
    rasterization_state->cullMode = VK_CULL_MODE_NONE;
    rasterization_state->polygonMode = VK_POLYGON_MODE_LINE;

    const vsg::GraphicsPipelineStates default_graphics_pipeline_states = {
        vsg::VertexInputState::create(),
        vsg::InputAssemblyState::create(),
        rasterization_state,
        vsg::ColorBlendState::create(),
        vsg::DepthStencilState::create(),
        vsg::MultisampleState::create()
    };

    flat_shader->defaultGraphicsPipelineStates =
        default_graphics_pipeline_states;

    pbr_shader->defaultGraphicsPipelineStates =
        default_graphics_pipeline_states;

    phong_shader->defaultGraphicsPipelineStates =
        default_graphics_pipeline_states;

    options->shaderSets.clear();
    options->shaderSets["flat"] = flat_shader;
    options->shaderSets["pbr"] = pbr_shader;
    options->shaderSets["phong"] = phong_shader;

    builder.options = options;
    builder.sharedObjects = options->sharedObjects;
    builder.shaderSet = flat_shader;

    const auto shader_stages = vsg::ShaderStages{vert_shader, frag_shader};

    const vsg::PushConstantRanges push_constant_ranges = {
        {VK_SHADER_STAGE_MESH_BIT_EXT, 0, 128}
    };

    const vsg::DescriptorSetLayoutBindings descriptor_bindings = {};

    const auto descriptor_set_layout = vsg::DescriptorSetLayout::create(
        descriptor_bindings);

    const auto pipeline_layout = vsg::PipelineLayout::create(
        vsg::DescriptorSetLayouts{descriptor_set_layout}, push_constant_ranges);

    const auto graphics_pipeline = vsg::GraphicsPipeline::create(
        pipeline_layout, shader_stages, default_graphics_pipeline_states);

    bind_graphics_pipeline = vsg::BindGraphicsPipeline::create(
        graphics_pipeline);

    const auto descriptor_set = vsg::DescriptorSet::create(
        descriptor_set_layout, vsg::Descriptors{});

    bind_descriptor_set = vsg::BindDescriptorSet::create(
        VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, descriptor_set);
}
