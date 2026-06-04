#pragma once
#include <ctime>
#include <fstream>
#include <iomanip>
#include <stop_token>
#include <string>
#include <thread>

#include "log_message.h"

template <typename Queue>
class LogThread {
   private:
    Queue& q_;
    std::ofstream logfile_;
    std::jthread thread_;

   public:
    LogThread(Queue& q, const std::string& filename)
        : q_(q), logfile_(filename, std::ios::app) {
        if (!logfile_) throw "err";

        thread_ = std::jthread(&LogThread::run, this);
    }
    ~LogThread() {
        q_.push(LogMessage{std::chrono::system_clock::now(), LogLevel::Fatal,
                           -1, "shutdown"});
        logfile_.close();
    }

    LogThread(const LogThread&) = delete;
    LogThread& operator=(const LogThread&) = delete;

   private:
    std::string level_to_str(LogLevel lvl) {
        switch (lvl) {
            case LogLevel::Debug:
                return "DEBUG";
            case LogLevel::Info:
                return "INFO ";
            case LogLevel::Warn:
                return "WARN ";
            case LogLevel::Error:
                return "ERROR";
            case LogLevel::Fatal:
                return "FATAL";
            default:
                return "????";
        }
    }

    void write_to_file(std::ofstream& file, const LogMessage& msg) {
        auto t = std::chrono::system_clock::to_time_t(msg.timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      msg.timestamp.time_since_epoch()) %
                  1000;

        file << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S") << '.'
             << std::setfill('0') << std::setw(3) << ms.count() << '\t'
             << level_to_str(msg.level) << "\t[Thread " << msg.thread_id
             << "]\t" << msg.text << "\n";
    }

    void run() {
        while (true) {
            LogMessage msg = q_.pop();
            if (msg.thread_id == -1) break;
            write_to_file(logfile_, msg);
        }
    }
};
