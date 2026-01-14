#include "RouteEditor.h"

#include "CameraHandler.h"
#include "EditorGui.h"
#include "EditorParams.h"
#include "EditorState.h"
#include "IntersectionHandler.h"
#include "KeyboardHandler.h"
#include "MouseHandler.h"
#include "ObjectSelector.h"
#include "Settings.h"
#include "WindowHandler.h"
#include "filesystem.h"
#include "shader_funcs.h"

#include <vsg/app/CloseHandler.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/app/Viewer.h>
#include <vsg/app/Window.h>
#include <vsg/commands/ClearAttachments.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/Layer.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/ViewportState.h>
#include <vsg/threading/OperationThreads.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/vk/DeviceFeatures.h>
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>
#include <vsgXchange/all.h>

#include <QString>

#include <vulkan/vulkan_core.h>

#include <string>

RouteEditor::RouteEditor() = default;
RouteEditor::~RouteEditor() = default;

bool RouteEditor::initialize()
{
    FileSystem& fs = FileSystem::getInstance();
    settings.read(fs.combinePath(fs.getConfigDir(), "settings.xml"));

    options = vsg::Options::create();
    options->sharedObjects = vsg::SharedObjects::create();
    options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->add(vsgXchange::all::create());

    configure_shaders();

    window_handler = WindowHandler::create(settings);
    const auto& window = window_handler->get_window();
    if (!window)
    {
        return false;
    }

    mouse_handler = MouseHandler::create();
    keyboard_handler = KeyboardHandler::create(settings);

    camera_handler = CameraHandler::create(settings, window->extent2D(),
        mouse_handler, keyboard_handler, 5.0);

    const auto& camera = camera_handler->get_camera();

    ambient_light = vsg::AmbientLight::create();

    scene_group = vsg::Group::create();
    scene_group->addChild(ambient_light);
    route = Route::create();
    scene_group->addChild(route);

    gui_group = vsg::Group::create();

    params = EditorParams::create();
    params->editor_state = &state;
    params->key_bindings = keyboard_handler->get_key_bindings();
    params->objects_ref = &route->get_objects_ref();
    params->route_map = &route->get_route_map();
    params->perspective = camera_handler->get_perspective();
    params->topology = &route->get_topology();

    auto editor_gui = EditorGui::create(params, settings, options);
    auto render_gui = vsgImGui::RenderImGui::create(window, editor_gui);

    auto scene_view = vsg::View::create(camera, scene_group);

    VkClearValue clear_value{};
    clear_value.depthStencil = {0.0f, 0};
    VkClearAttachment attachment{VK_IMAGE_ASPECT_DEPTH_BIT, 1, clear_value};
    const VkExtent2D& extent = window->extent2D();
    VkClearRect rect{VkRect2D{VkOffset2D{0, 0}, extent}, 0, 1};
    clear_attachments = vsg::ClearAttachments::create(vsg::ClearAttachments::Attachments{attachment}, vsg::ClearAttachments::Rects{rect});

    auto gui_view = vsg::View::create(camera, gui_group);

    auto render_graph = vsg::RenderGraph::create(window);
    render_graph->addChild(scene_view);
    render_graph->addChild(clear_attachments);
    render_graph->addChild(gui_view);
    render_graph->addChild(render_gui);

    auto command_graph = vsg::CommandGraph::create(window, render_graph);

    viewer = vsg::Viewer::create();
    vsg::observer_ptr<vsg::Viewer> observer_viewer(viewer);

    object_selector = ObjectSelector::create(settings, options, observer_viewer, gui_group);

    viewer->addWindow(window);
    viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    viewer->addEventHandler(window_handler);
    viewer->addEventHandler(mouse_handler);
    viewer->addEventHandler(keyboard_handler);
    viewer->addEventHandler(camera_handler);

    auto intersection_handler = IntersectionHandler::create(settings, options,
        camera_handler->get_look_at(), camera, scene_group, gui_group, observer_viewer, object_selector);

    viewer->addEventHandler(intersection_handler);

    params->selected_object = intersection_handler->get_curr_matrix_transform_ptr();
    params->perspective = camera_handler->get_perspective();

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
            route->directory = params->route_dir;
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
    // За основу берем встроенные комплекты вершинного и фрагментного шейдера
    auto flat_shader = vsg::createFlatShadedShaderSet(options);
    auto pbr_shader = vsg::createPhysicsBasedRenderingShaderSet(options);
    auto phong_shader = vsg::createPhongShaderSet(options);

    // Загружаем свои варианты вершинного и фрагментного шейдера вместо встроенного
    FileSystem& fs = FileSystem::getInstance();
    const auto shaders_dir = fs.combinePath(fs.getDataDir(), "shaders");

    const auto vert_shader = read_shader(VK_SHADER_STAGE_VERTEX_BIT, shaders_dir.c_str(), "standard.vert", options);

    configure_shader_set(shaders_dir.c_str(), vert_shader, "standard_flat_shaded.frag", options, "flat", flat_shader);
    configure_shader_set(shaders_dir.c_str(), vert_shader, "standard_pbr.frag", options, "PBR", pbr_shader);
    configure_shader_set(shaders_dir.c_str(), vert_shader, "standard_phong.frag", options, "Phong", phong_shader);

    // Рисуем текстуры на обеих сторонах полигонов
    auto rasterization_state = vsg::RasterizationState::create();
    rasterization_state->cullMode = VK_CULL_MODE_NONE;

    vsg::GraphicsPipelineStates default_graphics_pipeline_states = {
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

    // Добавляем шейдеры в стандартные опции
    options->shaderSets.clear();
    options->shaderSets["flat"] = flat_shader;
    options->shaderSets["pbr"] = pbr_shader;
    options->shaderSets["phong"] = phong_shader;
}
