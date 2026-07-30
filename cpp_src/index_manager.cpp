#include "inmem_db.h"
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>

namespace inmembase {

// Simple secondary index manager: supports creating an index on a single
// column and using it to accelerate equality lookups. Index is kept in-memory
// as a map<key, vector<row_idx>>. This demonstrates indexing logic and
// mutation handling.

class Index {
public:
    Index() = default;
    Index(const std::string& col): column(col) {}
    void build(const Table& tbl) {
        index_.clear();
        const auto& cols = tbl.columns(); size_t colidx = 0; for (size_t i=0;i<cols.size();++i) if (cols[i].name==column) { colidx = i; break; }
        auto rows = tbl.select_all(); for (size_t i=0;i<rows.size();++i) {
            if (colidx < rows[i].values.size()) {
                std::string key = val_to_string(rows[i].values[colidx]); index_[key].push_back(i);
            }
        }
    }
    std::vector<size_t> lookup(const std::string& key) const {
        auto it = index_.find(key); if (it==index_.end()) return {}; return it->second;
    }
    std::string column;
private:
    static std::string val_to_string(const Value& v) {
        if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v);
        if (std::holds_alternative<long long>(v)) return std::to_string(std::get<long long>(v));
        if (std::holds_alternative<double>(v)) return std::to_string(std::get<double>(v));
        if (std::holds_alternative<bool>(v)) return std::get<bool>(v)?"true":"false";
        return std::string();
    }
    std::map<std::string,std::vector<size_t>> index_;
};

class IndexManager {
public:
    IndexManager() = default;
    bool create_index(const std::string& table, const std::string& column) {
        std::string key = table+"::"+column; if (indexes_.find(key) != indexes_.end()) return false; indexes_[key] = Index(column); return true;
    }
    bool drop_index(const std::string& table, const std::string& column) { std::string key=table+"::"+column; return indexes_.erase(key)>0; }
    void build_for_table(const std::string& table, const Table& t) {
        for (auto &p : indexes_) {
            if (p.first.rfind(table+"::",0)==0) p.second.build(t);
        }
    }
    std::optional<std::vector<size_t>> lookup(const std::string& table, const std::string& column, const std::string& key) const {
        std::string k = table+"::"+column; auto it = indexes_.find(k); if (it==indexes_.end()) return std::nullopt; return it->second.lookup(key);
    }
private:
    std::map<std::string, Index> indexes_;
};

} // namespace inmembase
