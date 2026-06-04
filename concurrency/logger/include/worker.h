#pragma once
#include <chrono>
#include <random>
#include <thread>

#include "log_message.h"

template <typename Queue>
void worker(int id, Queue& q, int num_messages = 5) {
    std::mt19937 rng(id);
    std::uniform_int_distribution<int> delay(100, 400);

    static constexpr LogLevel levels[] = {LogLevel::Debug, LogLevel::Info,
                                          LogLevel::Warn, LogLevel::Error,
                                          LogLevel::Fatal};
    std::uniform_int_distribution<int> level_dist(0, std::size(levels) - 1);

    for (int i = 0; i < num_messages; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay(rng)));
        auto level = levels[level_dist(rng)];
        q.push(LogMessage{std::chrono::system_clock::now(), level, id,
                          "message " + std::to_string(i)});
    }
}
