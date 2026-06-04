#include <bits/stdc++.h>

constexpr uint_fast8_t CntOfMeasure = 30;

struct Value {
    std::string str;
    int num;
    Value() : str(), num(0) {}
    Value(std::string s, int n) : str(std::move(s)), num(n) {}
};

struct Node {
    int key;
    Value value;
    std::unique_ptr<Node> next;
    mutable std::mutex mtx;

    Node(int k, const Value& v) : key(k), value(v), next(nullptr) {}
};

class ParallelHashTable {
private:
    static constexpr size_t BUCKET_COUNT = 10'000'019;
    std::vector<std::unique_ptr<Node>> buckets;

    constexpr size_t hash(int key) const {
        return std::hash<int>{}(key) % BUCKET_COUNT;
    }
    void delete_chain(std::unique_ptr<Node> head) {
        while (head.get()) {
            head = std::move(head->next);
        }
    }

public:
    ParallelHashTable() : buckets(BUCKET_COUNT) {}

    ~ParallelHashTable() {
        for (auto& b : buckets)
            delete_chain(std::move(b));
    }

    bool contains(int key) const {
        size_t idx = hash(key);
        const auto& bucket = buckets[idx];
        if (!bucket) return false;

        Node* curr = bucket.get();
        std::unique_lock<std::mutex> lock_curr(curr->mtx);

        while (curr->key != key) {
            Node* next = curr->next.get();
            if (!next) return false;

            std::unique_lock<std::mutex> lock_next(next->mtx);
            lock_curr.unlock();
            
            curr = next;
            lock_curr = std::move(lock_next);
        }
        return true;
    }

    void put(int key, const Value& val) {
        size_t idx = hash(key);

        if (!buckets[idx]) {
            buckets[idx] = std::make_unique<Node>(key, val);
            return;
        }

        Node* curr = buckets[idx].get();
        std::unique_lock<std::mutex> lock_curr(curr->mtx);

        while (true) {
            if (curr->key == key) {
                curr->value = val;
                return;
            }

            if (!curr->next) break;

            Node* next = curr->next.get();
            std::unique_lock<std::mutex> lock_next(next->mtx);

            lock_curr.unlock();
            lock_curr = std::move(lock_next);
            curr = next;
        }

        curr->next = std::make_unique<Node>(key, val);
    }

    bool remove(int key) {
        size_t idx = hash(key);
        if (!buckets[idx]) return false;

        Node* curr = buckets[idx].get();
        std::unique_lock<std::mutex> lock_curr(curr->mtx);

        if (curr->key == key) {
            auto removed = std::move(buckets[idx]);
            if (removed->next) {
                std::unique_lock<std::mutex> lock_next(removed->next->mtx);
                buckets[idx] = std::move(removed->next);
            }
            return true;
        }

        Node* prev = curr;
        std::unique_lock<std::mutex> lock_prev(std::move(lock_curr));
        curr = prev->next.get();

        while (curr) {
            std::unique_lock<std::mutex> lock_curr(curr->mtx);

            if (curr->key == key) {
                auto removed = std::move(prev->next);
                prev->next = std::move(removed->next);
                return true;
            }

            if (!curr->next) break;

            std::unique_lock<std::mutex> lock_next(curr->next->mtx);
            lock_prev.unlock();
            prev = curr;
            lock_prev = std::move(lock_curr);
            curr = curr->next.get();
        }
        return false;
    }
};

void test_sequential_put(ParallelHashTable& table, int num_threads, int total_keys = 10'000'000) {
    std::vector<std::thread> threads;
    int per_thread = total_keys / num_threads;
    for (int t = 0; t < num_threads; ++t) {
        int start = t * per_thread + 1;
        int end = (t == num_threads - 1) ? total_keys : start + per_thread - 1;
        threads.emplace_back([&table, start, end]() {
            for (int k = start; k <= end; ++k)
                table.put(k, Value("key_" + std::to_string(k), k * 10));
        });
    }
    for (auto& th : threads) th.join();
    for (int k = 1; k <= total_keys; ++k)
        if (!table.contains(k)) std::cerr << k << " not found\n";
}

void test_random_operations(ParallelHashTable& table, int num_threads, int ops_per_thread = 1000) {
    std::vector<std::thread> threads;
    for (uint_fast8_t t = 0; t < num_threads; ++t) {
        threads.emplace_back([&table, t, ops_per_thread]() {
            thread_local std::minstd_rand gen(std::random_device{}() + 
                                              std::hash<std::thread::id>{}(std::this_thread::get_id()));
            std::uniform_int_distribution<uint_fast16_t> op_dist(0, 2);
            std::uniform_int_distribution<uint_fast16_t> key_dist(0, 1000);
            std::uniform_int_distribution<uint_fast16_t> val_dist(0, 1000);
            for (int i = 0; i < ops_per_thread; ++i) {
                uint_fast16_t key = key_dist(gen);
                uint_fast16_t op = op_dist(gen);
                if (op == 0) table.put(key, Value("rand_" + std::to_string(key), val_dist(gen)));
                else if (op == 1) table.remove(key);
                else table.contains(key);
            }
        });
    }
    for (auto& th : threads) th.join();
}

template<typename F, typename... Args>
auto measure(F&& f, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    for (uint_fast8_t _ = 0; _ < CntOfMeasure; ++_) {
        std::cout << '.' << std::flush;
        std::forward<F>(f)(std::forward<Args>(args)...);
    }
    auto end = std::chrono::high_resolution_clock::now();
    for (uint_fast8_t _ = 0; _ < CntOfMeasure; ++_) std::cout << "\b \b";
    return std::chrono::duration<float>(end - start).count() / CntOfMeasure;
}

int main(int argc, char* argv[]) {
    std::cout << std::fixed << std::setprecision(6);
    bool quiet = (argc > 1 && std::string(argv[1]) == "--quiet");

    uint_fast8_t num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 2;

    if (quiet) {
        ParallelHashTable ht;
        test_random_operations(ht, num_threads);
    } else {
        auto t1 = measure([](int threads) {
            ParallelHashTable ht;
            test_sequential_put(ht, threads);
        }, num_threads);
        std::cout << t1 << '\n';

        auto t2 = measure([](int threads) {
            ParallelHashTable ht;
            test_random_operations(ht, threads);
        }, num_threads);
        std::cout << t2 << '\n';
    }

    return 0;
}