#include "query_planner.h"
#include "query_parser.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>

namespace inmembase {

// Very small query optimizer: rewrites simple queries for projection pushdown
// and simple constant folding. This is intentionally minimal but provides
// a realistic code path for analysis and fuzzing.

class QueryOptimizer {
public:
    static ParsedCmd optimize(const ParsedCmd& cmd) {
        ParsedCmd out = cmd;
        if (cmd.type == CmdType::SELECT && cmd.select) {
            // projection pushdown: if columns empty treat as all
            // constant folding: if where expression is constant false, rewrite to limit 0
            if (cmd.select->where_expr) {
                std::string w = *cmd.select->where_expr;
                std::string lw=w; std::transform(lw.begin(), lw.end(), lw.begin(), ::tolower);
                if (lw.find("false")!=std::string::npos) {
                    out.select->limit = 0; // indicate empty
                }
            }
        }
        return out;
    }
};

} // namespace inmembase
