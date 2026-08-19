#include    "sim-journal.h"

#include    "Journal.h"
#include    "JournalFile.h"

#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void init_journal()
{
    FileSystem &fs = FileSystem::getInstance();
    QString path = QString(fs.combinePath(fs.getLogsDir(), "journal.log").c_str());

    Journal::instance()->addStorage( new JournalFile(path, JournalLevel::All) );

    QString line = "";

    for (int i = 0; i < 80; ++i)
        line += "=";

    Journal::instance()->message(" ");
    Journal::instance()->message(line);
    Journal::instance()->message("Started new session");
    Journal::instance()->message("Journal subsystem is initialized successfully");
    Journal::instance()->message(line);
}
