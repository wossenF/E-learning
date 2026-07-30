#include "task_scheduler.h"

namespace temene {

ThreadPool::ThreadPool(size_t threads) {
  size_t count = threads > 0 ? threads : 4;
  for (size_t i = 0; i < count; ++i) {
    workers_.emplace_back([this] {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(this->queueMutex_);
          this->cv_.wait(lock, [this] {
            return this->stop_ || !this->tasks_.empty();
          });
          if (this->stop_ && this->tasks_.empty()) return;
          task = std::move(this->tasks_.front());
          this->tasks_.pop();
        }
        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queueMutex_);
    stop_ = true;
  }
  cv_.notify_all();
  for (std::thread& worker : workers_) {
    if (worker.joinable()) worker.join();
  }
}

} // namespace temene
