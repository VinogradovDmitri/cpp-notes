#include <mutex>
#include <stdexcept>

class hierarchical_mutex {
    std::mutex internal_mutex;
    unsigned long const val;
    unsigned long prev_val;
    static thread_local unsigned long this_thread_val;

    void check() {
        if (this_thread_val <= val)
            throw std::logic_error("mutex hierarchy violated");
    }

    void update_val() {
        prev_val = this_thread_val;
        this_thread_val = val;
    }

   public:
    explicit hierarchical_mutex(unsigned long val) : val(val), prev_val(0) {}
    void lock() {
        check();
        internal_mutex.lock();
        update_val();
    }
    void unlock() {
        if (this_thread_val != val)
            throw std::logic_error("mutex hierarchy violated");
        this_thread_val = prev_val;
        internal_mutex.unlock();
    }
    bool try_lock() {
        check();
        if (!internal_mutex.try_lock()) return false;
        update_val();
        return true;
    }
};
thread_local unsigned long hierarchical_mutex::this_thread_val(ULONG_MAX);
