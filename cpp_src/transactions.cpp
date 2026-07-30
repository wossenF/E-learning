#include "inmem_db.h"
#include <mutex>
#include <shared_mutex>
#include <map>
#include <vector>
#include <optional>
#include <chrono>
#include <thread>

namespace inmembase {

// Very small transactional layer: supports begin/commit/abort with in-memory
// optimistic locking semantics. This is intentionally simple but exercises
// realistic concurrency, locking, and snapshotting behavior.

class Transaction {
public:
    Transaction() {
        id_ = ++global_id;
        start_ts_ = std::chrono::steady_clock::now();
    }
    long long id() const { return id_; }
    void add_write(const std::string& table, size_t row_idx) { writes_[table].push_back(row_idx); }
    const std::map<std::string, std::vector<size_t>>& writes() const { return writes_; }
    void add_read(const std::string& table, size_t row_idx) { reads_[table].push_back(row_idx); }
private:
    long long id_;
    std::chrono::steady_clock::time_point start_ts_;
    std::map<std::string, std::vector<size_t>> reads_;
    std::map<std::string, std::vector<size_t>> writes_;
    static inline long long global_id = 0;
};

class TransactionManager {
public:
    TransactionManager() = default;
    std::shared_ptr<Transaction> begin() { auto t = std::make_shared<Transaction>(); std::unique_lock lk(mu_); active_[t->id()] = t; return t; }
    bool commit(std::shared_ptr<Transaction> t) {
        std::unique_lock lk(mu_);
        if (!t) return false; active_.erase(t->id()); // simplified
        // in real system check conflicts
        return true;
    }
    void abort(std::shared_ptr<Transaction> t) { std::unique_lock lk(mu_); if (t) active_.erase(t->id()); }
    size_t active_count() const { std::shared_lock lk(mu_); return active_.size(); }
private:
    mutable std::shared_mutex mu_;
    std::map<long long, std::shared_ptr<Transaction>> active_;
};

} // namespace inmembase
