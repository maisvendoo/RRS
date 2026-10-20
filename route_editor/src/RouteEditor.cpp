#include "editor/RouteEditor.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/EditorGui.h"
#include "editor/EditorState.h"
#include "editor/EventHandler.h"
#include "editor/Gizmo.h"
#include "editor/Keyboard.h"
#include "editor/Mask.h"
#include "editor/Mouse.h"
#include "editor/ObjectSelector.h"
#include "editor/Outline.h"
#include "editor/Route.h"
#include "editor/RouteObject.h"
#include "editor/SingleSwitch.h"
#include "editor/StateManager.h"
#include "editor/StaticObjectManager.h"
#include "editor/WindowHandler.h"
#include "editor/WorldCulling.h"
#include "editor/commands/CommandManager.h"

#include <CfgReader.h>
#include <Journal.h>
#include <JournalFile.h>
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
#include <vsg/commands/ClearAttachments.h>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ResourceHints.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>
#include <vsgXchange/all.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <memory>
#include <string>

RouteEditor::RouteEditor() = default;
RouteEditor::~RouteEditor() = default;

bool RouteEditor::initialize()
{
    initialize_journal();
    read_settings();
    create_vsg_options();
    configure_shaders();

    window_handler_ = WindowHandler::create(editor_ctx);
    if (!editor_ctx.window)
    {
        return false;
    }

    editor_ctx.mouse = Mouse::create();
    editor_ctx.keyboard = Keyboard::create(editor_ctx.key_bindings);
    editor_ctx.command_manager = std::make_unique<CommandManager>();
    editor_ctx.camera = Camera::create(editor_ctx);
    editor_ctx.object_manager =
        std::make_unique<StaticObjectManager>(editor_ctx, 1000000);
    editor_ctx.route = Route::create(editor_ctx);
    editor_ctx.outline_builder = OutlineBuilder::create();

    const auto ambient_light = vsg::AmbientLight::create();

    const auto& scene_settings = editor_ctx.scene_settings;
    editor_ctx.world_culling
        = WorldCulling::create(scene_settings.culling_tiles_size_0,
                               scene_settings.culling_tiles_size_1);
    editor_ctx.scene_graph = vsg::Switch::create();
    editor_ctx.scene_graph->addChild(vsg::Mask{MASK_SCENE}, ambient_light);
    editor_ctx.scene_graph->addChild(vsg::MASK_ALL, editor_ctx.route);

    const auto scene_view
        = vsg::View::create(editor_ctx.camera, editor_ctx.scene_graph);
    scene_view->mask = MASK_SCENE;

    VkClearValue clear_value{};
    clear_value.depthStencil = {0.0f, 0};
    VkClearAttachment attachment{VK_IMAGE_ASPECT_DEPTH_BIT, 1, clear_value};
    const VkExtent2D& extent = editor_ctx.window->extent2D();
    VkClearRect rect{VkRect2D{VkOffset2D{0, 0}, extent}, 0, 1};

    const auto clear_attachments_ = vsg::ClearAttachments::create(
        vsg::ClearAttachments::Attachments{attachment},
        vsg::ClearAttachments::Rects{rect});

    const auto gui_view1
        = vsg::View::create(editor_ctx.camera, editor_ctx.scene_graph);
    gui_view1->mask = MASK_GUI1;

    const auto gui_view2
        = vsg::View::create(editor_ctx.camera, editor_ctx.scene_graph);
    gui_view2->mask = MASK_GUI2;

    editor_ctx.state_manager
        = std::make_unique<StateManager>(editor_ctx);
    const auto editor_gui = EditorGui::create(editor_ctx);

    const auto render_gui
        = vsgImGui::RenderImGui::create(editor_ctx.window, editor_gui);

    const auto render_graph_ = vsg::RenderGraph::create(editor_ctx.window);
    render_graph_->addChild(scene_view);
    render_graph_->addChild(clear_attachments_);
    render_graph_->addChild(gui_view1);
    render_graph_->addChild(clear_attachments_);
    render_graph_->addChild(gui_view2);
    render_graph_->addChild(clear_attachments_);
    render_graph_->addChild(render_gui);

    const auto command_graph
        = vsg::CommandGraph::create(editor_ctx.window, render_graph_);

    viewer_ = vsg::Viewer::create();

    editor_ctx.gizmo = Gizmo::create(editor_ctx);
    editor_ctx.scene_graph->addChild(vsg::Mask{MASK_GUI1 | MASK_CLICKABLE},
                                         editor_ctx.gizmo);

    editor_ctx.object_selector = ObjectSelector::create(editor_ctx);

    viewer_->addWindow(editor_ctx.window);

    viewer_->addEventHandler(editor_ctx.keyboard);
    viewer_->addEventHandler(vsgImGui::SendEventsToImGui::create());
    auto close_handler = vsg::CloseHandler::create(viewer_);
    close_handler->closeKey = vsg::KEY_P;
    viewer_->addEventHandler(close_handler);
    viewer_->addEventHandler(window_handler_);
    viewer_->addEventHandler(editor_ctx.mouse);

    viewer_->addEventHandler(
        EventHandler::create(editor_ctx.state_manager));

    viewer_->addEventHandler(editor_ctx.object_selector);

    viewer_->assignRecordAndSubmitTaskAndPresentation({command_graph});

    const uint32_t num_lights
        = static_cast<uint32_t>(editor_ctx.scene_settings.num_lights);

    auto resource_hints = vsg::ResourceHints::create();
    resource_hints->numLightsRange = {num_lights, num_lights + 1};

    viewer_->compile(resource_hints);

    return true;
}

void RouteEditor::run()
{
    while (viewer_->advanceToNextFrame())
    {
        static double prev_time = viewer_->getFrameStamp()->simulationTime;
        const double curr_time = viewer_->getFrameStamp()->simulationTime;
        const double delta_time = curr_time - prev_time;
        prev_time = curr_time;

        if (editor_ctx.editor_state == EditorState::LOAD_ROUTE)
        {
            editor_ctx.route->load();

            editor_ctx.compile_infos.lock()->emplace_back(
                CompileInfo{nullptr, editor_ctx.route, vsg::MASK_ALL});

            editor_ctx.editor_state = EditorState::EDIT_ROUTE;
        }

        viewer_->handleEvents();
        viewer_->update();
        viewer_->recordAndSubmit();
        viewer_->present();

        editor_ctx.state_manager->update(delta_time);

        compile_models();
        handle_deferred_selection();
    }

    editor_ctx.finish_topology_thread.store(true);

    if (editor_ctx.load_static_objects_thread.joinable())
    {
        editor_ctx.load_static_objects_thread.join();
    }

    if (editor_ctx.load_topology_thread.joinable())
    {
        editor_ctx.load_topology_thread.join();
    }
}

void RouteEditor::initialize_journal(const char* filename) const
{
    const FileSystem& fs = FileSystem::getInstance();

    JournalFile* const journal_file = new (std::nothrow)
        JournalFile(to_qstring(fs.combinePath(fs.getLogsDir(), filename)),
                    JournalLevel::All);

    if (!journal_file)
    {
        std::fputs("Failed to allocate memory for JournalFile\n", stderr);
        std::exit(EXIT_FAILURE);
    }

    Journal::instance()->addStorage(journal_file);

    const QString dash_line = QString('=').repeated(80);

    Journal::instance()->message(dash_line);
    Journal::instance()->message("Started new session");
    Journal::instance()->message(
        "Journal subsystem is initialized successfully");
    Journal::instance()->message(dash_line);
}

void RouteEditor::read_settings()
{
    const FileSystem& fs = FileSystem::getInstance();
    const std::string cfg_path
        = fs.combinePath(fs.getConfigDir(), "editor-settings.xml");

    CfgReader cfg;
    if (!cfg.load(cfg_path.c_str()))
    {
        return;
    }

    editor_ctx.camera_settings.read(cfg);
    editor_ctx.gizmo_settings.read(cfg);
    editor_ctx.gui_settings.read(cfg);
    editor_ctx.scene_settings.read(cfg);
    editor_ctx.window_settings.read(cfg);
    editor_ctx.key_bindings.read(cfg);
}

void RouteEditor::create_vsg_options()
{
    editor_ctx.vsg_options = vsg::Options::create();
    editor_ctx.vsg_options->sharedObjects = vsg::SharedObjects::create();
    editor_ctx.vsg_options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    editor_ctx.vsg_options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    editor_ctx.vsg_options->add(vsgXchange::all::create());
}

void RouteEditor::configure_shaders()
{
    const auto flat_shader
        = vsg::createFlatShadedShaderSet(editor_ctx.vsg_options);
    const auto pbr_shader
        = vsg::createPhysicsBasedRenderingShaderSet(editor_ctx.vsg_options);
    const auto phong_shader
        = vsg::createPhongShaderSet(editor_ctx.vsg_options);

    const FileSystem& fs = FileSystem::getInstance();
    const auto shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto vert_shader = read_shader(shaders_dir.c_str(), "standard.vert",
                                         editor_ctx.vsg_options);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
                         "standard_flat_shaded.frag",
                         editor_ctx.vsg_options, "flat", flat_shader);

    configure_shader_set(shaders_dir.c_str(), vert_shader, "standard_pbr.frag",
                         editor_ctx.vsg_options, "pbr", pbr_shader);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
                         "standard_phong.frag", editor_ctx.vsg_options,
                         "phong", phong_shader);

    const auto rasterization_state = vsg::RasterizationState::create();
    rasterization_state->cullMode = VK_CULL_MODE_NONE;

    const vsg::GraphicsPipelineStates default_graphics_pipeline_states
        = {vsg::VertexInputState::create(),
           vsg::InputAssemblyState::create(),
           rasterization_state,
           vsg::ColorBlendState::create(),
           vsg::DepthStencilState::create(),
           vsg::MultisampleState::create()};

    flat_shader->defaultGraphicsPipelineStates
        = default_graphics_pipeline_states;

    pbr_shader->defaultGraphicsPipelineStates
        = default_graphics_pipeline_states;

    phong_shader->defaultGraphicsPipelineStates
        = default_graphics_pipeline_states;

    editor_ctx.vsg_options->shaderSets.clear();
    editor_ctx.vsg_options->shaderSets["flat"] = flat_shader;
    editor_ctx.vsg_options->shaderSets["pbr"] = pbr_shader;
    editor_ctx.vsg_options->shaderSets["phong"] = phong_shader;
}

void RouteEditor::compile_models()
{
    auto compile_infos = editor_ctx.compile_infos.lock();
    if (compile_infos->empty())
    {
        return;
    }

    vsg::CompileResult compile_result;

    auto compile_info_nodes = editor_ctx.compile_info_nodes.lock();
    for (auto& compile_info_node : *compile_info_nodes)
    {

    }

    std::for_each(
        compile_infos->begin(), compile_infos->end(),
        [&](const CompileInfo& compile_info)
        {
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

            compile_result.add(viewer_->compileManager->compile(node));
        });

    vsg::updateViewer(*viewer_, compile_result);
    compile_infos->clear();
}

void RouteEditor::handle_deferred_selection()
{
    const auto size = editor_ctx.deferred_selection.size();

    editor_ctx.deferred_selection.remove_if(
        [](const vsg::ref_ptr<RouteObject>& object)
        { return object->select(); });

    if (editor_ctx.deferred_selection.size() != size)
    {
        editor_ctx.gizmo->update_visibility();
    }
}
