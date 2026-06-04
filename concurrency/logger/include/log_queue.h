#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <semaphore>

#include "log_message.h"

template <bool Bounded, bool UseCV>
class LogQueue;

template <bool Bounded, bool UseCV>
class LogQueueBase {
   protected:
    std::queue<LogMessage> q;
    mutable std::mutex mtx;

    void guarded_push(LogMessage msg) {
        std::lock_guard lk(mtx);
        q.push(std::move(msg));
    }

    LogMessage guarded_pop() {
        std::lock_guard lk(mtx);
        LogMessage msg = std::move(q.front());
        q.pop();
        return msg;
    }
};

template <>
class LogQueue<false, true> : public LogQueueBase<false, true> {
    std::condition_variable cv;

   public:
    void push(LogMessage msg) {
        std::lock_guard lk(mtx);
        q.push(std::move(msg));
        cv.notify_one();
    }
    LogMessage pop() {
        std::unique_lock lk(mtx);
        cv.wait(lk, [this] { return !q.empty(); });
        LogMessage msg = std::move(q.front());
        q.pop();
        return msg;
    }
};

template <>
class LogQueue<true, true> : public LogQueueBase<true, true> {
    std::condition_variable cv_not_empty, cv_not_full;
    size_t capacity;

   public:
    explicit LogQueue(size_t cap) : capacity(cap) {}
    void push(LogMessage msg) {
        std::unique_lock lk(mtx);
        cv_not_full.wait(lk, [this] { return q.size() < capacity; });
        q.push(std::move(msg));
        cv_not_empty.notify_one();
    }
    LogMessage pop() {
        std::unique_lock lk(mtx);
        cv_not_empty.wait(lk, [this] { return !q.empty(); });
        LogMessage msg = std::move(q.front());
        q.pop();
        cv_not_full.notify_one();
        return msg;
    }
};

template <>
class LogQueue<false, false> : public LogQueueBase<false, false> {
    std::counting_semaphore<> items{0};

   public:
    void push(LogMessage msg) {
        guarded_push(std::move(msg));
        items.release();
    }
    LogMessage pop() {
        items.acquire();
        return guarded_pop();
    }
};

template <>
class LogQueue<true, false> : public LogQueueBase<true, false> {
    std::counting_semaphore<> empty_slots;
    std::counting_semaphore<> full_slots{0};

   public:
    explicit LogQueue(size_t capacity) : empty_slots(capacity) {}
    void push(LogMessage msg) {
        empty_slots.acquire();
        guarded_push(std::move(msg));
        full_slots.release();
    }
    LogMessage pop() {
        full_slots.acquire();
        LogMessage msg = guarded_pop();
        empty_slots.release();
        return msg;
    }
};
