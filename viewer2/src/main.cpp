#include "filesystem.h"
#include "Logger.h"
#include "RouteViewer.h"

#include <QtWidgets/qapplication.h>
#include <memory>
#include <vsg/core/Exception.h>

int main(int argc, char* argv[])
{
    FileSystem& fs = FileSystem::getInstance();
    std::string log_filename = "viewer.log";
    std::string log_backup = "~previous-" + log_filename;

    Logger::instance().openFile((fs.getLogsDir() + fs.separator() + log_filename),
                                (fs.getLogsDir() + fs.separator() + log_backup));
    LOG_INFO("================================================================================");
    LOG_INFO("Logger initialized succesfully");
    std::string command_line = "";
    for (int i = 0; i < argc; ++i)
    {
        command_line += " ";
        command_line += argv[i];
    }
    LOG_INFO("Process started with command line:%s", command_line.c_str());
    LOG_INFO("================================================================================");

    try
    {
        QApplication application(argc, argv);
        auto viewer = std::make_unique<RouteViewer>(argc, argv);

        if (viewer->isReady())
        {
            return viewer->run();
        }
    }
    catch (const vsg::Exception& exception)
    {
        LOG_FATAL("%s", exception.message.c_str());
        return 1;
    }


    return 0;
}
