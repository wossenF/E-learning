#include "query_parser.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace inmembase {

static std::string lower_trim(const std::string& s) {
    size_t b=0,e=s.size(); while (b<e && isspace((unsigned char)s[b])) ++b; while (e>b && isspace((unsigned char)s[e-1])) --e; std::string t = s.substr(b,e-b); std::transform(t.begin(), t.end(), t.begin(), ::tolower); return t;
}

std::vector<std::string> QueryParser::split_tokens(const std::string& s) {
    std::vector<std::string> out; std::istringstream ss(s); std::string tok;
    while (ss >> tok) out.push_back(tok);
    return out;
}

ParsedCmd QueryParser::parse(const std::string& stmt) {
    ParsedCmd res;
    auto t = split_tokens(stmt);
    if (t.empty()) return res;
    std::string first = lower_trim(t[0]);
    if (first == "select") {
        res.type = CmdType::SELECT;
        SelectCmd sc;
        // naive: SELECT <cols> FROM <table> [WHERE <expr>] [LIMIT n]
        size_t i=1;
        // collect columns until FROM
        std::string cols;
        while (i < t.size() && lower_trim(t[i]) != "from") { if (!cols.empty()) cols += " "; cols += t[i++]; }
        if (i < t.size() && lower_trim(t[i]) == "from") ++i; else return res;
        if (i < t.size()) sc.table = t[i++];
        // where
        for (; i < t.size(); ++i) {
            std::string lit = lower_trim(t[i]);
            if (lit == "where") {
                // remainder as where expression
                std::string expr;
                for (size_t j=i+1;j<t.size();++j) { if (!expr.empty()) expr += ' '; expr += t[j]; }
                sc.where_expr = expr; break;
            } else if (lit == "limit") {
                if (i+1 < t.size()) sc.limit = std::stoul(t[i+1]); break;
            }
        }
        // parse cols
        if (cols == "*") sc.columns = {};
        else {
            std::vector<std::string> cols_out; std::istringstream cs(cols); std::string c; while (std::getline(cs,c,',')) { size_t b=0,e=c.size(); while (b<e && isspace((unsigned char)c[b])) ++b; while (e>b && isspace((unsigned char)c[e-1])) --e; cols_out.push_back(c.substr(b,e-b)); }
            sc.columns = cols_out;
        }
        res.select = sc;
        return res;
    } else if (first == "insert") {
        res.type = CmdType::INSERT;
        // simple: INSERT INTO table VALUES (v1, v2, ...)
        InsertCmd ic;
        size_t i=1; if (i<t.size() && lower_trim(t[i])=="into") ++i; if (i<t.size()) ic.table = t[i++]; // skip until values
        while (i<t.size() && lower_trim(t[i]) != "values") ++i; if (i<t.size() && lower_trim(t[i])=="values") ++i;
        // join remainder and find parentheses
        std::string rem;
        for (;i<t.size();++i) { if (!rem.empty()) rem += ' '; rem += t[i]; }
        size_t l = rem.find('('), r = rem.rfind(')');
        if (l!=std::string::npos && r!=std::string::npos && r>l) {
            std::string inner = rem.substr(l+1, r-l-1);
            std::vector<std::string> vals; std::istringstream vs(inner); std::string v;
            while (std::getline(vs, v, ',')) { size_t b=0,e=v.size(); while (b<e && isspace((unsigned char)v[b])) ++b; while (e>b && isspace((unsigned char)v[e-1])) --e; vals.push_back(v.substr(b,e-b)); }
            ic.values = vals;
        }
        res.insert = ic; return res;
    } else if (first == "create") {
        res.type = CmdType::CREATE; CreateCmd cc; // very simple
        // CREATE TABLE name (col type, ...)
        size_t i=1; if (i<t.size() && lower_trim(t[i])=="table") ++i; if (i<t.size()) cc.table = t[i++];
        // find parenthesis
        std::string rem;
        for (;i<t.size();++i){ if (!rem.empty()) rem += ' '; rem += t[i]; }
        size_t l = rem.find('('), r = rem.rfind(')');
        if (l!=std::string::npos && r!=std::string::npos && r>l) {
            std::string inner = rem.substr(l+1, r-l-1);
            std::istringstream cs(inner); std::string coldef;
            while (std::getline(cs, coldef, ',')) {
                std::istringstream cd(coldef); std::string cname, ctype; cd >> cname >> ctype; ColumnDef cdv; cdv.name = cname; std::string ct = lower_trim(ctype);
                if (ct.find("int")!=std::string::npos) cdv.type = ColumnDef::INT; else if (ct.find("char")!=std::string::npos || ct.find("text")!=std::string::npos) cdv.type = ColumnDef::STRING; else if (ct.find("double")!=std::string::npos||ct.find("real")!=std::string::npos) cdv.type = ColumnDef::DOUBLE; else cdv.type = ColumnDef::STRING;
                cc.columns.push_back(cdv);
            }
        }
        res.create = cc; return res;
    } else if (first == "drop") {
        res.type = CmdType::DROP; DropCmd dc; size_t i=1; if (i<t.size() && lower_trim(t[i])=="table") ++i; if (i<t.size()) dc.table = t[i++]; res.drop = dc; return res;
    }
    return res;
}

} // namespace inmembase
