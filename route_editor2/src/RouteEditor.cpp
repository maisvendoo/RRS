#include "editor/RouteEditor.h"

#include "editor/Camera.h"
#include "editor/check_macro.h"
#include "editor/settings/CameraSettings.h"
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
#include <vsg/nodes/Group.h>
#include <vsg/state/ResourceHints.h>

#include <QString>

#include <cstdio>

RouteEditor::RouteEditor(bool& success)
{
    CHECK(initialize_journal(), success);
    CHECK(read_settings(), success)
    CHECK(create_vsg_options(), success)
    CHECK(create_window(), success)

    camera = Camera::create(camera_settings, window->extent2D(), success);
    CHECK(success, success)

    scenegraph = vsg::Group::create();
    if (!scenegraph)
    {
        success = false;
        Journal::instance()->error("Failed to create scenegraph");
        return;
    }
    Journal::instance()->info("Scenegraph is created successfully");

    scene_view = vsg::View::create(camera, scenegraph);
    if (!scene_view)
    {
        success = false;
        Journal::instance()->error("Failed to create scene view");
        return;
    }
    Journal::instance()->info("Scene view is created successfully");

    render_graph = vsg::RenderGraph::create(window);
    if (!render_graph)
    {
        success = false;
        Journal::instance()->error("Failed to create render graph");
        return;
    }

    render_graph->addChild(scene_view);
    Journal::instance()->info("Render graph is created successfully");

    command_graph = vsg::CommandGraph::create(window, render_graph);
    if (!command_graph)
    {
        success = false;
        Journal::instance()->error("Failed to create command graph");
        return;
    }
    Journal::instance()->info("Command graph is created successfully");

    resource_hints = vsg::ResourceHints::create();
    if (!resource_hints)
    {
        success = false;
        Journal::instance()->error("Failed to create resource hints");
        return;
    }

    // TODO: Set resource hints's numLightsRange
    Journal::instance()->info("Resource hints is created successfully");

    viewer = vsg::Viewer::create();
    if (!viewer)
    {
        success = false;
        Journal::instance()->error("Failed to create viewer");
        return;
    }
    Journal::instance()->info("Viewer is created successfully");

    viewer->addWindow(window);
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    viewer->assignRecordAndSubmitTaskAndPresentation({command_graph});
    viewer->compile(resource_hints);

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

    window_settings.print_in_journal();
    camera_settings.print_in_journal();

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

    Journal::instance()->info("Window is created successfully");
    return true;
}
