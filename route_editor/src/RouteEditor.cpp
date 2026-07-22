#include "RouteEditor.h"

#include "Camera.h"
#include "CfgReader.h"
#include "EditorContext.h"
#include "EditorGui.h"
#include "EditorState.h"
#include "EventHandler.h"
#include "Gizmo.h"
#include "IntersectionHandler.h"
#include "Journal.h"
#include "JournalFile.h"
#include "Keyboard.h"
#include "Mask.h"
#include "Mouse.h"
#include "ObjectSelector.h"
#include "Outline.h"
#include "RouteObject.h"
#include "SceneGraph.h"
#include "Settings.h"
#include "SingleSwitch.h"
#include "StateManager.h"
#include "UndoRedoSaveHandler.h"
#include "WindowHandler.h"
#include "filesystem.h"
#include "graphics/common.h"
#include "graphics/shader_funcs.h"

#include <core/string_funcs.h>

#include <vsg/app/CloseHandler.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/CompileManager.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/app/Viewer.h>
#include <vsg/commands/ClearAttachments.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
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

#include <vulkan/vulkan_core.h>

#include <string>

RouteEditor::RouteEditor() = default;
RouteEditor::~RouteEditor() = default;

bool RouteEditor::initialize()
{
    initialize_journal();

    read_settings();

    context_.options = create_default_vsg_options();

    configure_shaders();

    window_handler_ = WindowHandler::create(context_.settings.window_settings,
        context_.window, context_.camera);

    if (!context_.window)
    {
        return false;
    }

    mouse = Mouse::create();
    keyboard = Keyboard::create(context_.settings.key_bindings);
    auto undo_redo_save_handler = UndoRedoSaveHandler::create(
        keyboard, context_.commands, context_.route_dir,
        context_.static_objects_mutex, context_.static_objects);

    context_.camera = Camera::create(
        camera_settings,
        context_.window->extent2D(),
        mouse,
        keyboard,
        context_.delta_time
    );

    context_.intersection_handler = IntersectionHandler::create(context_.camera->get_camera());
    context_.scene_graph = SceneGraph::create(context_, camera_settings);

    context_.outline_builder = OutlineBuilder::create();

    const auto scene_view = vsg::View::create(context_.camera->get_camera(), context_.scene_graph);
    scene_view->mask = MASK_SCENE;

    VkClearValue clear_value{};
    clear_value.depthStencil = {0.0f, 0};
    VkClearAttachment attachment{VK_IMAGE_ASPECT_DEPTH_BIT, 1, clear_value};
    const VkExtent2D& extent = context_.window->extent2D();
    VkClearRect rect{VkRect2D{VkOffset2D{0, 0}, extent}, 0, 1};

    const auto clear_attachments_ = vsg::ClearAttachments::create(
        vsg::ClearAttachments::Attachments{attachment},
        vsg::ClearAttachments::Rects{rect});

    const auto gui_view1 = vsg::View::create(context_.camera->get_camera(), context_.scene_graph);
    gui_view1->mask = MASK_GUI1;

    const auto gui_view2 = vsg::View::create(context_.camera->get_camera(), context_.scene_graph);
    gui_view2->mask = MASK_GUI2;

    const auto editor_gui = EditorGui::create(context_, camera_settings, gui_settings);

    const auto render_gui = vsgImGui::RenderImGui::create(context_.window, editor_gui);

    const auto render_graph_ = vsg::RenderGraph::create(context_.window);
    render_graph_->addChild(scene_view);
    render_graph_->addChild(clear_attachments_);
    render_graph_->addChild(gui_view1);
    render_graph_->addChild(clear_attachments_);
    render_graph_->addChild(gui_view2);
    render_graph_->addChild(clear_attachments_);
    render_graph_->addChild(render_gui);

    const auto command_graph = vsg::CommandGraph::create(context_.window,
        render_graph_);

    viewer_ = vsg::Viewer::create();

    context_.object_selector = ObjectSelector::create(context_, mouse, keyboard, gizmo_settings);

    viewer_->addWindow(context_.window);

    viewer_->addEventHandler(keyboard);
    viewer_->addEventHandler(vsgImGui::SendEventsToImGui::create());
    viewer_->addEventHandler(vsg::CloseHandler::create(viewer_));
    viewer_->addEventHandler(window_handler_);
    viewer_->addEventHandler(mouse);
    viewer_->addEventHandler(undo_redo_save_handler);

    state_manager = std::make_unique<StateManager>(mouse, keyboard);
    viewer_->addEventHandler(EventHandler::create(*state_manager));

    viewer_->addEventHandler(context_.camera);
    viewer_->addEventHandler(context_.intersection_handler);
    viewer_->addEventHandler(context_.object_selector);

    viewer_->assignRecordAndSubmitTaskAndPresentation({command_graph});

    const uint32_t num_lights = static_cast<uint32_t>(
        context_.settings.scene_settings.num_lights);

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
        double curr_time = viewer_->getFrameStamp()->simulationTime;
        context_.delta_time = curr_time - prev_time;
        prev_time = curr_time;

        if (context_.state == EditorState::LOAD_ROUTE)
        {
            context_.scene_graph->load_route();
            context_.state = EditorState::EDIT_ROUTE;
        }

        viewer_->handleEvents();
        viewer_->update();
        viewer_->recordAndSubmit();
        viewer_->present();

        compile_models();
        handle_deferred_selection();
    }

    if (context_.load_static_objects_thread.joinable())
    {
        context_.load_static_objects_thread.join();
    }

    if (context_.load_topology_thread.joinable())
    {
        context_.load_topology_thread.join();
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

void RouteEditor::read_settings()
{
    const FileSystem& fs = FileSystem::getInstance();
    context_.settings.read(fs.combinePath(
        fs.getConfigDir(), "editor-settings.xml"));
    std::string cfg_path = fs.combinePath(fs.getConfigDir(), "editor-settings.xml");
    CfgReader cfg;
    if (!cfg.load(cfg_path.c_str()))
    {
        return;
    }
    camera_settings.read(cfg);
    gizmo_settings.read(cfg);
    gui_settings.read(cfg);
}

void RouteEditor::configure_shaders()
{
    const auto flat_shader = vsg::createFlatShadedShaderSet(context_.options);
    const auto pbr_shader = vsg::createPhysicsBasedRenderingShaderSet(context_.options);
    const auto phong_shader = vsg::createPhongShaderSet(context_.options);

    const FileSystem& fs = FileSystem::getInstance();
    const auto shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto vert_shader = read_shader(shaders_dir.c_str(), "standard.vert", context_.options);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_flat_shaded.frag", context_.options, "flat", flat_shader);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_pbr.frag", context_.options, "pbr", pbr_shader);

    configure_shader_set(shaders_dir.c_str(), vert_shader,
        "standard_phong.frag", context_.options, "phong", phong_shader);

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

    context_.options->shaderSets.clear();
    context_.options->shaderSets["flat"] = flat_shader;
    context_.options->shaderSets["pbr"] = pbr_shader;
    context_.options->shaderSets["phong"] = phong_shader;
}

void RouteEditor::compile_models()
{
    if (context_.compile_infos.empty())
    {
        return;
    }

    vsg::CompileResult compile_result;

    context_.compile_infos.for_each([&](const CompileInfo& compile_info) -> void {
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
    context_.compile_infos.clear();
}

void RouteEditor::handle_deferred_selection()
{
    const auto size = context_.deferred_selection.size();

    context_.deferred_selection.remove_if(
        [](const vsg::ref_ptr<RouteObject>& object) {
            return object->select();
        }
    );

    if (context_.deferred_selection.size() != size)
    {
        context_.gizmo->update_visibility();
    }
}
