#include "Logger.h"

#include <cstdarg>
#include <cstdio>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Logger& Logger::instance() noexcept
{
    static Logger logger;
    return logger;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Logger::~Logger() noexcept
{
    if (streams[1])
    {
        fclose(streams[1]);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Logger::openFile(const char* path, const char* backup_path)
{
    std::remove(backup_path);
    std::rename(path, backup_path);
    streams[1] = std::fopen(path, "w");

    if (streams[1])
        streams_count = 2;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Logger::log_message(LogLevel level, const char* file, int line, const char* format, ...) const
{
    if (this->level > level)
    {
        return;
    }

    for (int i = 0; i < streams_count; ++i)
    {
        std::va_list args;
        va_start(args, format);
        std::vfprintf(streams[i], format, args);
        va_end(args);
/*
        if (level > LOG_LEVEL_INFO)
        {
            std::fprintf(streams[i], " | %s (%d)", file, line);
        }
*/
        std::fputs("\n", streams[i]);
    }
}
