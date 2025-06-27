#pragma once
#ifndef VIEWER_LOGGER_H
#define VIEWER_LOGGER_H

#include <cstdio>

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
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
class Logger final
{
public:
    static Logger& instance() noexcept;
    ~Logger() noexcept;

    void openFile(const char* path, const char* backup_path);

    void log_message(LogLevel level, const char* file, int line, const char* format, ...) const;

public:
    LogLevel level = LOG_LEVEL_INFO;

private:
    Logger() noexcept = default;

    void print_ansi_escape_code(LogLevel level, std::FILE* stream) const;

private:
    std::FILE* file = nullptr;
};

#endif // VIEWER_LOGGER_H
