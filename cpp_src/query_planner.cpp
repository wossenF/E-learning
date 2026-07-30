#include "query_planner.h"
#include <sstream>
#include <algorithm>

namespace inmembase {

static Value parse_value_for_type(const std::string& token, const ColumnDef::Type t) {
    try {
        if (t == ColumnDef::INT) return Value(std::stoll(token));
        if (t == ColumnDef::DOUBLE) return Value(std::stod(token));
        if (t == ColumnDef::BOOL) { std::string lt=token; std::transform(lt.begin(), lt.end(), lt.begin(), ::tolower); return Value(lt=="true"||lt=="1"); }
    } catch(...) {}
    // fallback: string (strip quotes)
    if (!token.empty() && token.front()=='\'' && token.back()=='\'') return Value(token.substr(1, token.size()-2));
    return Value(token);
}

std::pair<bool,std::string> QueryPlanner::execute(const ParsedCmd& cmd, std::vector<Row>* outRows) {
    if (cmd.type == CmdType::UNKNOWN) return {false, "unknown command"};
    if (cmd.type == CmdType::CREATE && cmd.create) {
        bool ok = db_.create_table(cmd.create->table, cmd.create->columns);
        return {ok, ok?"table created":"table exists"};
    }
    if (cmd.type == CmdType::DROP && cmd.drop) {
        bool ok = db_.drop_table(cmd.drop->table);
        return {ok, ok?"dropped":"not found"};
    }
    if (cmd.type == CmdType::INSERT && cmd.insert) {
        auto tblOpt = db_.get_table(cmd.insert->table);
        if (!tblOpt) return {false, "table not found"};
        Table tbl = *tblOpt; // copy; for simplicity
        if (cmd.insert->values.size() != tbl.columns().size()) return {false, "column count mismatch"};
        Row r; r.values.reserve(tbl.columns().size());
        for (size_t i=0;i<tbl.columns().size();++i) {
            const auto& col = tbl.columns()[i]; r.values.push_back(parse_value_for_type(cmd.insert->values[i], col.type));
        }
        bool ok = db_.insert_into(cmd.insert->table, r);
        return {ok, ok?"inserted":"insert failed"};
    }
    if (cmd.type == CmdType::SELECT && cmd.select) {
        auto tblOpt = db_.get_table(cmd.select->table);
        if (!tblOpt) return {false, "table not found"};
        Table tbl = *tblOpt; // copy
        auto all = tbl.select_all();
        // naive where: supports simple equality 'col = value' on single column
        if (cmd.select->where_expr) {
            std::string we = *cmd.select->where_expr;
            // parse: col = value
            size_t eq = we.find('=');
            if (eq!=std::string::npos) {
                std::string left = we.substr(0, eq); std::string right = we.substr(eq+1);
                auto trim = [](std::string s){ size_t b=0,e=s.size(); while (b<e && isspace((unsigned char)s[b])) ++b; while (e>b && isspace((unsigned char)s[e-1])) --e; return s.substr(b,e-b); };
                left = trim(left); right = trim(right);
                // find column index
                int idx=-1; for (size_t i=0;i<tbl.columns().size();++i) if (tbl.columns()[i].name == left) { idx=(int)i; break; }
                if (idx>=0) {
                    std::vector<Row> filtered; for (const auto& r: all) {
                        if (idx < (int)r.values.size()) {
                            std::string sval;
                            if (std::holds_alternative<std::string>(r.values[idx])) sval = std::get<std::string>(r.values[idx]);
                            else if (std::holds_alternative<long long>(r.values[idx])) sval = std::to_string(std::get<long long>(r.values[idx]));
                            else if (std::holds_alternative<double>(r.values[idx])) sval = std::to_string(std::get<double>(r.values[idx]));
                            else if (std::holds_alternative<bool>(r.values[idx])) sval = std::get<bool>(r.values[idx])?"true":"false";
                            if (sval == right || ('\''+sval+'\'')==right) filtered.push_back(r);
                        }
                    }
                    all.swap(filtered);
                }
            }
        }
        if (outRows) *outRows = all;
        return {true, std::to_string(all.size())+" rows"};
    }
    return {false, "unsupported"};
}

} // namespace inmembase
