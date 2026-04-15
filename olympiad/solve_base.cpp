#include <bits/stdc++.h>
#include <typeinfo>
 
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

#pragma GCC optimize("O3","unroll-loops")
#pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")

using namespace std;
using namespace __gnu_pbds;
//
typedef tree<
     int,
     null_type,
     less<int>,
     rb_tree_tag,
     tree_order_statistics_node_update>
     ordered_set;
typedef long long ll;
typedef long double ld;
//
#define int ll
#define double ld
//
#define inf INT_MAX
#define sz(a) (int)a.size()
// #define clear(arr) memset(arr, 0, sizeof(arr))
#define all(a) a.begin(), a.end()
#define ceil(a) (int)ceil((double)a) // (num + del-1)/del*del
#define round(a) (int)round((double)a)
#define trunc(a) (int)trunc((double)a)
#define pb push_back
#define ff first
#define ss second
#define cin_arr(arr) for (auto& _ : arr) cin >> _
#define cout_arr(arr) for (auto& _ : arr) cout << _ << " "

vector<int> primes;

class RandomGenerator {
private:
    std::mt19937 gen;
    
public:
    RandomGenerator() : gen(std::random_device{}()) {}
    
    int random_int(int min = 0, int max = 100) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen);
    }
    
    double random_double(double min = 0.0, double max = 100.0) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(gen);
    }};
RandomGenerator rnd;

void primes_nums(int n = 2*1e5) {
    vector<int> lp(n + 1, 0);
    
    for (int i = 2; i <= n; i++) {
        if (lp[i] == 0) {
            lp[i] = i;
            primes.push_back(i);
        }
        
        for (int j = 0; j < sz(primes) && primes[j] <= lp[i] && i * primes[j] <= n; j++) {
            lp[i * primes[j]] = primes[j];
        }
    }
    
    return;}

string strip(const std::string &inpt){
    auto start_it = inpt.begin();
    auto end_it = inpt.rbegin();
    while (std::isspace(*start_it))
        ++start_it;
    if (start_it != inpt.end()) { 
       while (std::isspace(*end_it))
          ++end_it;
     }
    return std::string(start_it, end_it.base());}

int bin_pow(int a, int b, int m){
    if (b == 0) return 1LL;
    if (b == 1) return a%m;
    if (b % 2 == 1) return (bin_pow(a, b-1LL, m)*a)%m;
    int x = bin_pow(a, b/2, m)%m;
    return (x*x)%m;
}

bool FermaTest(int n){
    cout << n << endl;
    if (n % 2 == 0) return false;
    int a;
    for (int i = 0; i < 1e4; i++){
        a = rnd.random_int(2, min(n - 2LL, (int)1e6));
        if (bin_pow(a, n - 1, n) != 1) {
            cout << bin_pow(a, n - 1, n) << " |" << a << " \n";
            return false;
        }
    } 
    return true;
}



ll mod = 998244353;
bool debug = false;
bool stress = false;


void solve() {
    
}


signed main() {
    ios_base::sync_with_stdio(false);
    std::cout.sync_with_stdio(false);
    cin.tie(nullptr); // ifstream cin(input.txt);
    cout.tie(nullptr); // ofstream cout(output.txt);
    primes_nums();
 
    // solve();
    int o=1;// cin >> o;if (stress) o = 1e5;

    
    // int o_ = o;
    while (o--){
        //int n;
        if (stress){
            //
        }else{
            //cin >> n;
            //int q; cin >> q;
            solve();
        }
    }
 
    cout << fixed << setprecision(10);
    // cerr << "Time:" << 1000*((double)clock())/(double)CLOCKS_PER_SEC << "ms\n";
    if (debug) cout << "---------DEBUG--------\n";
    if (stress) cout << "---------STRESS--------\n";

    return 0;
}
