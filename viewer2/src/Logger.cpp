#include "Logger.h"

#include <cstring>
#include <string>
#include <cstdarg>
#include <cstdio>

#define ANSI_ESCAPE_CODE_RED "\033[31m"
#define ANSI_ESCAPE_CODE_GREEN "\033[32m"
#define ANSI_ESCAPE_CODE_YELLOW "\033[33m"
#define ANSI_ESCAPE_CODE_BLUE "\033[34m"

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

void Logger::log_message(LogLevel level, const char* file, int line, const char* format, ...)
{
    if (this->level > level)
    {
        return;
    }

    char ansi_escape_code[16];

    switch (level)
    {
    case LOG_LEVEL_DEBUG:
    {
        std::strcpy(ansi_escape_code, ANSI_ESCAPE_CODE_GREEN);
        break;
    }
    case LOG_LEVEL_INFO:
    {
        std::strcpy(ansi_escape_code, ANSI_ESCAPE_CODE_BLUE);
        break;
    }
    case LOG_LEVEL_WARN:
    {
        std::strcpy(ansi_escape_code, ANSI_ESCAPE_CODE_YELLOW);
        break;
    }
    case LOG_LEVEL_ERROR:
    {
        std::strcpy(ansi_escape_code, ANSI_ESCAPE_CODE_RED);
        break;
    }
    case LOG_LEVEL_FATAL:
    {
        std::strcpy(ansi_escape_code, ANSI_ESCAPE_CODE_RED);
        break;
    }
    default:
    {
        break;
    }
    }

    std::fprintf(stderr, "%s", ansi_escape_code);

    std::va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);

    if (level > LOG_LEVEL_INFO)
    {
        std::fprintf(stderr, " | %s (%d)\033[0m\n", file, line);
    }
    else
    {
        std::fprintf(stderr, "\033[0m\n");
    }

    va_start(args, format);
    std::vfprintf(this->file, format, args);
    va_end(args);

    if (level > LOG_LEVEL_INFO)
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

Logger::Logger()
    : level(LOG_LEVEL_INFO)
    , file(nullptr)
{
}
