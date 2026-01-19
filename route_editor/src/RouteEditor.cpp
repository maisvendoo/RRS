#include "RouteEditor.h"

#include "CameraHandler.h"
#include "EditorGui.h"
#include "EditorState.h"
#include "IntersectionHandler.h"
#include "KeyboardHandler.h"
#include "MouseHandler.h"
#include "ObjectSelector.h"
#include "Route.h"
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
#include <vsg/nodes/Group.h>
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
    settings.read(fs.combinePath(fs.getConfigDir(), "settings.xml"));

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
        mouse_handler, keyboard_handler, 5.0);

    const auto camera = camera_handler->get_camera();

    intersection_handler = IntersectionHandler::create(camera);

    scene_graph = SceneGraph::create();
    const auto route = scene_graph->get_route();

    VkClearValue clear_value{};
    clear_value.depthStencil = {0.0f, 0};
    VkClearAttachment attachment{VK_IMAGE_ASPECT_DEPTH_BIT, 1, clear_value};
    const VkExtent2D& extent = window->extent2D();
    VkClearRect rect{VkRect2D{VkOffset2D{0, 0}, extent}, 0, 1};

    clear_attachments = vsg::ClearAttachments::create(
        vsg::ClearAttachments::Attachments{attachment},
        vsg::ClearAttachments::Rects{rect});

    gui_group = vsg::Group::create();

    const auto scene_view = vsg::View::create(camera, scene_graph);
    const auto gui_view = vsg::View::create(camera, gui_group);

    const auto editor_gui = EditorGui::create(state,
        keyboard_handler->get_key_bindings(), route,
        settings, options);

    const auto render_gui = vsgImGui::RenderImGui::create(window, editor_gui);

    render_graph = vsg::RenderGraph::create(window);
    render_graph->addChild(scene_view);
    render_graph->addChild(clear_attachments);
    render_graph->addChild(gui_view);
    render_graph->addChild(render_gui);

    const auto command_graph = vsg::CommandGraph::create(window, render_graph);

    viewer = vsg::Viewer::create();
    vsg::observer_ptr<vsg::Viewer> observer_viewer(viewer);

    object_selector = ObjectSelector::create(settings, intersection_handler,
        route, gui_group, observer_viewer);

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
    const auto route = scene_graph->get_route();

    while (viewer->advanceToNextFrame())
    {
        if (state == EditorState::LOAD_ROUTE)
        {
            if (route->load(settings, options, viewer))
            {
                state = EditorState::EDIT_ROUTE;
            }
            else
            {
                state = EditorState::SELECT_ROUTE;
            }
        }

        viewer->handleEvents();
        viewer->update();
        viewer->recordAndSubmit();
        viewer->present();
    }
}

void RouteEditor::configure_shaders()
{
    enum
    {
        FLAT,
        PBR,
        PHONG,
        TOTAL_SHADER_SETS
    };

    const char* fragment_shader_names[TOTAL_SHADER_SETS];
    fragment_shader_names[FLAT] = "standard_flat_shaded.frag";
    fragment_shader_names[PBR] = "standard_pbr.frag";
    fragment_shader_names[PHONG] = "standard_phong.frag";

    const char* shader_set_names[TOTAL_SHADER_SETS];
    shader_set_names[FLAT] = "flat";
    shader_set_names[PBR] = "pbr";
    shader_set_names[PHONG] = "Phong";

    // За основу берем встроенные комплекты вершинного и фрагментного шейдера
    vsg::ref_ptr<vsg::ShaderSet> shader_sets[TOTAL_SHADER_SETS];
    shader_sets[FLAT] = vsg::createFlatShadedShaderSet(options);
    shader_sets[PBR] = vsg::createPhysicsBasedRenderingShaderSet(options);
    shader_sets[PHONG] = vsg::createPhongShaderSet(options);

    // Загружаем свои варианты вершинного и фрагментного шейдера
    // вместо встроенного
    const FileSystem& fs = FileSystem::getInstance();
    const std::string shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto vert_shader = read_shader(VK_SHADER_STAGE_VERTEX_BIT,
        shaders_dir.c_str(), "standard.vert", options);

    // Рисуем текстуры на обеих сторонах полигонов
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

    options->shaderSets.clear();

    for (int i = 0; i < TOTAL_SHADER_SETS; ++i)
    {
        configure_shader_set(shaders_dir.c_str(), vert_shader,
            fragment_shader_names[i], options,
            shader_set_names[i], shader_sets[i]);

        shader_sets[i]->defaultGraphicsPipelineStates =
            default_graphics_pipeline_states;

        // Добавляем шейдеры в стандартные опции
        options->shaderSets[shader_set_names[i]] = shader_sets[i];
    }
}
