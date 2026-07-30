#include <chrono>
#include <iostream>
#include <vector>
#include "inmem_db.h"
#include "query_parser.h"
#include "query_planner.h"

using namespace inmembase;

int main_benchmark(int argc, char** argv) {
    Database db; QueryPlanner planner(db);
    planner.execute(QueryParser::parse("CREATE TABLE numbers id INT, val INT"));
    // insert many rows
    const int N = 10000;
    for (int i=0;i<N;++i) {
        std::ostringstream ss; ss << "INSERT INTO numbers VALUES (" << i << ", " << (i*2) << ")";
        planner.execute(QueryParser::parse(ss.str()));
    }
    auto t0 = std::chrono::steady_clock::now();
    std::vector<Row> rows; auto res = planner.execute(QueryParser::parse("SELECT * FROM numbers WHERE id = 1234"), &rows);
    auto t1 = std::chrono::steady_clock::now();
    std::chrono::duration<double> dt = t1 - t0;
    std::cout << "Query returned " << rows.size() << " rows in " << dt.count() << "s\n";
    return 0;
}
