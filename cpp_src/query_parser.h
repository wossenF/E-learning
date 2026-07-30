#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>
#include "inmem_db.h"

namespace inmembase {

enum class CmdType { SELECT, INSERT, CREATE, DROP, UNKNOWN };

struct SelectCmd {
    std::string table;
    std::vector<std::string> columns; // empty => *
    std::optional<std::string> where_expr; // simple expression string
    size_t limit = 0;
};

struct InsertCmd {
    std::string table;
    std::vector<std::string> values; // raw string values; conversion later
};

struct CreateCmd {
    std::string table;
    std::vector<ColumnDef> columns;
};

struct DropCmd { std::string table; };

struct ParsedCmd {
    CmdType type = CmdType::UNKNOWN;
    std::optional<SelectCmd> select;
    std::optional<InsertCmd> insert;
    std::optional<CreateCmd> create;
    std::optional<DropCmd> drop;
};

class QueryParser {
public:
    // Parse a single semicolon-terminated statement
    static ParsedCmd parse(const std::string& stmt);
private:
    static std::vector<std::string> split_tokens(const std::string& s);
};

} // namespace inmembase
