#include "Outline.h"

#include "Settings.h"
#include "filesystem.h"
#include "shader_funcs.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/maths/box.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/utils/Builder.h>
#include <vsg/utils/ComputeBounds.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>
#include <vsgXchange/all.h>

#include <vulkan/vulkan_core.h>

#include <cassert>

struct OutlineStatic
{
    OutlineStatic();

    vsg::ref_ptr<vsg::Options> options;
    vsg::Builder builder;
};

Outline::Outline(
    const settings_t& settings,
    vsg::ref_ptr<vsg::PagedLOD> paged_lod
)
{
    assert(paged_lod);

    static OutlineStatic outline_static;
    const auto options = outline_static.options;
    auto& builder = outline_static.builder;

    const auto wireframe_outline = vsg::read_cast<vsg::Node>(
        paged_lod->filename, options);

    vsg::ComputeBounds compute_bounds;
    compute_bounds.useNodeBounds = false;
    wireframe_outline->accept(compute_bounds);

    const vsg::GeometryInfo geometry_info(vsg::box(compute_bounds.bounds));

    vsg::StateInfo state_info;
    state_info.blending = true;
    state_info.wireframe = true;

    const auto box_outline = builder.createBox(geometry_info, state_info);

    if (settings.show_wireframe)
    {
        this->addChild(wireframe_outline);
    }

    this->addChild(box_outline);
}

OutlineStatic::OutlineStatic()
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

//     VkPipelineColorBlendAttachmentState color_blend_attachment = {};
//     color_blend_attachment.blendEnable = VK_TRUE;
//     color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//     color_blend_attachment.dstColorBlendFactor =
//         VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//     color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
//     color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//     color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//     color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
//     color_blend_attachment.colorWriteMask =
//         VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
//         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

//     const auto color_blend_state = vsg::ColorBlendState::create();
//     color_blend_state->attachments = {color_blend_attachment};

//     const auto depth_stencil_state = vsg::DepthStencilState::create();
//     depth_stencil_state->depthTestEnable = VK_TRUE;
//     depth_stencil_state->depthWriteEnable = VK_FALSE;

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
}
