#include "inmem_db.h"
#include "query_parser.h"
#include "query_planner.h"
#include <iostream>
#include <cassert>

using namespace inmembase;

static void test_create_insert_select() {
    Database db; QueryPlanner planner(db);
    ParsedCmd c1 = QueryParser::parse("CREATE TABLE users id INT, name TEXT");
    auto r1 = planner.execute(c1);
    assert(r1.first);

    ParsedCmd ins = QueryParser::parse("INSERT INTO users VALUES (1, 'alice')");
    auto r2 = planner.execute(ins);
    assert(r2.first);

    ParsedCmd sel = QueryParser::parse("SELECT * FROM users");
    std::vector<Row> rows; auto r3 = planner.execute(sel, &rows);
    assert(r3.first && rows.size()==1);
    std::cout << "test_create_insert_select passed\n";
}

static void test_where() {
    Database db; QueryPlanner planner(db);
    auto c = QueryParser::parse("CREATE TABLE t id INT, name TEXT"); planner.execute(c);
    planner.execute(QueryParser::parse("INSERT INTO t VALUES (10, 'bob')"));
    planner.execute(QueryParser::parse("INSERT INTO t VALUES (11, 'carol')"));
    std::vector<Row> rows; auto r = planner.execute(QueryParser::parse("SELECT * FROM t WHERE id = 10"), &rows);
    assert(r.first && rows.size()==1);
    std::cout << "test_where passed\n";
}

int main_db_tests(int argc, char** argv) {
    test_create_insert_select();
    test_where();
    std::cout << "All DB tests passed\n";
    return 0;
}
