#include <string>
#include <vector>
#include <sstream>

// Simple CSV parser that supports quoted fields and escapes.
std::vector<std::vector<std::string>> parse_csv(const std::string& input, char sep=',') {
    std::vector<std::vector<std::string>> rows;
    std::istringstream ss(input);
    std::string line;
    while (std::getline(ss, line)) {
        std::vector<std::string> cols;
        std::string cur;
        bool in_quotes = false;
        for (size_t i=0;i<line.size();++i) {
            char c = line[i];
            if (in_quotes) {
                if (c == '"') {
                    if (i+1 < line.size() && line[i+1]=='"') { cur.push_back('"'); ++i; }
                    else in_quotes = false;
                } else cur.push_back(c);
            } else {
                if (c == '"') in_quotes = true;
                else if (c == sep) { cols.push_back(cur); cur.clear(); }
                else cur.push_back(c);
            }
        }
        cols.push_back(cur);
        rows.push_back(std::move(cols));
    }
    return rows;
}
