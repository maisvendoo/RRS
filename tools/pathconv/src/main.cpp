#include    "app.h"
#include    <filesystem.h>
#include    <Logger.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    {
        const FileSystem& fs = FileSystem::getInstance();

        constexpr const char* log_filename = "pathconv.log";

        char log_backup[64];
        std::sprintf(log_backup, "~previous-%s", log_filename);

        const std::string new_log_file = fs.getLogsDir() + fs.separator() + log_filename;
        const std::string old_log_file = fs.getLogsDir() + fs.separator() + log_backup;

        Logger::instance().openFile(new_log_file.c_str(), old_log_file.c_str());
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

    Application *app = new Application(argc, argv);

    return app->exec();
}
