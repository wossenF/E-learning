#include <thread>
#include <vector>
#include <functional>

// Small threading helpers to launch a pool of workers and execute tasks.
void parallel_for(size_t workers, const std::function<void(size_t)>& fn) {
    if (workers == 0) workers = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    for (size_t i=0;i<workers;++i) threads.emplace_back([i, &fn]{ fn(i); });
    for (auto& t : threads) if (t.joinable()) t.join();
}
