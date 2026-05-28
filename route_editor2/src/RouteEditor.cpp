#include "editor/RouteEditor.h"

#include "editor/Camera.h"
#include "editor/settings/CameraSettings.h"
#include "editor/settings/WindowSettings.h"

#include <CfgReader.h>
#include <Journal.h>
#include <JournalFile.h>
#include <JournalStorage.h>
#include <core/string_funcs.h>
#include <filesystem.h>
#include <graphics/common.h>

#include <vsg/app/Window.h>
#include <vsg/app/WindowTraits.h>
#include <vsg/io/Options.h>

#include <QString>

RouteEditor::RouteEditor(bool& success)
{
    initialize_journal();
    Journal* const journal = Journal::instance();

    if (!read_settings())
    {
        success = false;
        return;
    }
    journal->info("Settings are readed successfully");

    vsg_options = create_default_vsg_options();
    journal->info("VSG options are initialized successfully");

    if (!create_window())
    {
        success = false;
        return;
    }
    journal->info("Window is created successfully");

    success = true;
}

RouteEditor::~RouteEditor() = default;

void RouteEditor::initialize_journal(const char* filename) const
{
    Journal* const journal = Journal::instance();
    const FileSystem& fs = FileSystem::getInstance();

    journal->addStorage(
        new JournalFile(
            to_qstring(fs.combinePath(fs.getLogsDir(), filename)),
            JournalLevel::All
        )
    );

    const QString dash_line = QString('=').repeated(80);

    journal->message(dash_line);
    journal->message("Started new session");
    journal->message("Journal subsystem is initialized successfully");
    journal->message(dash_line);
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

    return true;
}

bool RouteEditor::create_window()
{
    const auto window_traits = vsg::WindowTraits::create();
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

    return true;
}
