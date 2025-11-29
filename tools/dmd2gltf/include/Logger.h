#ifndef DMD2GLTF_LOGGER_H
#define DMD2GLTF_LOGGER_H

#include <cstdio>

#define LOG_DEBUG(...) LOG_MESSAGE(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...)  LOG_MESSAGE(LOG_LEVEL_INFO,  __VA_ARGS__)
#define LOG_WARN(...)  LOG_MESSAGE(LOG_LEVEL_WARN,  __VA_ARGS__)
#define LOG_ERROR(...) LOG_MESSAGE(LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(...) LOG_MESSAGE(LOG_LEVEL_FATAL, __VA_ARGS__)

#define LOG_MESSAGE(log_level, ...) Logger::instance().log_message(log_level, __FILE__, __LINE__, __VA_ARGS__)

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
    static Logger& instance() noexcept;
    ~Logger() noexcept;

    void openFile(const char* path, const char* backup_path);

    void log_message(LogLevel level, const char* file, int line, const char* format, ...) const;

    LogLevel level = LOG_LEVEL_INFO;

private:
    Logger() noexcept = default;

    int streams_count = 1;
    std::FILE* streams[2] = {stderr, nullptr};
};

#endif // DMD2GLTF_LOGGER_H
