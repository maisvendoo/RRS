#include "editor/RouteEditor.h"

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
    initialize_journal("editor.log");

    if (!read_settings("editor-settings.xml"))
    {
        success = false;
        return;
    }
    Journal::instance()->info("Settings are readed successfully");

    vsg_options = create_default_vsg_options();
    Journal::instance()->info("VSG options are initialized successfully");

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

    return true;
}

void RouteEditor::create_window()
{
    const auto window_traits = vsg::WindowTraits::create();
}
