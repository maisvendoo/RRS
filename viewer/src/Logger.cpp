#include "Logger.h"

#include <cstdarg>
#include <cstdio>

#define ANSI_ESCAPE_CODE_RESET "\033[0m"
#define ANSI_ESCAPE_CODE_RED "\033[31m"
#define ANSI_ESCAPE_CODE_GREEN "\033[32m"
#define ANSI_ESCAPE_CODE_YELLOW "\033[33m"
#define ANSI_ESCAPE_CODE_BLUE "\033[34m"

#define IS_TEXT_COLORED 0

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
    if (file)
    {
        std::fclose(file);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Logger::openFile(const char* path, const char* backup_path)
{
    std::remove(backup_path);
    std::rename(path, backup_path);
    file = std::fopen(path, "w");
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

    std::FILE* const streams[2] = {stderr, this->file};
    for (int i = 0; i < 2; ++i)
    {
#if IS_TEXT_COLORED
        print_ansi_escape_code(level, streams[i]);
#endif

        std::va_list args;
        va_start(args, format);
        std::vfprintf(streams[i], format, args);
        va_end(args);

        if (level > LOG_LEVEL_INFO)
        {
            std::fprintf(streams[i], " | %s (%d)", file, line);
        }

#if IS_TEXT_COLORED
        std::fputs(ANSI_ESCAPE_CODE_RESET, streams[i]);
#endif

        std::fputs("\n", streams[i]);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Logger::print_ansi_escape_code(LogLevel level, std::FILE* stream) const
{
    switch (level)
    {
        case LOG_LEVEL_DEBUG:
        {
            std::fputs(ANSI_ESCAPE_CODE_GREEN, stream);
            return;
        }
        case LOG_LEVEL_INFO:
        {
            std::fputs(ANSI_ESCAPE_CODE_BLUE, stream);
            return;
        }
        case LOG_LEVEL_WARN:
        {
            std::fputs(ANSI_ESCAPE_CODE_YELLOW, stream);
            return;
        }
        case LOG_LEVEL_ERROR:
        case LOG_LEVEL_FATAL:
        {
            std::fputs(ANSI_ESCAPE_CODE_RED, stream);
            return;
        }
        default:
        {
            return;
        }
    }
}
