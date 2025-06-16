#include "Logger.h"

#include <cstdio>
#include <map>
#include <string>

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

Logger::~Logger()
{
    if (file)
    {
        fclose(file);
    }
}

void Logger::log_message(LogLevel level, const char* file, int line, const char* message)
{
    if (this->level > level)
    {
        return;
    }

    print_ansi_escape_code(level);

    std::fprintf(stderr, "%s", message);

    if (level > LOG_LEVEL_WARN)
    {
        std::fprintf(stderr, " | %s (%d)\033[0m\n", file, line);
    }
    else
    {
        std::fprintf(stderr, "\033[0m\n");
    }

    std::fprintf(this->file, "%s", message);

    if (level > LOG_LEVEL_WARN)
    {
        std::fprintf(this->file, " | %s (%d)\n", file, line);
    }
    else
    {
        std::fprintf(this->file, "\n");
    }
}

void Logger::openFile(const std::string& path, const std::string &backup_path)
{
    std::remove(backup_path.c_str());
    std::rename(path.c_str(), backup_path.c_str());
    file = fopen(path.c_str(), "w");
}

void Logger::print_ansi_escape_code(LogLevel level)
{
    const std::map<LogLevel, const char*> level_map = {
        {LOG_LEVEL_DEBUG, ANSI_ESCAPE_CODE_GREEN},
        {LOG_LEVEL_INFO, ANSI_ESCAPE_CODE_BLUE},
        {LOG_LEVEL_WARN, ANSI_ESCAPE_CODE_YELLOW},
        {LOG_LEVEL_ERROR, ANSI_ESCAPE_CODE_RED},
        {LOG_LEVEL_FATAL, ANSI_ESCAPE_CODE_RED}
    };

    if (level_map.count(level))
    {
        std::fprintf(stderr, "%s", level_map.at(level));
    }
}
