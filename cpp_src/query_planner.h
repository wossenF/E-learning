#pragma once
#include "query_parser.h"
#include "inmem_db.h"
#include <string>
#include <vector>

namespace inmembase {

class QueryPlanner {
public:
    QueryPlanner(Database& db): db_(db) {}
    // Execute a parsed command and return rows (for select) or status string
    std::pair<bool,std::string> execute(const ParsedCmd& cmd, std::vector<Row>* outRows=nullptr);
private:
    Database& db_;
};

} // namespace inmembase
