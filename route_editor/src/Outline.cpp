#include "Outline.h"

#include "Settings.h"
#include "filesystem.h"
#include "shader_funcs.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/observer_ptr.h>
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

#include <cassert>

const settings_t* Outline::s_settings = nullptr;

struct OutlineStatic
{
    OutlineStatic();

    vsg::ref_ptr<vsg::Options> options;
    vsg::Builder builder;
};

Outline::Outline(vsg::ref_ptr<vsg::PagedLOD> paged_lod)
    : paged_lod(paged_lod)
{
    assert(paged_lod);
}

void Outline::load(vsg::observer_ptr<vsg::Viewer> observer_viewer)
{
    if (box)
    {
        return;
    }

    if (!paged_lod->pending)
    {
        return;
    }

    static OutlineStatic outline_static;

    vsg::ComputeBounds compute_bounds;
    compute_bounds.useNodeBounds = false;
    paged_lod->pending->accept(compute_bounds);

    const vsg::GeometryInfo geometry_info(vsg::box(compute_bounds.bounds));

    vsg::StateInfo state_info;
    state_info.blending = true;
    state_info.wireframe = true;

    const auto viewer = observer_viewer.ref_ptr();
    const auto compile_manager = viewer->compileManager;

    box = outline_static.builder.createBox(geometry_info, state_info);

    const auto compile_result = compile_manager->compile(box);

    this->addChild(box);

    vsg::updateViewer(*viewer, compile_result);
}

void Outline::set_settings(const settings_t* settings)
{
    s_settings = settings;
}

OutlineStatic::OutlineStatic()
{
    options = vsg::Options::create();
    options->sharedObjects = vsg::SharedObjects::create();
    options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->add(vsgXchange::all::create());

    const auto flat_shader = vsg::createFlatShadedShaderSet(options);

    const FileSystem& fs = FileSystem::getInstance();
    const auto shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

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
