#include    "filesystem.h"
#include    "Logger.h"
#include    "topologycheck.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    std::string log_filename = "topologycheck.log";
    {
        FileSystem& fs = FileSystem::getInstance();
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

    TopologyCheck checker;

    if (!checker.run(argc, argv))
    {
        return -1;
    }

    LOG_INFO("Info: topology is checked. See /logs/%s", log_filename.c_str());
    return 0;
}
