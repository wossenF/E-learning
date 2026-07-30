#include <chrono>
#include <functional>

// Tiny timing utilities to measure function execution time.
template<typename F, typename... Args>
static double time_function_ms(F&& f, Args&&... args) {
    using clock = std::chrono::high_resolution_clock;
    auto t0 = clock::now();
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    auto t1 = clock::now();
    std::chrono::duration<double, std::milli> diff = t1 - t0;
    return diff.count();
}
