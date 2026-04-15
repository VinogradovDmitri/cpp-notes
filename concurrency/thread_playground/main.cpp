/*
Пример с пары: разобрал пример в котором может возникать дедлок:
допустим есть несколько потоков работающих над одним одномерным массивом, 
каждый из которых выбирает произвольные три элемента, складывает их и записывает 
на место каждого из них их сумму. Были рассказаны четыре варианта предотвращения дедлоков:
a)Мутекс бОльшей гранулярности
b)Trylock с откатом
c)Диспетчер
d)Протокол: набор замков в правильном порядке

Задача 2: на pthreads или std::thread написать несколько вариантов для параллельной программы из примера приведенного на паре:
Наивная реализация без mutex, показать что valgrind ругается
Реализация a), показать что valgrind не ругается
Реализация b), показать что valgrind не ругается
Реализация d), показать что valgrind не ругается
Сравнить производительность в каждом случае на нормальном количестве потоков.
*/
#include <bits/stdc++.h>

using ll = long long;

constexpr int CntOfMeasure = 1;
constexpr int ArraySize = 100'000;
inline int random_int(int min = 0, int max = ArraySize-1) {
    thread_local std::minstd_rand gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

struct ArrNode {
    std::mutex glob_mut;

    struct alignas(std::hardware_destructive_interference_size) PaddedMutex { std::mutex mutex; };
    std::array<PaddedMutex, ArraySize> per_mut;
    
    struct alignas(std::hardware_destructive_interference_size) PaddedLong { ll value; };
    std::array<PaddedLong, ArraySize> array = []{
        std::array<PaddedLong, ArraySize> arr;
        for (auto& v : arr) v.value = random_int();
        return arr;
    }();
} arr_node;

template<typename F>
auto measure(F&& f, uint8_t cnt_threads, int total = 100'000'000, uint8_t cnt = CntOfMeasure) {
    auto start = std::chrono::high_resolution_clock::now();

    uint8_t cnt_ = cnt;
    while (cnt_--) {
        std::cout << '.' << std::flush;
        std::forward<F>(f)(cnt_threads, total);
    }
    for (int i = 0; i < cnt; ++i) std::cout << "\b \b";

    auto end = std::chrono::high_resolution_clock::now();
    return static_cast<std::chrono::duration<double>>(end - start).count()/cnt;
}

void solve_linear([[maybe_unused]]const int cnt_threads, const int total) {
    for (int i = 0; i < total; i++){
        int a = random_int(), b = random_int(), c = random_int();
        ll sum = arr_node.array[a].value + arr_node.array[b].value + arr_node.array[c].value;
        arr_node.array[a].value = sum;
        arr_node.array[b].value = sum;
        arr_node.array[c].value = sum;
    }
}

void solve_no_mutex(const int cnt_threads, const int total) {
    std::vector<std::thread> threads;
    threads.reserve(cnt_threads + 5);
    std::atomic<int> task_index{0};
    constexpr int chunk_size = 1024;

    auto worker = [&] {
        while (true) {
            int start = task_index.fetch_add(chunk_size, std::memory_order_relaxed); if (start >= total) break;
            int end = std::min(start + chunk_size, total);

            for (int _ = start; _ < end; _++){
                int a = random_int(), b = random_int(), c = random_int();
                ll sum = arr_node.array[a].value + arr_node.array[b].value + arr_node.array[c].value;
                arr_node.array[a].value = sum;
                arr_node.array[b].value = sum;
                arr_node.array[c].value = sum;
            }
        }
    };

    for (int i = 0; i < cnt_threads; i++) threads.emplace_back(worker);
    for (auto& t : threads) t.join();
}

void solve_no_mutex_for_daun(const int cnt_threads, const int total) {
    std::vector<std::thread> threads;
    threads.reserve(cnt_threads);
    std::atomic<int> task_index{0};

    auto worker = [&] {
        while (true) {
            int idx = task_index.fetch_add(1, std::memory_order_relaxed);
            if (idx >= total) break;

            int a = 0, b = 1, c = 2;
            ll sum = arr_node.array[a].value + arr_node.array[b].value + arr_node.array[c].value;
            arr_node.array[a].value = sum;
            arr_node.array[b].value = sum;
            arr_node.array[c].value = sum;
        }
    };

    for (int i = 0; i < cnt_threads; i++) threads.emplace_back(worker);
    for (auto& t : threads) t.join();
}

void solve_global_lock(const int cnt_threads, const int total) {
    std::vector<std::thread> threads;
    threads.reserve(cnt_threads + 5);
    std::atomic<int> task_index{0};
    constexpr int chunk_size = 1024;

    auto worker = [&] {
        while (true) {
            int start = task_index.fetch_add(chunk_size, std::memory_order_relaxed); if (start >= total) break;
            int end = std::min(start + chunk_size, total);
            
            std::lock_guard lock(arr_node.glob_mut);
            for (int _ = start; _ < end; _++) {
                int a = random_int(), b = random_int(), c = random_int();
                ll sum = arr_node.array[a].value + arr_node.array[b].value + arr_node.array[c].value;
                arr_node.array[a].value = sum;
                arr_node.array[b].value = sum;
                arr_node.array[c].value = sum;
            }
        }
    };

    for (int i = 0; i < cnt_threads; i++) threads.emplace_back(worker);
    for (auto& t : threads) t.join();
}

void solve_trylock(const int cnt_threads, const int total) {
    std::vector<std::thread> threads;
    threads.reserve(cnt_threads + 5);
    std::atomic<int> task_index{0};
    constexpr int chunk_size = 1024;

    auto worker = [&] {
        while (true) {
            int start = task_index.fetch_add(chunk_size, std::memory_order_relaxed); if (start >= total) break;
            int end = std::min(start + chunk_size, total);

            for (int _ = start; _ < end; _++){
                int a = random_int(), b = random_int(), c = random_int();
                if (a > b) std::swap(a, b); 
                if (b > c) std::swap(b, c);
                if (a > b) std::swap(a, b); 
                // a <= b <= c

                while (true) {
                    std::unique_lock l1(arr_node.per_mut[a].mutex, std::defer_lock);
                    std::unique_lock l2(arr_node.per_mut[b].mutex, std::defer_lock);
                    std::unique_lock l3(arr_node.per_mut[c].mutex, std::defer_lock);

                    if (a == b && b == c) {
                        if (l1.try_lock()) {
                            arr_node.array[a].value = arr_node.array[a].value * 3;
                            break;
                        }
                    } else if (a == b) {
                        if (std::try_lock(l1, l3) == -1) {
                            ll sum = arr_node.array[a].value * 2 + arr_node.array[c].value;
                            arr_node.array[a].value = sum;
                            arr_node.array[c].value = sum;
                            break;
                        }
                    } else if (b == c) {
                        if (std::try_lock(l1, l2) == -1) {
                            ll sum = arr_node.array[a].value + arr_node.array[b].value * 2;
                            arr_node.array[a].value = sum;
                            arr_node.array[b].value = sum;
                            break;
                        }
                    } else {
                        if (std::try_lock(l1, l2, l3) == -1) {
                            ll sum = arr_node.array[a].value + arr_node.array[b].value + arr_node.array[c].value;
                            arr_node.array[a].value = sum;
                            arr_node.array[b].value = sum;
                            arr_node.array[c].value = sum;
                            break;
                        }
                    }
                    std::this_thread::yield();
                }
            }
        }
    };

    for (int i = 0; i < cnt_threads; i++) threads.emplace_back(worker);
    for (auto& t : threads) t.join();
}

void solve_ordered_locks(const int cnt_threads, const int total) {
    std::vector<std::thread> threads;
    threads.reserve(cnt_threads + 5);
    std::atomic<int> task_index{0};
    constexpr int chunk_size = 1024;

    auto worker = [&] {
        while (true) {
            int start = task_index.fetch_add(chunk_size, std::memory_order_relaxed); if (start >= total) break;
            int end = std::min(start + chunk_size, total);
                
            for (int _ = start; _ < end; _++){
                int a = random_int(), b = random_int(), c = random_int();
                if (a > b) std::swap(a, b);
                if (b > c) std::swap(b, c);
                if (a > b) std::swap(a, b);

                if (a == b && b == c) {
                    arr_node.per_mut[a].mutex.lock();
                    // std::lock_guard lock(arr_node.per_mut[a].mutex);
                    arr_node.array[a].value = arr_node.array[a].value * 3;

                    arr_node.per_mut[a].mutex.unlock();
                } else if (a == b) {
                    arr_node.per_mut[a].mutex.lock();
                    arr_node.per_mut[c].mutex.lock();
                    // std::scoped_lock lock(arr_node.per_mut[a].mutex, arr_node.per_mut[c].mutex);
                    ll sum = arr_node.array[a].value * 2 + arr_node.array[c].value;
                    arr_node.array[a].value = sum;
                    arr_node.array[c].value = sum;

                    arr_node.per_mut[a].mutex.unlock();
                    arr_node.per_mut[c].mutex.unlock();
                } else if (b == c) {
                    arr_node.per_mut[a].mutex.lock();
                    arr_node.per_mut[b].mutex.lock();
                    // std::scoped_lock lock(arr_node.per_mut[a].mutex, arr_node.per_mut[b].mutex);
                    ll sum = arr_node.array[a].value + arr_node.array[b].value * 2;
                    arr_node.array[a].value = sum;
                    arr_node.array[b].value = sum;

                    arr_node.per_mut[a].mutex.unlock();
                    arr_node.per_mut[b].mutex.unlock();
                } else {
                    arr_node.per_mut[a].mutex.lock();
                    arr_node.per_mut[b].mutex.lock();
                    arr_node.per_mut[c].mutex.lock();
                    // std::scoped_lock lock(arr_node.per_mut[a].mutex, arr_node.per_mut[b].mutex, arr_node.per_mut[c].mutex);
                    ll sum = arr_node.array[a].value + arr_node.array[b].value + arr_node.array[c].value;
                    arr_node.array[a].value = sum;
                    arr_node.array[b].value = sum;
                    arr_node.array[c].value = sum;

                    arr_node.per_mut[a].mutex.unlock();
                    arr_node.per_mut[b].mutex.unlock();
                    arr_node.per_mut[c].mutex.unlock();
                }
            }
        }
    };

    for (int i = 0; i < cnt_threads; i++) threads.emplace_back(worker);
    for (auto& t : threads) t.join();
}

int main(int argc, char* argv[]) {
    std::cout << std::fixed << std::setprecision(8);
    const bool quiet = (argc > 1 && strcmp(argv[1], "--quiet") == 0);

    uint8_t max_possible = std::thread::hardware_concurrency()-1;
    if (max_possible == 0) max_possible = 8;

    if (quiet) {
        measure(solve_no_mutex_for_daun, max_possible, 5121);
        measure(solve_no_mutex, max_possible, 5121);
        measure(solve_global_lock, max_possible, 5121);
        measure(solve_trylock, max_possible, 5121);
        measure(solve_ordered_locks, max_possible, 5121);
    } else {
        std::cout << "solve_linear: " << measure(solve_linear, max_possible) << std::endl;
        std::cout << "solve_no_mutex: " << measure(solve_no_mutex, max_possible) << std::endl;
        std::cout << "solve_global_lock: " << measure(solve_global_lock, max_possible) << std::endl;
        std::cout << "solve_trylock: " << measure(solve_trylock, max_possible) << std::endl;
        std::cout << "solve_ordered_locks: " << measure(solve_ordered_locks, max_possible) << std::endl;
    }

    return 0;
}
