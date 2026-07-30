#include <string>
#include <map>
#include <algorithm>

// Minimal HTTP header parser to exercise string parsing and map handling.

std::map<std::string,std::string> parse_http_headers(const std::string& raw) {
    std::map<std::string,std::string> out;
    size_t pos = 0;
    while (pos < raw.size()) {
        size_t eol = raw.find('\n', pos);
        if (eol == std::string::npos) eol = raw.size();
        std::string line = raw.substr(pos, eol - pos);
        // trim
        while (!line.empty() && (line.back()=='\r' || line.back()=='\n')) line.pop_back();
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon+1);
            // trim spaces
            while (!key.empty() && std::isspace((unsigned char)key.front())) key.erase(key.begin());
            while (!key.empty() && std::isspace((unsigned char)key.back())) key.pop_back();
            while (!val.empty() && std::isspace((unsigned char)val.front())) val.erase(val.begin());
            while (!val.empty() && std::isspace((unsigned char)val.back())) val.pop_back();
            // lowercase the header name for normalization
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return std::tolower(c); });
            out[key] = val;
        }
        pos = eol + 1;
    }
    return out;
}
