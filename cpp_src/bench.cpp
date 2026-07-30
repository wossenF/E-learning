#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>

double time_sum_loop(size_t n) {
    using clock = std::chrono::high_resolution_clock;
    std::vector<int> v(n);
    for (size_t i=0;i<n;++i) v[i] = (int)i;
    auto t0 = clock::now();
    long long s = 0; for (size_t i=0;i<n;++i) s += v[i];
    auto t1 = clock::now();
    std::chrono::duration<double> d = t1 - t0;
    (void)s; return d.count()*1000.0;
}

int main_bench() {
    std::cout << "bench: " << time_sum_loop(1000000) << " ms\n";
    return 0;
}
