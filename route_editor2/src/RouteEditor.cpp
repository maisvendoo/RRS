#include "editor/RouteEditor.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/EditorGui.h"
#include "editor/EventHandler.h"
#include "editor/Gizmo.h"
#include "editor/IntersectionHandler.h"
#include "editor/KeyBindings.h"
#include "editor/ObjectSelector.h"
#include "editor/Route.h"
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/commands/DrawIndexed.h>


#include <cmath>
#include <vector>
#include "editor/SingleSwitch.h"
#include "editor/TrajectoryPicker.h"
#include "editor/states/EditorState.h"

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
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/Group.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ResourceHints.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>
#include <vsgXchange/all.h>

#include <QString>

#include <vulkan/vulkan_core.h>

#include <cstdio>
#include <cstdlib>
#include <memory>

RouteEditor::RouteEditor()
{
    initialize_journal();
    read_settings();
    print_settings();

    context = new EditorContext(camera_settings, gui_settings);

    create_vsg_options();
    configure_shaders();
    create_window();

    camera = Camera::create(camera_settings, window->extent2D());
    context->camera = camera;

    create_scenegraph();
    create_scene_view();
    create_handlers();

    VkClearValue clear_value = {};
    clear_value.depthStencil = {0.0f, 0};
    VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_DEPTH_BIT, 1, clear_value};
    const VkExtent2D window_extent = window->extent2D();
    VkClearRect clear_rect = {VkRect2D{VkOffset2D{0, 0}, window_extent}, 0, 1};

    clear_attachments = vsg::ClearAttachments::create(
        vsg::ClearAttachments::Attachments{clear_attachment},
        vsg::ClearAttachments::Rects{clear_rect}
    );

    editor_gui = EditorGui::create(*context, gui_settings);
    render_gui = vsgImGui::RenderImGui::create(window, editor_gui);
    context->render_gui = render_gui;

    create_render_graph();
    create_command_graph();
    create_resource_hints();
    create_viewer();

    prev_frame_time = 0.0;
}

RouteEditor::~RouteEditor()
{
    delete context;
}

void RouteEditor::run()
{
    while (viewer->advanceToNextFrame())
    {
        const double curr_time = viewer->getFrameStamp()->simulationTime;
        context->delta_time = curr_time - prev_frame_time;
        prev_frame_time = curr_time;

        if (context->state == EditorState::LOAD_ROUTE)
        {
            load_route();
        }

        viewer->handleEvents();
        viewer->update();
        viewer->recordAndSubmit();
        viewer->present();

        compile_models();
    }
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
    gizmo_settings.read(cfg);

    Journal::instance()->info("Settings are readed successfully");
}

void RouteEditor::print_settings() const
{
    window_settings.print_in_journal();
    camera_settings.print_in_journal();
    scene_settings.print_in_journal();
    gui_settings.print_in_journal();
}

void RouteEditor::create_vsg_options()
{
    vsg_options = create_default_vsg_options();
    if (!vsg_options)
    {
        Journal::instance()->error("Failed to initialize VSG options");
        std::exit(EXIT_FAILURE);
    }

    context->options = vsg_options;

    Journal::instance()->info("VSG options are initialized successfully");
}

void RouteEditor::configure_shaders()
{
    const auto flat_shader = vsg::createFlatShadedShaderSet(vsg_options);
    const auto pbr_shader = vsg::createPhysicsBasedRenderingShaderSet(vsg_options);
    const auto phong_shader = vsg::createPhongShaderSet(vsg_options);

    const FileSystem& fs = FileSystem::getInstance();
    const auto shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto vert_shader = read_shader(shaders_dir.c_str(),
        "standard.vert", vsg_options);

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

    flat_shader->defaultGraphicsPipelineStates = default_graphics_pipeline_states;
    pbr_shader->defaultGraphicsPipelineStates = default_graphics_pipeline_states;
    phong_shader->defaultGraphicsPipelineStates = default_graphics_pipeline_states;

    vsg_options->shaderSets.clear();
    vsg_options->shaderSets["flat"] = flat_shader;
    vsg_options->shaderSets["pbr"] = pbr_shader;
    vsg_options->shaderSets["phong"] = phong_shader;

    Journal::instance()->info("Shader sets are configured successfully");
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

    window->clearColor() = vsg::vec4(0.55f, 0.70f, 0.85f, 1.0f);
    Journal::instance()->info("Window is created successfully");
}

/// Опорная сетка земли (как тайловая сетка TSRE): 10x10 км, шаг 100 м,
/// каждые 500 м - ярче; даёт точку отсчёта в пустых маршрутах
//------------------------------------------------------------------------------
static vsg::ref_ptr<vsg::Node> create_ground_grid(
    vsg::ref_ptr<const vsg::Options> options)
{
    const double half = 5000.0;
    const double step = 100.0;

    std::vector<vsg::vec3> vertices;
    std::vector<vsg::vec3> colors;
    std::vector<unsigned short> indices;

    unsigned short index = 0;

    for (double v = -half; v <= half + 1.0; v += step)
    {
        const bool major = (std::fmod(v, 500.0) < 0.5) || (std::fmod(v, 500.0) > 499.5);
        const vsg::vec3 color = major ? vsg::vec3(0.28f, 0.30f, 0.34f)
                                      : vsg::vec3(0.16f, 0.17f, 0.20f);

        // Линия вдоль Y
        vertices.push_back({static_cast<float>(-half), static_cast<float>(v), 0.0f});
        vertices.push_back({static_cast<float>(half), static_cast<float>(v), 0.0f});
        colors.push_back(color);
        colors.push_back(color);
        indices.push_back(index++);
        indices.push_back(index++);

        // Линия вдоль X
        vertices.push_back({static_cast<float>(v), static_cast<float>(-half), 0.0f});
        vertices.push_back({static_cast<float>(v), static_cast<float>(half), 0.0f});
        colors.push_back(color);
        colors.push_back(color);
        indices.push_back(index++);
        indices.push_back(index++);
    }

    const auto vertex_array = vsg::vec3Array::create(vertices.size());
    const auto color_array = vsg::vec3Array::create(colors.size());
    const auto index_array = vsg::ushortArray::create(indices.size());

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        vertex_array->at(i) = vertices[i];
        color_array->at(i) = colors[i];
    }

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        index_array->at(i) = indices[i];
    }

    const auto geometry = vsg::Geometry::create();
    geometry->assignArrays(vsg::DataList{vertex_array, color_array});
    geometry->assignIndices(index_array);
    geometry->commands.push_back(vsg::DrawIndexed::create(
        static_cast<uint32_t>(indices.size()), 1, 0, 0, 0));

    const auto state_group = create_trajectory_lines_state_group(options);
    state_group->addChild(geometry);
    return state_group;
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
    scenegraph->addChild(ambient_light);

    // Сетка земли: постоянный элемент сцены (и для пустых маршрутов)
    scenegraph->addChild(create_ground_grid(context->options));

    context->scenegraph = scenegraph;

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

void RouteEditor::create_handlers()
{
    context->intersection_handler = IntersectionHandler::create(camera);
    context->object_selector = ObjectSelector::create(*context);
    context->trajectory_picker = TrajectoryPicker::create(*context);

    // Гизмо трансформаций (G - цикл режимов): узел вешаем в сцену,
    // видимость включает/выключает сам по режиму. MeasureTool живёт
    // в самом EditorContext (создаётся его конструктором)
    context->gizmo = Gizmo::create(*context, gizmo_settings);
    context->scenegraph->addChild(context->gizmo);

    // Переназначаемые клавиши из настроек редактора (нет секции -
    // останутся значения по умолчанию)
    {
        const FileSystem& fs = FileSystem::getInstance();

        const QString cfg_path = to_qstring(
                    fs.combinePath(fs.getConfigDir(), "editor-settings.xml"));

        CfgReader cfg;

        if (cfg.load(cfg_path))
        {
            context->key_bindings.read(cfg);
        }
    }

    event_handler = EventHandler::create(*context);
    context->event_handler = event_handler;

    Journal::instance()->info("Event handlers are created successfully");
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

    const unsigned int num_lights =
        static_cast<unsigned int>(scene_settings.num_lights);
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
    viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    viewer->addEventHandler(event_handler);
    viewer->assignRecordAndSubmitTaskAndPresentation({command_graph});
    viewer->compile(resource_hints);

    context->viewer = viewer;

    Journal::instance()->info("Viewer is created successfully");
}

void RouteEditor::load_route()
{
    context->route = Route::create(*context);

    context->compile_infos.emplace_back(CompileInfo{
        scenegraph, context->route, vsg::MASK_ALL});

    context->state = EditorState::EDIT_ROUTE;

    Journal::instance()->info(QString("Route %1 is loading")
        .arg(context->route_dir.c_str()));
}

void RouteEditor::compile_models()
{
    if (context->compile_infos.empty())
    {
        return;
    }

    vsg::CompileResult compile_result;

    context->compile_infos.for_each(
        [&](const CompileInfo& compile_info) -> void {
            const auto& group_node = compile_info.group_node;
            const vsg::Mask mask = compile_info.mask;
            const auto& node = compile_info.node;

            if (group_node)
            {
                if (auto group = group_node.cast<vsg::Group>())
                {
                    group->addChild(node);
                }
                else if (auto switch_ = group_node.cast<vsg::Switch>())
                {
                    switch_->addChild(mask, node);
                }
                else if (auto single_switch = group_node.cast<SingleSwitch>())
                {
                    single_switch->node = node;
                }
            }

            compile_result.add(viewer->compileManager->compile(node));
        });

    vsg::updateViewer(*viewer, compile_result);
    context->compile_infos.clear();
}
