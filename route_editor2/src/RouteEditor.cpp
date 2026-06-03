#include "editor/RouteEditor.h"

#include "editor/Camera.h"
#include "editor/check_macro.h"
#include "editor/settings/CameraSettings.h"
#include "editor/settings/SceneSettings.h"
#include "editor/settings/WindowSettings.h"

#include <CfgReader.h>
#include <Journal.h>
#include <JournalFile.h>
#include <JournalStorage.h>
#include <core/string_funcs.h>
#include <filesystem.h>
#include <graphics/common.h>

#include <vsg/app/CloseHandler.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/app/Viewer.h>
#include <vsg/app/Window.h>
#include <vsg/app/WindowTraits.h>
#include <vsg/io/Options.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/Group.h>
#include <vsg/state/ResourceHints.h>

#include <QString>

#include <cstdio>

RouteEditor::RouteEditor(bool& success)
{
    CHECK(initialize_journal(), success)
    CHECK(read_settings(), success)
    CHECK(create_vsg_options(), success)
    CHECK(create_window(), success)
    CHECK(create_camera(), success)
    CHECK(create_scenegraph(), success)
    CHECK(create_scene_view(), success)
    CHECK(create_render_graph(), success)
    CHECK(create_command_graph(), success)
    CHECK(create_resource_hints(), success)
    CHECK(create_viewer(), success)

    success = true;
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
    }
}

bool RouteEditor::initialize_journal(const char* filename) const
{
    const FileSystem& fs = FileSystem::getInstance();

    JournalFile* const journal_file = new(std::nothrow) JournalFile(
        to_qstring(fs.combinePath(fs.getLogsDir(), filename)),
        JournalLevel::All
    );

    if (!journal_file)
    {
        std::fputs("Failed to allocate memory for JournalFile\n", stderr);
        return false;
    }

    Journal::instance()->addStorage(journal_file);

    const QString dash_line = QString('=').repeated(80);

    Journal::instance()->message(dash_line);
    Journal::instance()->message("Started new session");
    Journal::instance()->message("Journal subsystem is initialized successfully");
    Journal::instance()->message(dash_line);

    return true;
}

bool RouteEditor::read_settings(const char* filename)
{
    const FileSystem& fs = FileSystem::getInstance();

    const QString cfg_path = to_qstring(
        fs.combinePath(fs.getConfigDir(), filename)
    );

    CfgReader cfg;
    if (!cfg.load(cfg_path))
    {
        Journal::instance()->error("Failed to load config file " + cfg_path);
        return false;
    }

    window_settings.read(cfg);
    camera_settings.read(cfg);
    scene_settings.read(cfg);

    window_settings.print_in_journal();
    camera_settings.print_in_journal();
    scene_settings.print_in_journal();

    Journal::instance()->info("Settings are readed successfully");

    return true;
}

bool RouteEditor::create_vsg_options()
{
    vsg_options = create_default_vsg_options();
    if (!vsg_options)
    {
        Journal::instance()->error("Failed to initialize VSG options");
        return false;
    }

    Journal::instance()->info("VSG options are initialized successfully");
    return true;
}

bool RouteEditor::create_window()
{
    const auto window_traits = vsg::WindowTraits::create();
    if (!window_traits)
    {
        Journal::instance()->error("Failed to create window traits");
        return false;
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
        return false;
    }

    window->clearColor() = vsg::vec4(0.03f, 0.03f, 0.03f, 1.0f);
    Journal::instance()->info("Window is created successfully");
    return true;
}

bool RouteEditor::create_camera()
{
    bool success;
    camera = Camera::create(camera_settings, window->extent2D(), success);
    return success;
}

bool RouteEditor::create_scenegraph()
{
    scenegraph = vsg::Group::create();
    if (!scenegraph)
    {
        Journal::instance()->error("Failed to create scenegraph");
        return false;
    }

    Journal::instance()->info("Scenegraph is created successfully");
    return true;
}

bool RouteEditor::create_scene_view()
{
    scene_view = vsg::View::create(camera, scenegraph);
    if (!scene_view)
    {
        Journal::instance()->error("Failed to create scene view");
        return false;
    }

    Journal::instance()->info("Scene view is created successfully");
    return true;
}

bool RouteEditor::create_render_graph()
{
    render_graph = vsg::RenderGraph::create(window);
    if (!render_graph)
    {
        Journal::instance()->error("Failed to create render graph");
        return false;
    }

    render_graph->addChild(scene_view);

    Journal::instance()->info("Render graph is created successfully");
    return true;
}

bool RouteEditor::create_command_graph()
{
    command_graph = vsg::CommandGraph::create(window, render_graph);
    if (!command_graph)
    {
        Journal::instance()->error("Failed to create command graph");
        return false;
    }

    Journal::instance()->info("Command graph is created successfully");
    return true;
}

bool RouteEditor::create_resource_hints()
{
    resource_hints = vsg::ResourceHints::create();
    if (!resource_hints)
    {
        Journal::instance()->error("Failed to create resource hints");
        return false;
    }

    const unsigned int num_lights = static_cast<unsigned int>(scene_settings.num_lights);
    resource_hints->numLightsRange = {num_lights, num_lights + 1};

    Journal::instance()->info("Resource hints is created successfully");
    return true;
}

bool RouteEditor::create_viewer()
{
    viewer = vsg::Viewer::create();
    if (!viewer)
    {
        Journal::instance()->error("Failed to create viewer");
        return false;
    }

    viewer->addWindow(window);
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    viewer->assignRecordAndSubmitTaskAndPresentation({command_graph});
    viewer->compile(resource_hints);

    Journal::instance()->info("Viewer is created successfully");
    return true;
}
