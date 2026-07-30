// inmem_db.h
#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <optional>

namespace inmembase {

using Value = std::variant<std::nullptr_t, bool, long long, double, std::string>;

struct Row {
    std::vector<Value> values;
};

struct ColumnDef {
    std::string name;
    enum Type { INT, DOUBLE, STRING, BOOL } type;
    bool nullable = true;
};

class Table {
public:
    Table() = default;
    Table(const std::string& name, const std::vector<ColumnDef>& cols);
    const std::string& name() const { return name_; }
    const std::vector<ColumnDef>& columns() const { return cols_; }
    size_t row_count() const;
    void insert_row(const Row& r);
    std::vector<Row> select_all() const;
    std::vector<Row> select_where(std::function<bool(const Row&)> pred) const;

private:
    std::string name_;
    std::vector<ColumnDef> cols_;
    std::vector<Row> rows_;
};

class Database {
public:
    Database() = default;
    bool create_table(const std::string& name, const std::vector<ColumnDef>& cols);
    bool drop_table(const std::string& name);
    std::optional<Table> get_table(const std::string& name) const;
    bool insert_into(const std::string& table, const Row& r);
    std::vector<std::string> list_tables() const;

private:
    std::map<std::string, Table> tables_;
};

} // namespace inmembase
