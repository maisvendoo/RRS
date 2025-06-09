//------------------------------------------------------------------------------
//
//      ZDS to RRS profile converter
//      (c) maisvendoo, 20/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief ZDS to RRS profile converter
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 20/12/2018
 */

#include    <main.h>
#include    <filesystem.h>
#include    <Logger.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    {
        FileSystem& fs = FileSystem::getInstance();
        std::string log_filename = "profconv.log";
        std::string log_backup = "~previous-" + log_filename;

        Logger::instance().openFile((fs.getLogsDir() + fs.separator() + log_filename),
                                    (fs.getLogsDir() + fs.separator() + log_backup));
        LOG_INFO("================================================================================");
        LOG_INFO("Logger initialized succesfully");
    }

    {
        std::string command_line = "";
        for (int i = 0; i < argc; ++i)
        {
            command_line += " ";
            command_line += argv[i];
        }
        LOG_INFO("Process started with command line:%s", command_line.c_str());
        LOG_INFO("================================================================================");
    }

    ZDSimConverter conv;

    if (!conv.run(argc, argv))
    {
        return -1;
    }

    LOG_INFO("Info: conversion is done succesfully");
    return 0;
}
