#include "editor/RouteEditor.h"

#include "editor/Camera.h"
#include "editor/EditorGui.h"
#include "editor/EventHandler.h"
#include "editor/Keyboard.h"
#include "editor/Mouse.h"
#include "editor/ObjectManager.h"
#include "editor/Route.h"
#include "editor/StateManager.h"
#include "editor/check_initialization.h"
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
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/ImageInfo.h>
#include <vsg/state/ImageView.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ResourceHints.h>
#include <vsg/state/Sampler.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>

#include <QString>

#include <vulkan/vulkan_core.h>

#include <cstdio>
#include <cstdlib>
#include <memory>

RouteEditor::RouteEditor()
    : route(object_manager, camera_settings.view_distance, vsg_options)
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
    create_gui_group();
    create_state_manager();
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

    const auto context = vsg::Context::create(window->getOrCreateDevice());
    const VkExtent2D target_extent = window->extent2D();
    const auto color_image_info = vsg::ImageInfo::create();
    const auto depth_image_info = vsg::ImageInfo::create();

    const auto device = context->device;
    VkExtent3D attachment_extent{target_extent.width, target_extent.height, 1};

    const auto color_image = vsg::Image::create();
    color_image->imageType = VK_IMAGE_TYPE_2D;
    color_image->format = VK_FORMAT_R8G8B8A8_SRGB;
    color_image->extent = attachment_extent;
    color_image->mipLevels = 1;
    color_image->arrayLayers = 1;
    color_image->samples = VK_SAMPLE_COUNT_1_BIT;
    color_image->tiling = VK_IMAGE_TILING_OPTIMAL;
    color_image->usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    color_image->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_image->flags = 0;
    color_image->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    const auto color_image_view = vsg::createImageView(*context, color_image, VK_IMAGE_ASPECT_COLOR_BIT);

    const auto color_sampler = vsg::Sampler::create();
    color_sampler->flags = 0;
    color_sampler->magFilter = VK_FILTER_LINEAR;
    color_sampler->minFilter = VK_FILTER_LINEAR;
    color_sampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    color_sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    color_sampler->addressModeV = color_sampler->addressModeU;
    color_sampler->addressModeW = color_sampler->addressModeU;
    color_sampler->mipLodBias = 0.0f;
    color_sampler->maxAnisotropy = 1.0f;
    color_sampler->minLod = 0.0f;
    color_sampler->maxLod = 1.0f;
    color_sampler->borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    color_image_info->imageView = color_image_view;
    color_image_info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    color_image_info->sampler = color_sampler;

    const VkFormat depth_format = VK_FORMAT_D32_SFLOAT;

    const auto depth_image = vsg::Image::create();
    depth_image->imageType = VK_IMAGE_TYPE_2D;
    depth_image->extent = attachment_extent;
    depth_image->mipLevels = 1;
    depth_image->arrayLayers = 1;
    depth_image->samples = VK_SAMPLE_COUNT_1_BIT;
    depth_image->format = depth_format;
    depth_image->tiling = VK_IMAGE_TILING_OPTIMAL;
    depth_image->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_image->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_image->flags = 0;
    depth_image->sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    depth_image_info->sampler = nullptr;
    depth_image_info->imageView = vsg::createImageView(*context, depth_image, VK_IMAGE_ASPECT_DEPTH_BIT);
    depth_image_info->imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    vsg::RenderPass::Attachments attachments(2);
    // Color attachment
    attachments[0].format = VK_FORMAT_R8G8B8A8_SRGB;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // Depth attachment
    attachments[1].format = depth_format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    const vsg::AttachmentReference color_reference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const vsg::AttachmentReference depth_reference = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    vsg::RenderPass::Subpasses subpass_description(1);
    subpass_description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description[0].colorAttachments.emplace_back(color_reference);
    subpass_description[0].depthStencilAttachments.emplace_back(depth_reference);

    vsg::RenderPass::Dependencies dependencies(2);

    // XXX This dependency is copied from the offscreenrender.cpp
    // example. I don't completely understand it, but I think its
    // purpose is to create a barrier if some earlier render pass was
    // using this framebuffer's attachment as a texture.
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // This is the heart of what makes Vulkan offscreen rendering
    // work: render passes that follow are blocked from using this
    // passes' color attachment in their fragment shaders until all
    // this pass' color writes are finished.
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    const auto render_pass = vsg::RenderPass::create(device, attachments, subpass_description, dependencies);

    const auto framebuffer = vsg::Framebuffer::create(render_pass,
        vsg::ImageViews{color_image_info->imageView,
        depth_image_info->imageView}, target_extent.width,
        target_extent.height, 1);

    const auto offscreen_render_graph = vsg::RenderGraph::create();
    offscreen_render_graph->renderArea.offset = VkOffset2D{0, 0};
    offscreen_render_graph->renderArea.extent = target_extent;
    offscreen_render_graph->framebuffer = framebuffer;

    offscreen_render_graph->clearValues.resize(2);
    offscreen_render_graph->clearValues[0].color = vsg::sRGB_to_linear(0.4f, 0.2f, 0.4f, 1.0f);
    offscreen_render_graph->clearValues[1].depthStencil = VkClearDepthStencilValue{0.0f, 0};

    const auto offscreen_view = vsg::View::create(camera, scenegraph);
    offscreen_render_graph->addChild(offscreen_view);
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

        if (route.just_loaded)
        {
            const auto& transforms = object_manager->get_transforms();
            scenegraph->children.reserve(transforms.size());
            for (const auto& transform : transforms)
            {
              scenegraph->addChild(transform);
            }
            vsg::updateViewer(*viewer, viewer->compileManager->compile(scenegraph));

            route.just_loaded = false;
        }

        static double prev_simulation_time = viewer->getFrameStamp()->simulationTime;
        const double simulation_time = viewer->getFrameStamp()->simulationTime;
        const double delta_time = simulation_time - prev_simulation_time;
        prev_simulation_time = simulation_time;

        state_manager->update(delta_time);
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
    object_manager = std::make_unique<ObjectManager>(
        static_cast<std::size_t>(scene_settings.max_object_count)
    );
    CHECK_INITIALIZATION(object_manager);
}

void RouteEditor::create_vsg_options()
{
    vsg_options = create_default_vsg_options();
    CHECK_INITIALIZATION(vsg_options);
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
    event_handler = EventHandler::create(*state_manager);
    CHECK_INITIALIZATION(event_handler);
}

void RouteEditor::create_camera()
{
    camera = Camera::create(camera_settings, window->extent2D(), mouse, keyboard);
    CHECK_INITIALIZATION(camera);
}

void RouteEditor::create_window()
{
    const auto window_traits = vsg::WindowTraits::create();
    CHECK_INITIALIZATION(window_traits);

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
    CHECK_INITIALIZATION(window);

    window->clearColor() = vsg::vec4(0.03f, 0.03f, 0.03f, 1.0f);
}

void RouteEditor::create_mouse()
{
    mouse = Mouse::create();
    CHECK_INITIALIZATION(mouse);
}

void RouteEditor::create_keyboard()
{
    keyboard = Keyboard::create(key_bindings);
    CHECK_INITIALIZATION(keyboard);
}

void RouteEditor::create_gui_group()
{
    gui_group = vsg::Group::create();
    CHECK_INITIALIZATION(gui_group);
}

void RouteEditor::create_state_manager()
{
    state_manager = std::make_unique<StateManager>(window, mouse, keyboard,
        camera, route_dir, vsg_options, gui_group);
    CHECK_INITIALIZATION(state_manager);
}

void RouteEditor::create_scenegraph()
{
    const auto ambient_light = vsg::AmbientLight::create();
    CHECK_INITIALIZATION(ambient_light);

    ambient_light->color = {1.0f, 1.0f, 1.0f};
    ambient_light->intensity = 1.0f;

    scenegraph = vsg::Group::create();
    CHECK_INITIALIZATION(scenegraph);

    scenegraph->addChild(ambient_light);
}

void RouteEditor::create_scene_view()
{
    scene_view = vsg::View::create(camera, scenegraph);
    CHECK_INITIALIZATION(scene_view);
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
    CHECK_INITIALIZATION(clear_attachments);
}

void RouteEditor::create_editor_gui()
{
    editor_gui = EditorGui::create(gui_settings, *state_manager, route_dir,
        route, close_handler);
    CHECK_INITIALIZATION(editor_gui);
}

void RouteEditor::create_render_gui()
{
    render_gui = vsgImGui::RenderImGui::create(window, editor_gui);
    CHECK_INITIALIZATION(render_gui);
}

void RouteEditor::create_render_graph()
{
    render_graph = vsg::RenderGraph::create(window);
    CHECK_INITIALIZATION(render_graph);

    render_graph->addChild(scene_view);
    render_graph->addChild(clear_attachments);
    render_graph->addChild(gui_group);
    render_graph->addChild(clear_attachments);
    render_graph->addChild(render_gui);
}

void RouteEditor::create_command_graph()
{
    command_graph = vsg::CommandGraph::create(window, render_graph);
    CHECK_INITIALIZATION(command_graph);
}

void RouteEditor::create_resource_hints()
{
    resource_hints = vsg::ResourceHints::create();
    CHECK_INITIALIZATION(resource_hints);

    const unsigned int num_lights = static_cast<unsigned int>(scene_settings.num_lights);
    resource_hints->numLightsRange = {num_lights, num_lights + 1};
}

void RouteEditor::create_viewer()
{
    viewer = vsg::Viewer::create();
    CHECK_INITIALIZATION(viewer);

    viewer->addWindow(window);

    close_handler = vsg::CloseHandler::create(viewer);
    CHECK_INITIALIZATION(close_handler);

    close_handler->closeKey = vsg::KEY_Undefined;

    viewer->addEventHandler(close_handler);
    viewer->addEventHandler(keyboard);
    viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    viewer->addEventHandler(mouse);
    viewer->addEventHandler(event_handler);
    viewer->assignRecordAndSubmitTaskAndPresentation({command_graph});
    viewer->compile(resource_hints);
}
