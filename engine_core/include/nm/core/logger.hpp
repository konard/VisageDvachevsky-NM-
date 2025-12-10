#pragma once

#include <string>
#include <string_view>
#include <fstream>
#include <mutex>
#include <memory>
#include <cstdio>

namespace nm::core
{

enum class LogLevel
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
    Off
};

class Logger
{
public:
    static Logger& instance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void setLevel(LogLevel level);
    [[nodiscard]] LogLevel getLevel() const;

    void setOutputFile(const std::string& path);
    void closeOutputFile();

    void log(LogLevel level, std::string_view message);

    void trace(std::string_view message);
    void debug(std::string_view message);
    void info(std::string_view message);
    void warning(std::string_view message);
    void error(std::string_view message);
    void fatal(std::string_view message);

private:
    Logger();
    ~Logger();

    [[nodiscard]] const char* levelToString(LogLevel level) const;
    [[nodiscard]] std::string getCurrentTimestamp() const;

    LogLevel m_level;
    std::ofstream m_fileStream;
    std::mutex m_mutex;
    bool m_useColors;
};

} // namespace nm::core

#define NM_LOG_TRACE(msg) nm::core::Logger::instance().trace(msg)
#define NM_LOG_DEBUG(msg) nm::core::Logger::instance().debug(msg)
#define NM_LOG_INFO(msg)  nm::core::Logger::instance().info(msg)
#define NM_LOG_WARN(msg)  nm::core::Logger::instance().warning(msg)
#define NM_LOG_ERROR(msg) nm::core::Logger::instance().error(msg)
#define NM_LOG_FATAL(msg) nm::core::Logger::instance().fatal(msg)
