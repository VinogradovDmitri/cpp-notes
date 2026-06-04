#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

template <typename T>
class threadsafe_queue {
   private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

   public:
    threadsafe_queue() {}
    threadsafe_queue(threadsafe_queue const& other) {
        std::lock_guard lk(other.mut);
        data_queue = other.data_queue;
    }
    void push(T val) {
        std::lock_guard lk(mut);
        data_queue.push(val);
        data_cond.notify_one();
    }
    void wait_and_pop(T& res) {
        std::unique_lock lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        res = data_queue.front();
        data_queue.pop();
    }
    shared_ptr<T> wait_and_pop() {
        std::unique_lock lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool try_pop(T& res) {
        std::lock_guard lk(mut);
        if (data_queue.empty()) return false;
        res = data_queue.front();
        data_queue.pop();
        return true;
    }
    shared_ptr<T> try_pop() {
        std::lock_guard lk(mut);
        if (data_queue.empty()) return std::shared_ptr<T>();
        shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool empty() const {
        std::lock_guard lk(mut);
        return data_queue.empty();
    }
};
