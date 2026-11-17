#include "editor/RouteEditor.h"

#include <Journal.h>
#include <JournalFile.h>
#include <JournalStorage.h>
#include <core/string_funcs.h>
#include <filesystem.h>

#include <QString>

#include <vsg/core/Exception.h>

#include <cstdio>
#include <cstdlib>
#include <exception>

static void initialize_journal(const char* log_filename)
{
    const FileSystem& fs = FileSystem::getInstance();
    JournalFile* const journal_file = new JournalFile{
        to_qstring(fs.combinePath(fs.getLogsDir(), log_filename)),
        JournalLevel::All};

    Journal& journal = *Journal::instance();
    journal.addStorage(journal_file);

    const QString dash_line = QString{'='}.repeated(80);

    journal.message(dash_line);
    journal.message("Started new session");
    journal.message("Journal subsystem is initialized successfully");
    journal.message(dash_line);
}

int main()
{
    try
    {
        initialize_journal("editor.log");
    }
    catch (const std::exception& exception)
    {
        std::fprintf(stderr, "Failed to initialize Journal subsystem.\n"
                             "Exception:\n"
                             "%s\n", exception.what());
        return EXIT_FAILURE;
    }

    try
    {
        RouteEditor route_editor{};
        if (!route_editor.initialize())
        {
            return EXIT_FAILURE;
        }

        route_editor.run();
    }
    catch (const vsg::Exception& exception)
    {
        Journal::instance()->error(exception.message.c_str());
        return EXIT_FAILURE;
    }
    catch (const std::exception& exception)
    {
        Journal::instance()->error(exception.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
