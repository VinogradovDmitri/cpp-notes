/*
Задача 3: на pthreads или std::thread реализовать многопоточную хеш-таблицу с методом 
разрешения коллизий в виде цепочек. Она должна поддерживать следующий набор операций:
1)Put
2)Remove
3)Check

Для простоты считать что ключи int, а значения --- класс с двумя полями (строкой и числом). 
Списки должны быть свои, мутекс не должен быть на целую цепочку или на всю структуру. 
Максимизируем параллельность. Для тестирования реализовать несколько сценариев:
Последовательно добавляем 1...1000 при наличии N потоков. Потом, проверить что все положилось.
Стучимся из N потоков случайно добавляя/удаляя/проверяя случайное число из диапазона [1..M]. 
Продемонстрировать на valgrind что есть чистая выдача.
*/
#include <bits/stdc++.h>

constexpr uint_fast8_t CntOfMeasure = 5;

struct Value {
    std::string str;
    int num;
    Value() : str(), num(0) {}
    Value(std::string s, int n) : str(std::move(s)), num(n) {}
};

struct Node {
    int key;
    Value value;
    std::shared_ptr<Node> next;
    mutable std::mutex mtx;
    Node(int k, const Value& v) : key(k), value(v), next(nullptr) {}
};

class ParallelHashTable {
private:
    static constexpr size_t BUCKET_COUNT = 10'000'019; // 10'000'019 1'000'003
    std::vector<std::shared_ptr<Node>> buckets;

    constexpr size_t hash(int key) const {
        return std::hash<int>{}(key) % BUCKET_COUNT;
    }
    void delete_chain(std::shared_ptr<Node> head) {
        while (head) {
            std::shared_ptr<Node> next = head->next;
            head->next.reset();
            head = next;
        }
    }

public:
    ParallelHashTable() : buckets(BUCKET_COUNT) {
        for (auto& b : buckets)
            b = std::make_shared<Node>(-1, Value{});
    }
    ~ParallelHashTable() {
        for (size_t i = 0; i < buckets.size(); ++i)
            delete_chain(buckets[i]);
    }

    bool contains(int key) {
        size_t idx = hash(key);
        std::shared_ptr<Node> curr = buckets[idx];
        std::unique_lock<std::mutex> lock_curr(curr->mtx);

        while (curr) {
            if (curr->key == key)
                return true;

            std::shared_ptr<Node> next = curr->next;
            if (!next) break;

            std::unique_lock<std::mutex> lock_next(next->mtx);
            lock_curr.unlock();
            curr = next;
            lock_curr = std::move(lock_next);
        }
        return false;
    }

    void put(int key, const Value& val) {
        size_t idx = hash(key);
        std::shared_ptr<Node> newNode = std::make_shared<Node>(key, val);

        std::shared_ptr<Node> head = buckets[idx];
        std::unique_lock<std::mutex> lock_prev(head->mtx);
        std::shared_ptr<Node> prev = head;
        std::shared_ptr<Node> curr = prev->next;
        std::unique_lock<std::mutex> lock_curr;
        if (curr) lock_curr = std::unique_lock<std::mutex>(curr->mtx);

        while (curr) {
            if (curr->key == key) {
                curr->value = val;
                return;
            }

            std::shared_ptr<Node> next = curr->next;
            if (!next) break;

            std::unique_lock<std::mutex> lock_next(next->mtx);
            lock_prev.unlock();
            prev = curr;
            lock_prev = std::move(lock_curr);
            curr = next;
            lock_curr = std::move(lock_next);
        }

        newNode->next = curr;
        prev->next = newNode;
    }

    bool remove(int key) {
        size_t idx = hash(key);
        std::shared_ptr<Node> head = buckets[idx];
        if (!head) return false;

        std::unique_lock<std::mutex> lock_prev(head->mtx);
        std::shared_ptr<Node> prev = head;
        std::shared_ptr<Node> curr = prev->next;
        std::unique_lock<std::mutex> lock_curr;
        if (curr) lock_curr = std::unique_lock<std::mutex>(curr->mtx);

        while (curr) {
            if (curr->key == key) {
                prev->next = curr->next;
                return true;
            }

            std::shared_ptr<Node> next = curr->next;
            if (!next) break;

            std::unique_lock<std::mutex> lock_next(next->mtx);
            lock_prev.unlock();
            prev = curr;
            lock_prev = std::move(lock_curr);
            curr = next;
            lock_curr = std::move(lock_next);
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
