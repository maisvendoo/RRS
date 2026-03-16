#include "Outline.h"

#include "filesystem.h"
#include "shader_funcs.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/maths/box.h>
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

#include <string>

OutlineBuilder::OutlineBuilder(const EditorContext& context)
    : context(context)
{
    options = vsg::Options::create();
    options->sharedObjects = vsg::SharedObjects::create();
    options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->add(vsgXchange::all::create());

    const auto flat_shader = vsg::createFlatShadedShaderSet(options);

    const FileSystem& fs = FileSystem::getInstance();
    const std::string shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto vert_shader = read_shader(VK_SHADER_STAGE_VERTEX_BIT,
        shaders_dir.c_str(), "standard.vert", options);

    const auto frag_shader = read_shader(VK_SHADER_STAGE_FRAGMENT_BIT,
        shaders_dir.c_str(), "outline.frag", options);

    configure_shader_set(vert_shader, frag_shader, "flat", flat_shader);

    const auto rasterization_state = vsg::RasterizationState::create();
    rasterization_state->cullMode = VK_CULL_MODE_NONE;
    rasterization_state->polygonMode = VK_POLYGON_MODE_LINE;
    rasterization_state->lineWidth = 1.2f;

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

    builder.shaderSet = flat_shader;
}

vsg::ref_ptr<vsg::Node> OutlineBuilder::create_outline(
    vsg::ref_ptr<vsg::PagedLOD> paged_lod)
{
    if (!paged_lod->pending)
    {
        return nullptr;
    }

    vsg::ComputeBounds compute_bounds;
    compute_bounds.useNodeBounds = false;
    paged_lod->pending->accept(compute_bounds);

    const vsg::GeometryInfo geometry_info(vsg::box(compute_bounds.bounds));

    vsg::StateInfo state_info;
    state_info.blending = true;
    state_info.wireframe = true;

    return builder.createBox(geometry_info, state_info);
}
