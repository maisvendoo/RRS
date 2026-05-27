#include "editor/RouteEditor.h"

#include <Journal.h>
#include <JournalFile.h>
#include <JournalStorage.h>
#include <core/string_funcs.h>
#include <filesystem.h>
#include <graphics/common.h>

#include <vsg/io/Options.h>

#include <QString>

RouteEditor::RouteEditor()
{
    initialize_journal();
    vsg_options = create_default_vsg_options();
}

RouteEditor::~RouteEditor() = default;

void RouteEditor::initialize_journal() const
{
    Journal* const journal = Journal::instance();
    const FileSystem& fs = FileSystem::getInstance();

    journal->addStorage(
        new JournalFile(
            to_qstring(fs.combinePath(fs.getLogsDir(), "editor.log")),
            JournalLevel::All
        )
    );

    const QString dash_line = QString('=').repeated(80);

    journal->message(dash_line);
    journal->message("Started new session");
    journal->message("Journal subsystem is initialized successfully");
    journal->message(dash_line);
}
