#include "RouteEditor.h"

#include "CameraHandler.h"
#include "EditorGui.h"
#include "EditorState.h"
#include "Gizmo.h"
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
#include "SingleSwitch.h"
#include "WindowHandler.h"
#include "filesystem.h"
#include "shader_funcs.h"

#include <vsg/app/CloseHandler.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/CompileManager.h>
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
    const FileSystem& fs = FileSystem::getInstance();
    context_.settings.read(fs.combinePath(
        fs.getConfigDir(), "editor-settings.xml"));

    context_.options = vsg::Options::create();
    context_.options->sharedObjects = vsg::SharedObjects::create();
    context_.options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    context_.options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    context_.options->add(vsgXchange::all::create());

    configure_shaders();

    context_.window_handler = WindowHandler::create(context_);
    if (!context_.window)
    {
        return false;
    }

    context_.mouse_handler = MouseHandler::create();
    context_.keyboard_handler = KeyboardHandler::create(context_);
    context_.camera_handler = CameraHandler::create(context_);
    context_.intersection_handler = IntersectionHandler::create(context_);
    context_.scene_graph = SceneGraph::create(context_);

    context_.outline_builder = OutlineBuilder::create(context_);

    const auto scene_view = vsg::View::create(context_.camera, context_.scene_graph);
    scene_view->mask = MASK_SCENE;

    VkClearValue clear_value{};
    clear_value.depthStencil = {0.0f, 0};
    VkClearAttachment attachment{VK_IMAGE_ASPECT_DEPTH_BIT, 1, clear_value};
    const VkExtent2D& extent = context_.window->extent2D();
    VkClearRect rect{VkRect2D{VkOffset2D{0, 0}, extent}, 0, 1};

    context_.clear_attachments = vsg::ClearAttachments::create(
        vsg::ClearAttachments::Attachments{attachment},
        vsg::ClearAttachments::Rects{rect});

    const auto gui_view1 = vsg::View::create(context_.camera, context_.scene_graph);
    gui_view1->mask = MASK_GUI1;

    const auto gui_view2 = vsg::View::create(context_.camera, context_.scene_graph);
    gui_view2->mask = MASK_GUI2;

    const auto editor_gui = EditorGui::create(context_);

    const auto render_gui = vsgImGui::RenderImGui::create(context_.window, editor_gui);

    context_.render_graph = vsg::RenderGraph::create(context_.window);
    context_.render_graph->addChild(scene_view);
    context_.render_graph->addChild(context_.clear_attachments);
    context_.render_graph->addChild(gui_view1);
    context_.render_graph->addChild(context_.clear_attachments);
    context_.render_graph->addChild(gui_view2);
    context_.render_graph->addChild(context_.clear_attachments);
    context_.render_graph->addChild(render_gui);

    const auto command_graph = vsg::CommandGraph::create(context_.window,
        context_.render_graph);

    context_.viewer = vsg::Viewer::create();
    const vsg::observer_ptr<vsg::Viewer> observer_viewer(context_.viewer);

    RouteObjects selected_objects;
    RouteObjects hidden_objects;

    context_.object_selector = ObjectSelector::create(context_);

    context_.viewer->addWindow(context_.window);

    context_.viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    context_.viewer->addEventHandler(vsg::CloseHandler::create(context_.viewer));
    context_.viewer->addEventHandler(context_.window_handler);
    context_.viewer->addEventHandler(context_.mouse_handler);
    context_.viewer->addEventHandler(context_.keyboard_handler);
    context_.viewer->addEventHandler(context_.camera_handler);
    context_.viewer->addEventHandler(context_.intersection_handler);
    context_.viewer->addEventHandler(context_.object_selector);

    context_.viewer->assignRecordAndSubmitTaskAndPresentation({command_graph});

    const uint32_t num_lights = static_cast<uint32_t>(
        context_.settings.num_lights);

    auto resource_hints = vsg::ResourceHints::create();
    resource_hints->numLightsRange = {num_lights, num_lights + 1};

    context_.viewer->compile(resource_hints);

    return true;
}

void RouteEditor::run()
{
    while (context_.viewer->advanceToNextFrame())
    {
        static double prev_time = context_.viewer->getFrameStamp()->simulationTime;
        double curr_time = context_.viewer->getFrameStamp()->simulationTime;
        context_.delta_time = curr_time - prev_time;
        prev_time = curr_time;

        if (context_.state == EditorState::LOAD_ROUTE)
        {
            context_.scene_graph->load_route();
            context_.state = EditorState::EDIT_ROUTE;
        }

        context_.viewer->handleEvents();
        context_.viewer->update();
        context_.viewer->recordAndSubmit();
        context_.viewer->present();

        if (!context_.compile_infos.empty())
        {
            vsg::CompileResult compile_result;

            for (const CompileInfo& compile_info : context_.compile_infos)
            {
                const auto& group_node = compile_info.group_node;
                const vsg::Mask mask = compile_info.mask;
                const auto& node = compile_info.node;

                compile_result.add(context_.viewer->compileManager->compile(node));

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
            }

            vsg::updateViewer(*context_.viewer, compile_result);
            context_.compile_infos.clear();
        }

        RouteObjects& deferred_selection = context_.deferred_selection;
        for (auto it = deferred_selection.begin();
            it != deferred_selection.end();)
        {
            if ((*it)->select())
            {
                it = deferred_selection.erase(it);
                context_.gizmo->update_visibility();
            }
            else
            {
                ++it;
            }
        }
    }
}

void RouteEditor::configure_shaders()
{
    const auto flat_shader = vsg::createFlatShadedShaderSet(context_.options);
    const auto pbr_shader = vsg::createPhysicsBasedRenderingShaderSet(context_.options);
    const auto phong_shader = vsg::createPhongShaderSet(context_.options);

    const FileSystem& fs = FileSystem::getInstance();
    const auto shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto vert_shader = read_shader(VK_SHADER_STAGE_VERTEX_BIT,
        shaders_dir.c_str(), "standard.vert", context_.options);

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
