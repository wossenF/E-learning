#include "http_server_stub.cpp"
#include "inmem_db.h"
#include <string>
#include <iostream>

// This file wires the http_server_stub to a minimal REST-like handler
// exposing the in-memory DB for GET/POST on /api/<table>

namespace inmembase {

void register_rest_handlers(Router& router, Database& db) {
    router.add_route("/api/", [&db](const HttpRequest& req){
        HttpResponse r;
        // parse path
        std::string path = req.path; if (path.size()>5) path = path.substr(5); // strip /api/
        if (req.method == "GET") {
            auto tab = db.get_table(path);
            if (!tab) { r.status=404; r.body="not found"; r.headers["content-length"] = "9"; return r; }
            auto rows = tab->select_all(); std::ostringstream ss;
            ss << "[";
            bool first=true;
            for (const auto& row: rows) {
                if (!first) ss << ","; first=false; ss << "{";
                for (size_t i=0;i<row.values.size();++i) {
                    if (i) ss << ","; ss << "\"c"<<i<<"\":";
                    if (std::holds_alternative<std::string>(row.values[i])) ss << '"' << std::get<std::string>(row.values[i]) << '"';
                    else if (std::holds_alternative<long long>(row.values[i])) ss << std::get<long long>(row.values[i]);
                    else if (std::holds_alternative<double>(row.values[i])) ss << std::get<double>(row.values[i]);
                    else if (std::holds_alternative<bool>(row.values[i])) ss << (std::get<bool>(row.values[i])?"true":"false");
                    else ss << "null";
                }
                ss << "}";
            }
            ss << "]"; r.status=200; r.body = ss.str(); r.headers["content-length"] = std::to_string(r.body.size()); r.headers["content-type"] = "application/json"; return r;
        }
        r.status=405; r.body="method not allowed"; r.headers["content-length"] = std::to_string(r.body.size()); return r;
    });
}

} // namespace inmembase
