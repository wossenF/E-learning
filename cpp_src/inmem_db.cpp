#include "inmem_db.h"
#include <stdexcept>

namespace inmembase {

Table::Table(const std::string& name, const std::vector<ColumnDef>& cols)
    : name_(name), cols_(cols) {}

size_t Table::row_count() const { return rows_.size(); }

void Table::insert_row(const Row& r) {
    if (r.values.size() != cols_.size()) throw std::runtime_error("column count mismatch");
    // simple type checks (lightweight)
    rows_.push_back(r);
}

std::vector<Row> Table::select_all() const { return rows_; }

std::vector<Row> Table::select_where(std::function<bool(const Row&)> pred) const {
    std::vector<Row> out; for (const auto& r: rows_) if (pred(r)) out.push_back(r); return out;
}

bool Database::create_table(const std::string& name, const std::vector<ColumnDef>& cols) {
    if (tables_.find(name) != tables_.end()) return false;
    tables_.emplace(name, Table(name, cols));
    return true;
}

bool Database::drop_table(const std::string& name) {
    return tables_.erase(name) > 0;
}

std::optional<Table> Database::get_table(const std::string& name) const {
    auto it = tables_.find(name);
    if (it == tables_.end()) return std::nullopt;
    return it->second;
}

bool Database::insert_into(const std::string& table, const Row& r) {
    auto it = tables_.find(table);
    if (it == tables_.end()) return false;
    it->second.insert_row(r);
    return true;
}

std::vector<std::string> Database::list_tables() const {
    std::vector<std::string> out; out.reserve(tables_.size());
    for (const auto &p: tables_) out.push_back(p.first); return out;
}

} // namespace inmembase
