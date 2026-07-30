#include "inmem_db.h"
#include <vector>
#include <string>
#include <mutex>
#include <map>
#include <optional>

namespace inmembase {

// Lightweight storage engine abstraction: manages tables and provides
// basic persistence hooks (delegating to Persistence). The engine exposes
// a clean API for the planner/CLI to interact with.

class StorageEngine {
public:
    StorageEngine() {}
    bool create_table(Database& db, const std::string& name, const std::vector<ColumnDef>& cols) {
        std::unique_lock lk(mu_);
        return db.create_table(name, cols);
    }
    bool drop_table(Database& db, const std::string& name) { std::unique_lock lk(mu_); return db.drop_table(name); }
    bool insert_row(Database& db, const std::string& table, const Row& r) { std::unique_lock lk(mu_); return db.insert_into(table, r); }
    std::optional<Table> get_table(Database& db, const std::string& name) { std::shared_lock lk(mu_); return db.get_table(name); }
private:
    std::shared_mutex mu_;
};

} // namespace inmembase
