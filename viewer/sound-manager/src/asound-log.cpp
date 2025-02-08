//------------------------------------------------------------------------------
//
//      ASound log messages handler
//      (c) DimaGVRH, 09/01/2020
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief ASound log messages handler
 * \copyright DimaGVRH
 * \author DimaGVRH
 * \date 09/01/2020
 */

#include    <cstdio>
#include    "asound-log.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LogFileHandler::LogFileHandler(const std::string &dir, const std::string &file)
{
    std::string backup_prefix = "~previous-";
    std::string log_file = dir + file;
    std::string log_backup = dir + backup_prefix + file;

    std::remove(log_backup.c_str());
    std::rename(log_file.c_str(), log_backup.c_str());

    log_.open(log_file.c_str());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LogFileHandler::~LogFileHandler()
{
    log_.close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LogFileHandler::notify(const std::string msg)
{
    log_ << msg << std::endl;
}
