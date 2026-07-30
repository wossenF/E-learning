#include <string>
#include <map>

// Tiny WebSocket handshake header parser
std::map<std::string,std::string> parse_ws_headers(const std::string& raw) {
    std::map<std::string,std::string> out;
    size_t pos = 0;
    while (pos < raw.size()) {
        size_t eol = raw.find('\n', pos);
        if (eol == std::string::npos) eol = raw.size();
        std::string line = raw.substr(pos, eol - pos);
        if (line.empty()) break;
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon+1);
            out[key] = val;
        }
        pos = eol + 1;
    }
    return out;
}
