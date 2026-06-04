#pragma once
#include <chrono>
#include <string>

enum class LogLevel { Debug=0, Info, Warn, Error, Fatal };

struct LogMessage {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    int thread_id;
    std::string text;
};
