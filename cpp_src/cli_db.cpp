#include <iostream>
#include <string>
#include <vector>
#include "inmem_db.h"
#include "query_parser.h"
#include "query_planner.h"

using namespace inmembase;

static void print_row(const Row& r) {
    bool first=true; std::cout << "| ";
    for (const auto& v : r.values) {
        if (!first) std::cout << " | "; first=false;
        if (std::holds_alternative<std::nullptr_t>(v)) std::cout << "NULL";
        else if (std::holds_alternative<bool>(v)) std::cout << (std::get<bool>(v) ? "true" : "false");
        else if (std::holds_alternative<long long>(v)) std::cout << std::get<long long>(v);
        else if (std::holds_alternative<double>(v)) std::cout << std::get<double>(v);
        else if (std::holds_alternative<std::string>(v)) std::cout << std::get<std::string>(v);
    }
    std::cout << " |\n";
}

int main_cli_db(int argc, char** argv) {
    Database db; QueryPlanner planner(db);
    std::cout << "SimpleInMemDB v0.1 - interactive shell\n";
    std::string line;
    while (true) {
        std::cout << "> "; if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;
        if (line == "quit" || line == "exit") break;
        auto cmd = QueryParser::parse(line);
        std::vector<Row> rows;
        auto res = planner.execute(cmd, &rows);
        if (!res.first) std::cout << "ERR: " << res.second << "\n";
        else {
            std::cout << "OK: " << res.second << "\n";
            for (const auto& r : rows) print_row(r);
        }
    }
    return 0;
}
