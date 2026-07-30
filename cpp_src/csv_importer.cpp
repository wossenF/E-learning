#include "inmem_db.h"
#include "query_parser.h"
#include "query_planner.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

namespace inmembase {

// CSV importer: reads a CSV file and inserts rows into a table by converting
// text values to column types. Handles simple quoted values and escapes.

static std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> out; std::string cur; bool inq=false;
    for (size_t i=0;i<line.size();++i) {
        char c=line[i]; if (c=='\"') { inq = !inq; continue; }
        if (c==',' && !inq) { out.push_back(cur); cur.clear(); } else cur.push_back(c);
    }
    out.push_back(cur); return out;
}

bool import_csv_to_table(Database& db, const std::string& filename, const std::string& table) {
    std::ifstream is(filename); if (!is.good()) return false;
    auto tblOpt = db.get_table(table); if (!tblOpt) return false; Table tbl = *tblOpt;
    std::string line; size_t line_no=0;
    while (std::getline(is, line)) {
        ++line_no; if (line.empty()) continue;
        auto parts = split_csv_line(line);
        if (parts.size() != tbl.columns().size()) {
            // try to tolerate missing optional trailing columns
            if (parts.size() < tbl.columns().size()) parts.resize(tbl.columns().size());
        }
        Row r; r.values.reserve(tbl.columns().size());
        for (size_t i=0;i<tbl.columns().size();++i) {
            const auto& col = tbl.columns()[i]; std::string token = parts[i]; // trim
            size_t b=0,e=token.size(); while (b<e && isspace((unsigned char)token[b])) ++b; while (e>b && isspace((unsigned char)token[e-1])) --e; token = token.substr(b,e-b);
            if (token.empty()) { r.values.push_back(nullptr); continue; }
            if (col.type == ColumnDef::INT) { try { r.values.push_back(std::stoll(token)); } catch(...) { r.values.push_back(0LL); } }
            else if (col.type == ColumnDef::DOUBLE) { try { r.values.push_back(std::stod(token)); } catch(...) { r.values.push_back(0.0); } }
            else if (col.type == ColumnDef::BOOL) { std::string lt=token; std::transform(lt.begin(), lt.end(), lt.begin(), ::tolower); r.values.push_back(lt=="true"||lt=="1"); }
            else { // string
                if (token.size()>=2 && token.front()=='\"' && token.back()=='\"') token = token.substr(1, token.size()-2);
                r.values.push_back(token);
            }
        }
        db.insert_into(table, r);
    }
    return true;
}

} // namespace inmembase
