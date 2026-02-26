#include "RouteEditor.h"

#include "CameraHandler.h"
#include "EditorGui.h"
#include "EditorState.h"
#include "IntersectionHandler.h"
#include "KeyboardHandler.h"
#include "Mask.h"
#include "MouseHandler.h"
#include "ObjectSelector.h"
#include "Outline.h"
#include "Route.h"
#include "RouteObject.h"
#include "SceneGraph.h"
#include "Settings.h"
#include "WindowHandler.h"
#include "filesystem.h"
#include "shader_funcs.h"

#include <vsg/app/CloseHandler.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/app/Viewer.h>
#include <vsg/commands/ClearAttachments.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>
#include <vsgXchange/all.h>

#include <vulkan/vulkan_core.h>

#include <string>

RouteEditor::RouteEditor() = default;
RouteEditor::~RouteEditor() = default;

bool RouteEditor::initialize()
{
    const FileSystem& fs = FileSystem::getInstance();
    context.settings.read(fs.combinePath(
        fs.getConfigDir(), "editor-settings.xml"));

    Outline::set_settings(&context.settings);

    context.options = vsg::Options::create();
    context.options->sharedObjects = vsg::SharedObjects::create();
    context.options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    context.options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    context.options->add(vsgXchange::all::create());

    configure_shaders();

    context.window_handler = WindowHandler::create(context);
    const auto window = context.window_handler->get_window();
    if (!window)
    {
        return false;
    }

    context.mouse_handler = MouseHandler::create();
    context.keyboard_handler = KeyboardHandler::create(context);
    context.camera_handler = CameraHandler::create(context);
    context.intersection_handler = IntersectionHandler::create(context);
    context.scene_graph = SceneGraph::create(context);

    const auto camera = context.camera_handler->get_camera();

    const auto scene_view = vsg::View::create(camera, context.scene_graph);
    scene_view->mask = MASK_SCENE;

    VkClearValue clear_value{};
    clear_value.depthStencil = {0.0f, 0};
    VkClearAttachment attachment{VK_IMAGE_ASPECT_DEPTH_BIT, 1, clear_value};
    const VkExtent2D& extent = window->extent2D();
    VkClearRect rect{VkRect2D{VkOffset2D{0, 0}, extent}, 0, 1};

    context.clear_attachments = vsg::ClearAttachments::create(
        vsg::ClearAttachments::Attachments{attachment},
        vsg::ClearAttachments::Rects{rect});

    const auto gui_view1 = vsg::View::create(camera, context.scene_graph);
    gui_view1->mask = MASK_GUI1;

    const auto gui_view2 = vsg::View::create(camera, context.scene_graph);
    gui_view2->mask = MASK_GUI2;

    const auto editor_gui = EditorGui::create(context);

    const auto render_gui = vsgImGui::RenderImGui::create(window, editor_gui);

    context.render_graph = vsg::RenderGraph::create(window);
    context.render_graph->addChild(scene_view);
    context.render_graph->addChild(context.clear_attachments);
    context.render_graph->addChild(gui_view1);
    context.render_graph->addChild(context.clear_attachments);
    context.render_graph->addChild(gui_view2);
    context.render_graph->addChild(context.clear_attachments);
    context.render_graph->addChild(render_gui);

    const auto command_graph = vsg::CommandGraph::create(window,
        context.render_graph);

    context.viewer = vsg::Viewer::create();
    const vsg::observer_ptr<vsg::Viewer> observer_viewer(context.viewer);

    RouteObjects selected_objects;
    RouteObjects hidden_objects;

    RouteObject::set_observer_viewer(observer_viewer);

    context.object_selector = ObjectSelector::create(context);

    context.viewer->addWindow(window);

    context.viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    context.viewer->addEventHandler(vsg::CloseHandler::create(context.viewer));
    context.viewer->addEventHandler(context.window_handler);
    context.viewer->addEventHandler(context.mouse_handler);
    context.viewer->addEventHandler(context.keyboard_handler);
    context.viewer->addEventHandler(context.camera_handler);
    context.viewer->addEventHandler(context.intersection_handler);
    context.viewer->addEventHandler(context.object_selector);

    context.viewer->assignRecordAndSubmitTaskAndPresentation({command_graph});
    context.viewer->compile();

    return true;
}

void RouteEditor::run()
{
    while (context.viewer->advanceToNextFrame())
    {
        if (context.state == EditorState::LOAD_ROUTE)
        {
            context.scene_graph->load_route();
            context.state = EditorState::EDIT_ROUTE;
        }

        context.viewer->handleEvents();
        context.viewer->update();
        context.viewer->recordAndSubmit();
        context.viewer->present();
    }
}

void RouteEditor::configure_shaders()
{
    const auto flat_shader = vsg::createFlatShadedShaderSet(context.options);
    const auto pbr_shader = vsg::createPhysicsBasedRenderingShaderSet(context.options);
    const auto phong_shader = vsg::createPhongShaderSet(context.options);

    const FileSystem& fs = FileSystem::getInstance();
    const auto shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto vert_shader = read_shader(VK_SHADER_STAGE_VERTEX_BIT,
        shaders_dir.c_str(), "standard.vert", context.options);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_flat_shaded.frag", context.options, "flat", flat_shader);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_pbr.frag", context.options, "pbr", pbr_shader);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_phong.frag", context.options, "phong", phong_shader);

    const auto rasterization_state = vsg::RasterizationState::create();
    rasterization_state->cullMode = VK_CULL_MODE_NONE;

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

    context.options->shaderSets.clear();
    context.options->shaderSets["flat"] = flat_shader;
    context.options->shaderSets["pbr"] = pbr_shader;
    context.options->shaderSets["phong"] = phong_shader;
}
