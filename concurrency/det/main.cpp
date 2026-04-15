/*
Задача 1: на pthreads или std::thread написать параллельную программу вычисляющую детерминант 
матрицы по методу миноров. Пул потоков не используем, лимитируем уровень на котором запускаются 
потоки. Потом померить время в зависимости от количества потоков. Объяснить результаты.
*/

#include <bits/stdc++.h>

using namespace std;

using ll = long long;
#define DEBUG_MODE false

vector<vector<ll>> matrix;
atomic<int> t_cnt(1);

ll det_parallel(const vector<int>& rows, const vector<int>& cols, uint16_t max_threads) {
    size_t n = rows.size();
    if (n == 1) return matrix[rows[0]][cols[0]];
    if (n == 2) return matrix[rows[0]][cols[0]] * matrix[rows[1]][cols[1]] - matrix[rows[1]][cols[0]] * matrix[rows[0]][cols[1]];
    if (n == 3) return matrix[rows[0]][cols[0]] * (matrix[rows[1]][cols[1]] * matrix[rows[2]][cols[2]] - matrix[rows[1]][cols[2]] * matrix[rows[2]][cols[1]]) -
                       matrix[rows[0]][cols[1]] * (matrix[rows[1]][cols[0]] * matrix[rows[2]][cols[2]] - matrix[rows[1]][cols[2]] * matrix[rows[2]][cols[0]]) +
                       matrix[rows[0]][cols[2]] * (matrix[rows[1]][cols[0]] * matrix[rows[2]][cols[1]] - matrix[rows[1]][cols[1]] * matrix[rows[2]][cols[0]]);

    vector<ll> minor(n, 0);
    vector<thread> threads;

    for (size_t i = 0; i < n; ++i) {
        vector<int> rows_(rows.begin() + 1, rows.end());
        vector<int> cols_;
        cols_.reserve(n - 1);
        for (size_t k = 0; k < n; ++k) if (k != i) cols_.push_back(cols[k]);

        if (n >= 10 && t_cnt.load() < max_threads) {
            int cur = t_cnt.load();
            bool created = false;
            while (cur < max_threads) {
                if (t_cnt.compare_exchange_weak(cur, cur + 1)) {
                    threads.emplace_back([&, rows_ = move(rows_), cols_ = move(cols_), i]() {
                        minor[i] = det_parallel(rows_, cols_, max_threads);
                        t_cnt.fetch_sub(1);
                    });
                    created = true;
                    break;
                }
            }
            if (created) continue;
        }
        minor[i] = det_parallel(rows_, cols_, max_threads);
    }
    for (auto& t : threads) t.join();

    ll res = 0;
    for (size_t j = 0; j < n; ++j)
        res += (j & 1 ? -1LL : 1LL) * matrix[rows[0]][cols[j]] * minor[j];

    return res;
}

double run_measure(uint16_t max_threads, const vector<int>& all_rows, const vector<int>& all_cols, bool print = false) {
    auto start = chrono::high_resolution_clock::now();
    ll res = det_parallel(all_rows, all_cols, max_threads);
    if (print) cout << res << "\n";
    auto end = chrono::high_resolution_clock::now();
    return static_cast<chrono::duration<double>>(end - start).count();
}

int main(int argc, char* argv[]) {
    ifstream cin("input.txt");
    bool quiet = (argc > 1 && strcmp(argv[1], "--quiet") == 0);
    bool test = (argc > 1 && strcmp(argv[1], "--test") == 0);

    atomic<bool> monitor_active{true};
    if (DEBUG_MODE){
        thread monitor([&]() {
            while (monitor_active) {
                this_thread::sleep_for(chrono::seconds(1));
                cerr << "t_cnt = " << t_cnt.load() << endl;
            }
        });
        monitor.detach();
    }

    int n; cin >> n;
    matrix.assign(n, vector<ll>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            cin >> matrix[i][j];

    vector<int> all_rows(n), all_cols(n);
    for (int i = 0; i < n; ++i) {
        all_rows[i] = i; all_cols[i] = i;
    }

    uint16_t max_possible = thread::hardware_concurrency();
    if (max_possible == 0) max_possible = 8;

    if (quiet) {
        run_measure(max_possible, all_rows, all_cols);
    } else if (test) {
        for (uint16_t threads = 2; threads <= 2*max_possible+1; ++threads) {
            double total = 0.0;
            for (int _ = 0; _ < 10; ++_) total += run_measure(threads, all_rows, all_cols);
            double avg = total / 10.0;
            cout << fixed << setprecision(8) << threads << "\t" << avg << endl;
        }
    } else {
        run_measure(1, all_rows, all_cols, true);
    }

    if (DEBUG_MODE){
        monitor_active = false;
        this_thread::sleep_for(chrono::seconds(2));
    }

    return 0;
}
