#include "filesystem.h"
#include "Logger.h"
#include "RouteViewer.h"

#include <vsg/core/Exception.h>

#include <QApplication>

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>

#include <crash-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void initialize_logger();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void print_command_line_arguments(int argc, char* argv[]);

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    try
    {
        setup_crash_handler("../logs/viewer-crash.log");

        initialize_logger();
        print_command_line_arguments(argc, argv);

        QApplication application(argc, argv);

        RouteViewer viewer;
        viewer.initialize(argc, argv);
        return viewer.run();
    }
    catch (const vsg::Exception& exception)
    {
        LOG_FATAL("%s", exception.message.c_str());
        return EXIT_FAILURE;
    }
    catch (const std::exception& exception)
    {
        LOG_FATAL("%s", exception.what());
        return EXIT_FAILURE;
    }
    catch (const char* exception)
    {
        LOG_FATAL("%s", exception);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void initialize_logger()
{
    const FileSystem& fs = FileSystem::getInstance();

    constexpr const char* log_filename = "viewer.log";

    char log_backup[64];
    std::sprintf(log_backup, "~previous-%s", log_filename);

    const std::string new_log_file = fs.getLogsDir() + fs.separator() + log_filename;
    const std::string old_log_file = fs.getLogsDir() + fs.separator() + log_backup;

    Logger::instance().openFile(new_log_file.c_str(), old_log_file.c_str());

    LOG_INFO("================================================================================");
    LOG_INFO("Logger initialized succesfully");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void print_command_line_arguments(int argc, char* argv[])
{
    std::string command_line = "";
    for (int i = 0; i < argc; ++i)
    {
        command_line += " ";
        command_line += argv[i];
    }

    LOG_INFO("Process started with command line: %s", command_line.c_str());
    LOG_INFO("================================================================================");
}
