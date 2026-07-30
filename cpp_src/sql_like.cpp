#include <vector>
#include <string>
#include <map>

// Tiny in-memory SQL-like store: supports SELECT keys WHERE key=val
using Row = std::map<std::string,std::string>;
using Table = std::vector<Row>;

Table select_where(const Table& t, const std::string& key, const std::string& value) {
    Table out;
    for (const auto& r : t) {
        auto it = r.find(key);
        if (it != r.end() && it->second == value) out.push_back(r);
    }
    return out;
}
