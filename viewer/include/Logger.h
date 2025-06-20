#ifndef VIEWER_LOGGER_H
#define VIEWER_LOGGER_H

#include <cstdio>
#include <string>

#define ANSI_ESCAPE_CODE_RED "\033[31m"
#define ANSI_ESCAPE_CODE_GREEN "\033[32m"
#define ANSI_ESCAPE_CODE_YELLOW "\033[33m"
#define ANSI_ESCAPE_CODE_BLUE "\033[34m"

#define LOG_DEBUG(...) \
    Logger::instance().log_message(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_INFO(...) \
    Logger::instance().log_message(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_WARN(...) \
    Logger::instance().log_message(LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_ERROR(...) \
    Logger::instance().log_message(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_FATAL(...) \
    Logger::instance().log_message(LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

enum LogLevel
{
    LOG_LEVEL_ALL,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Logger
{
public:
    static Logger& instance();
    ~Logger();

public:
    LogLevel level = LOG_LEVEL_INFO;

    void log_message(LogLevel level, const char* file, int line, const char* message);

    template <typename... Args>
    void log_message(LogLevel level, const char* file, int line, const char* format, Args... args);

    void openFile(const std::string& path, const std::string& backup_path);

private:
    void print_ansi_escape_code(LogLevel level);

private:
    Logger() = default;
    FILE* file = nullptr;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <typename... Args>
void Logger::log_message(LogLevel level, const char* file, int line, const char* format, Args... args)
{
    if (this->level > level)
    {
        return;
    }

    print_ansi_escape_code(level);

    std::fprintf(stderr, format, args...);

    if (level > LOG_LEVEL_INFO)
    {
        std::fprintf(stderr, " | %s (%d)\033[0m\n", file, line);
    }
    else
    {
        std::fprintf(stderr, "\033[0m\n");
    }

    std::fprintf(this->file, format, args...);

    if (level > LOG_LEVEL_INFO)
    {
        std::fprintf(this->file, " | %s (%d)\n", file, line);
    }
    else
    {
        std::fprintf(this->file, "\n");
    }
}

#endif // VIEWER_LOGGER_H
