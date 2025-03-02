#ifndef VIEWER_LOGGER_H
#define VIEWER_LOGGER_H

#include <string>

#include <cstdio>

#define LOG_DEBUG(format, ...) \
    Logger::instance().log_message(LOG_LEVEL_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_INFO(format, ...) \
    Logger::instance().log_message(LOG_LEVEL_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_WARN(format, ...) \
    Logger::instance().log_message(LOG_LEVEL_WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
    Logger::instance().log_message(LOG_LEVEL_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_FATAL(format, ...) \
    Logger::instance().log_message(LOG_LEVEL_FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

enum LogLevel
{
    LOG_LEVEL_ALL,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
};

class Logger
{
public:
    static Logger& instance();
    ~Logger();

public:
    LogLevel level;

    void log_message(LogLevel level, const char* file, int line, const char* format, ...);

    void openFile(const std::string& path, const std::string& backup_path);

private:
    Logger();
    FILE* file;
};

#endif // VIEWER_LOGGER_H
