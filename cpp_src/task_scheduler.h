#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <memory>

namespace temene {

class ThreadPool {
public:
  explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());
  ~ThreadPool();

  template<class F, class... Args>
  auto enqueue(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result<F, Args...>::type>;

  size_t activeWorkers() const { return workers_.size(); }

private:
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;

  mutable std::mutex queueMutex_;
  std::condition_variable cv_;
  bool stop_{false};
};

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
  -> std::future<typename std::invoke_result<F, Args...>::type> {
  using return_type = typename std::invoke_result<F, Args...>::type;

  auto task = std::make_shared<std::packaged_task<return_type()>>(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
  );

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(queueMutex_);
    if (stop_) throw std::runtime_error("Enqueue on stopped ThreadPool.");
    tasks_.emplace([task]() { (*task)(); });
  }
  cv_.notify_one();
  return res;
}

} // namespace temene

#endif // TASK_SCHEDULER_H
