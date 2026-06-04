#include <iostream>
#include <thread>
#include <vector>

#include "log_message.h"
#include "log_queue.h"
#include "log_thread.h"
#include "worker.h"

#ifndef QUEUE_TYPE
#define QUEUE_TYPE 1
#endif

#if QUEUE_TYPE == 1
using QueueType = LogQueue<false, true>;
#elif QUEUE_TYPE == 2
using QueueType = LogQueue<true, true>;
#elif QUEUE_TYPE == 3
using QueueType = LogQueue<false, false>;
#elif QUEUE_TYPE == 4
using QueueType = LogQueue<true, false>;
#else
#error "QUEUE_TYPE must be 1, 2, 3 or 4"
#endif

int main() {
    constexpr std::size_t kWorkers = 3;
    const std::string logFile = ".log";

#if QUEUE_TYPE == 2 || QUEUE_TYPE == 4
    constexpr size_t CAPACITY = 10;
    QueueType queue(CAPACITY);
#else
    QueueType queue;
#endif

    LogThread<QueueType> lh(queue, logFile);

    std::vector<std::jthread> workers;
    for (std::size_t i = 0; i < kWorkers; ++i)
        workers.emplace_back(worker<QueueType>, i, std::ref(queue), 10);

    for (auto& w : workers) w.join();

    return 0;
}
