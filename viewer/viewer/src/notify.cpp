//------------------------------------------------------------------------------
//
//      OSG log messages handler
//      (c) maisvendoo, 12/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief OSG log messages handler
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 12/12/2018
 */

#include    <cstdio>
#include    "notify.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ViewerLogFileHandler::ViewerLogFileHandler(const std::string &dir, const std::string &file)
{
    std::string backup_prefix = "~previous-";
    std::string log_file = dir + file;
    std::string log_backup = dir + backup_prefix + file;

    std::remove(log_backup.c_str());
    std::rename(log_file.c_str(), log_backup.c_str());

    log.open(log_file.c_str());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ViewerLogFileHandler::~ViewerLogFileHandler()
{
    log.close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ViewerLogFileHandler::notify(osg::NotifySeverity severity, const char *msg)
{
    (void) severity;

    log << msg;
}
