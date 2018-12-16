#include    "log.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LogFileHandler::LogFileHandler(const std::string &path)
{
    _log.open(path.c_str());
}

LogFileHandler::~LogFileHandler()
{
    _log.close();
}

void LogFileHandler::notify(osg::NotifySeverity severity, const char *msg)
{
    _log << msg;
}

#include	"log.h"
