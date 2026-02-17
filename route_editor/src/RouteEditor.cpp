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

RouteEditor::RouteEditor() = default;
RouteEditor::~RouteEditor() = default;

bool RouteEditor::initialize()
{
    const FileSystem& fs = FileSystem::getInstance();
    settings.read(fs.combinePath(fs.getConfigDir(), "editor-settings.xml"));

    Outline::set_settings(&settings);

    options = vsg::Options::create();
    options->sharedObjects = vsg::SharedObjects::create();
    options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->add(vsgXchange::all::create());

    configure_shaders();

    window_handler = WindowHandler::create(settings);
    const auto window = window_handler->get_window();
    if (!window)
    {
        return false;
    }

    mouse_handler = MouseHandler::create();
    keyboard_handler = KeyboardHandler::create(settings);

    camera_handler = CameraHandler::create(settings, window->extent2D(),
        mouse_handler, keyboard_handler);

    const auto camera = camera_handler->get_camera();

    intersection_handler = IntersectionHandler::create(camera);

    scene_graph = SceneGraph::create(settings, options);

    const auto scene_view = vsg::View::create(camera, scene_graph);
    scene_view->mask = MASK_SCENE;

    VkClearValue clear_value{};
    clear_value.depthStencil = {0.0f, 0};
    VkClearAttachment attachment{VK_IMAGE_ASPECT_DEPTH_BIT, 1, clear_value};
    const VkExtent2D& extent = window->extent2D();
    VkClearRect rect{VkRect2D{VkOffset2D{0, 0}, extent}, 0, 1};

    clear_attachments = vsg::ClearAttachments::create(
        vsg::ClearAttachments::Attachments{attachment},
        vsg::ClearAttachments::Rects{rect});

    const auto gui_view1 = vsg::View::create(camera, scene_graph);
    gui_view1->mask = MASK_GUI1;

    const auto gui_view2 = vsg::View::create(camera, scene_graph);
    gui_view2->mask = MASK_GUI2;

    const auto editor_gui = EditorGui::create(settings, commands, state,
        camera_handler->get_perspective(), scene_graph,
        object_selector, route_directory);

    const auto render_gui = vsgImGui::RenderImGui::create(window, editor_gui);

    render_graph = vsg::RenderGraph::create(window);
    render_graph->addChild(scene_view);
    render_graph->addChild(clear_attachments);
    render_graph->addChild(gui_view1);
    render_graph->addChild(clear_attachments);
    render_graph->addChild(gui_view2);
    render_graph->addChild(clear_attachments);
    render_graph->addChild(render_gui);

    const auto command_graph = vsg::CommandGraph::create(window, render_graph);

    viewer = vsg::Viewer::create();
    const vsg::observer_ptr<vsg::Viewer> observer_viewer(viewer);

    RouteObject::set_observer_viewer(observer_viewer);

    object_selector = ObjectSelector::create(settings, commands, mouse_handler,
        keyboard_handler, camera_handler, intersection_handler, scene_graph,
        observer_viewer);

    viewer->addWindow(window);

    viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    viewer->addEventHandler(window_handler);
    viewer->addEventHandler(mouse_handler);
    viewer->addEventHandler(keyboard_handler);
    viewer->addEventHandler(camera_handler);
    viewer->addEventHandler(intersection_handler);
    viewer->addEventHandler(object_selector);

    viewer->assignRecordAndSubmitTaskAndPresentation({command_graph});
    viewer->compile();

    return true;
}

void RouteEditor::run()
{
    while (viewer->advanceToNextFrame())
    {
        if (state == EditorState::LOAD_ROUTE)
        {
            const vsg::observer_ptr<vsg::Viewer> observer_viewer(viewer);
            scene_graph->load_route(observer_viewer, route_directory);
            state = EditorState::EDIT_ROUTE;
        }

        viewer->handleEvents();
        viewer->update();
        viewer->recordAndSubmit();
        viewer->present();
    }
}

void RouteEditor::configure_shaders()
{
    const auto flat_shader = vsg::createFlatShadedShaderSet(options);
    const auto pbr_shader = vsg::createPhysicsBasedRenderingShaderSet(options);
    const auto phong_shader = vsg::createPhongShaderSet(options);

    const FileSystem& fs = FileSystem::getInstance();
    const auto shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto vert_shader = read_shader(VK_SHADER_STAGE_VERTEX_BIT,
        shaders_dir.c_str(), "standard.vert", options);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_flat_shaded.frag", options, "flat", flat_shader);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_pbr.frag", options, "pbr", pbr_shader);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_phong.frag", options, "phong", phong_shader);

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

    options->shaderSets.clear();
    options->shaderSets["flat"] = flat_shader;
    options->shaderSets["pbr"] = pbr_shader;
    options->shaderSets["phong"] = phong_shader;
}
