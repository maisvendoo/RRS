#include "editor/RouteEditor.h"

#include "editor/Camera.h"
#include "editor/EditorGui.h"
#include "editor/EventHandler.h"
#include "editor/Keyboard.h"
#include "editor/Mouse.h"
#include "editor/ObjectManager.h"
#include "editor/Route.h"
#include "editor/StateManager.h"
#include "editor/settings/CameraSettings.h"
#include "editor/settings/GuiSettings.h"
#include "editor/settings/SceneSettings.h"
#include "editor/settings/WindowSettings.h"

#include <CfgReader.h>
#include <Journal.h>
#include <JournalFile.h>
#include <JournalStorage.h>
#include <core/string_funcs.h>
#include <filesystem.h>
#include <graphics/common.h>
#include <graphics/shader_funcs.h>

#include <vsg/app/CloseHandler.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/CompileManager.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/app/Viewer.h>
#include <vsg/app/Window.h>
#include <vsg/app/WindowTraits.h>
#include <vsg/commands/ClearAttachments.h>
#include <vsg/io/Options.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/Group.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ResourceHints.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/utils/ShaderSet.h>
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>

#include <QString>

#include <vulkan/vulkan_core.h>

#include <cstdio>
#include <cstdlib>
#include <memory>

RouteEditor::RouteEditor()
    : state_manager(keyboard, route_dir, camera)
    , route(object_manager, camera_settings.view_distance, vsg_options)
{
    initialize_journal();
    read_settings();
    print_settings();
    create_object_manager();
    create_vsg_options();
    configure_shaders();
    create_window();
    create_mouse();
    create_keyboard();
    create_event_handler();
    create_camera();
    create_scenegraph();
    create_scene_view();
    create_clear_attachments();
    create_editor_gui();
    create_render_gui();
    create_render_graph();
    create_command_graph();
    create_resource_hints();
    create_viewer();
}

RouteEditor::~RouteEditor() = default;

void RouteEditor::run()
{
    while (viewer->advanceToNextFrame())
    {
        viewer->handleEvents();
        viewer->update();
        viewer->recordAndSubmit();
        viewer->present();

        static size_t ts = 0;
        size_t tts = route.temp_transforms.size();
        if (tts != ts)
        {
            scenegraph->children.reserve(tts);
            for (size_t i = ts; i < tts; ++i)
            {
                scenegraph->addChild(route.temp_transforms[i]);
            }
            vsg::updateViewer(*viewer, viewer->compileManager->compile(scenegraph));
            ts = tts;
        }

        static double prev_simulation_time = viewer->getFrameStamp()->simulationTime;
        const double simulation_time = viewer->getFrameStamp()->simulationTime;
        const double delta_time = simulation_time - prev_simulation_time;
        prev_simulation_time = simulation_time;

        state_manager.update(delta_time);
    }

    route.join_threads();
}

void RouteEditor::initialize_journal(const char* filename) const
{
    const FileSystem& fs = FileSystem::getInstance();

    JournalFile* const journal_file = new(std::nothrow) JournalFile(
        to_qstring(fs.combinePath(fs.getLogsDir(), filename)),
        JournalLevel::All
    );

    if (!journal_file)
    {
        std::fputs("Failed to allocate memory for JournalFile\n", stderr);
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->addStorage(journal_file);

    const QString dash_line = QString('=').repeated(80);

    Journal::instance()->message(dash_line);
    Journal::instance()->message("Started new session");
    Journal::instance()->message("Journal subsystem is initialized successfully");
    Journal::instance()->message(dash_line);
}

void RouteEditor::read_settings(const char* filename)
{
    const FileSystem& fs = FileSystem::getInstance();

    const QString cfg_path = to_qstring(
        fs.combinePath(fs.getConfigDir(), filename)
    );

    CfgReader cfg;
    if (!cfg.load(cfg_path))
    {
        Journal::instance()->error("Failed to load config file " + cfg_path);
        return;
    }

    window_settings.read(cfg);
    camera_settings.read(cfg);
    scene_settings.read(cfg);
    gui_settings.read(cfg);
    key_bindings.read(cfg);

    Journal::instance()->info("Settings are readed successfully");
}

void RouteEditor::print_settings() const
{
    window_settings.print_in_journal();
    camera_settings.print_in_journal();
    scene_settings.print_in_journal();
    gui_settings.print_in_journal();
}

void RouteEditor::create_object_manager()
{
    object_manager = std::make_unique<ObjectManager>(scene_settings.max_object_count);
    if (!object_manager)
    {
        Journal::instance()->error("Failed to create object manager");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Object manager is created successfully");
}

void RouteEditor::create_vsg_options()
{
    vsg_options = create_default_vsg_options();
    if (!vsg_options)
    {
        Journal::instance()->error("Failed to initialize VSG options");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("VSG options are initialized successfully");
}

void RouteEditor::configure_shaders()
{
    const auto flat_shader = vsg::createFlatShadedShaderSet(vsg_options);
    const auto pbr_shader = vsg::createPhysicsBasedRenderingShaderSet(vsg_options);
    const auto phong_shader = vsg::createPhongShaderSet(vsg_options);

    const FileSystem& fs = FileSystem::getInstance();
    const auto shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto vert_shader = read_shader(shaders_dir.c_str(), "standard.vert", vsg_options);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_flat_shaded.frag", vsg_options, "flat", flat_shader);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_pbr.frag", vsg_options, "pbr", pbr_shader);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_phong.frag", vsg_options, "phong", phong_shader);

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
        pbr_shader->defaultGraphicsPipelineStates =
        phong_shader->defaultGraphicsPipelineStates =
        default_graphics_pipeline_states;

    vsg_options->shaderSets.clear();
    vsg_options->shaderSets["flat"] = flat_shader;
    vsg_options->shaderSets["pbr"] = pbr_shader;
    vsg_options->shaderSets["phong"] = phong_shader;
}

void RouteEditor::create_event_handler()
{
    event_handler = EventHandler::create(state_manager);
    if (!event_handler)
    {
        Journal::instance()->error("Failed to create event handler");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Event handler is created successfully");
}

void RouteEditor::create_camera()
{
    camera = Camera::create(camera_settings, window->extent2D(), keyboard);
    if (!camera)
    {
        Journal::instance()->error("Failed to create camera");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Camera is created successfully");
}

void RouteEditor::create_window()
{
    const auto window_traits = vsg::WindowTraits::create();
    if (!window_traits)
    {
        Journal::instance()->error("Failed to create window traits");
        std::exit(EXIT_FAILURE);
    }
    Journal::instance()->info("Window traits is created successfully");

    window_traits->x = window_settings.pos_x;
    window_traits->y = window_settings.pos_y;
    window_traits->width = window_settings.width;
    window_traits->height = window_settings.height;
    window_traits->fullscreen = window_settings.fullscreen;
    window_traits->screenNum = window_settings.screen_number;
    window_traits->windowTitle = window_settings.title;
    window_traits->swapchainPreferences.presentMode =
        window_settings.vsync
        ? VK_PRESENT_MODE_FIFO_KHR
        : VK_PRESENT_MODE_IMMEDIATE_KHR;
    window_traits->samples = get_vk_sample_count_flag(window_settings.samples);

    window = vsg::Window::create(window_traits);
    if (!window)
    {
        Journal::instance()->error("Failed to create window");
        std::exit(EXIT_FAILURE);
    }

    window->clearColor() = vsg::vec4(0.03f, 0.03f, 0.03f, 1.0f);

    Journal::instance()->info("Window is created successfully");
}

void RouteEditor::create_mouse()
{
    mouse = Mouse::create();
    if (!mouse)
    {
        Journal::instance()->error("Failed to create mouse");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Mouse is created successfully");
}

void RouteEditor::create_keyboard()
{
    keyboard = Keyboard::create(key_bindings);
    if (!keyboard)
    {
        Journal::instance()->error("Failed to create keyboard");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Keyboard is created successfully");
}

void RouteEditor::create_scenegraph()
{
    scenegraph = vsg::Group::create();
    if (!scenegraph)
    {
        Journal::instance()->error("Failed to create scenegraph");
        std::exit(EXIT_FAILURE);
    }

    const auto ambient_light = vsg::AmbientLight::create();
    ambient_light->color = {1.0, 1.0, 1.0};
    ambient_light->intensity = 1.0f;

    scenegraph->addChild(ambient_light);

    Journal::instance()->info("Scenegraph is created successfully");
}

void RouteEditor::create_scene_view()
{
    scene_view = vsg::View::create(camera, scenegraph);
    if (!scene_view)
    {
        Journal::instance()->error("Failed to create scene view");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Scene view is created successfully");
}

void RouteEditor::create_clear_attachments()
{
    VkClearValue clear_value = {};
    clear_value.depthStencil = {0.0f, 0};
    VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_DEPTH_BIT, 1, clear_value};
    const VkExtent2D window_extent = window->extent2D();
    VkClearRect clear_rect = {VkRect2D{VkOffset2D{0, 0}, window_extent}, 0, 1};

    clear_attachments = vsg::ClearAttachments::create(
        vsg::ClearAttachments::Attachments{clear_attachment},
        vsg::ClearAttachments::Rects{clear_rect}
    );

    if (!clear_attachments)
    {
        Journal::instance()->error("Failed to create clear attachments");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Clear attachments are created successfully");
}

void RouteEditor::create_editor_gui()
{
    editor_gui = EditorGui::create(gui_settings, state_manager, route_dir, route, close_handler);
    if (!editor_gui)
    {
        Journal::instance()->error("Failed to create editor GUI");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Editor GUI is created successfully");
}

void RouteEditor::create_render_gui()
{
    render_gui = vsgImGui::RenderImGui::create(window, editor_gui);
    if (!render_gui)
    {
        Journal::instance()->error("Failed to create render GUI");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Render GUI is created successfully");
}

void RouteEditor::create_render_graph()
{
    render_graph = vsg::RenderGraph::create(window);
    if (!render_graph)
    {
        Journal::instance()->error("Failed to create render graph");
        std::exit(EXIT_FAILURE);
    }

    render_graph->addChild(scene_view);
    render_graph->addChild(clear_attachments);
    render_graph->addChild(render_gui);

    Journal::instance()->info("Render graph is created successfully");
}

void RouteEditor::create_command_graph()
{
    command_graph = vsg::CommandGraph::create(window, render_graph);
    if (!command_graph)
    {
        Journal::instance()->error("Failed to create command graph");
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->info("Command graph is created successfully");
}

void RouteEditor::create_resource_hints()
{
    resource_hints = vsg::ResourceHints::create();
    if (!resource_hints)
    {
        Journal::instance()->error("Failed to create resource hints");
        std::exit(EXIT_FAILURE);
    }

    const unsigned int num_lights = static_cast<unsigned int>(scene_settings.num_lights);
    resource_hints->numLightsRange = {num_lights, num_lights + 1};

    Journal::instance()->info("Resource hints is created successfully");
}

void RouteEditor::create_viewer()
{
    viewer = vsg::Viewer::create();
    if (!viewer)
    {
        Journal::instance()->error("Failed to create viewer");
        std::exit(EXIT_FAILURE);
    }

    viewer->addWindow(window);

    close_handler = vsg::CloseHandler::create(viewer);
    close_handler->closeKey = vsg::KEY_Undefined;
    viewer->addEventHandler(close_handler);

    viewer->addEventHandler(mouse);
    viewer->addEventHandler(keyboard);
    viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    viewer->addEventHandler(event_handler);
    viewer->assignRecordAndSubmitTaskAndPresentation({command_graph});
    viewer->compile(resource_hints);

    Journal::instance()->info("Viewer is created successfully");
}
