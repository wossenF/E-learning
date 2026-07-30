#include "inmem_db.h"
#include "query_parser.h"
#include "query_planner.h"
#include "csv_importer.cpp"
#include <iostream>
#include <cassert>

using namespace inmembase;

static void test_csv_import() {
    Database db; QueryPlanner planner(db);
    planner.execute(QueryParser::parse("CREATE TABLE t id INT, name TEXT, val DOUBLE"));
    // create a small csv file
    std::string fname = "test_tmp.csv";
    std::ofstream os(fname);
    os << "1,\"Alice\",3.14\n";
    os << "2,\"Bob\",2.71\n"; os.close();
    bool ok = import_csv_to_table(db, fname, "t"); assert(ok);
    std::vector<Row> rows; planner.execute(QueryParser::parse("SELECT * FROM t"), &rows);
    assert(rows.size()==2);
    std::remove(fname.c_str());
    std::cout << "test_csv_import passed\n";
}

int main_more_tests(int argc, char** argv) {
    test_csv_import();
    std::cout << "more tests passed\n"; return 0;
}
