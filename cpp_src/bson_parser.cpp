#include <vector>
#include <cstdint>
#include <string>
#include <map>

// Minimal BSON-like stub: interpret a byte vector as a sequence of key-value
// string pairs where each entry is [len:uint16][key][len:uint16][value]. This
// is not BSON spec compliant but provides binary parsing exercises.

std::map<std::string,std::string> parse_bson_stub(const std::vector<uint8_t>& data) {
    std::map<std::string,std::string> out;
    size_t i = 0;
    while (i + 4 <= data.size()) {
        uint16_t klen = (uint16_t)data[i] | ((uint16_t)data[i+1]<<8); i+=2;
        if (i + klen > data.size()) break;
        std::string key((const char*)&data[i], klen); i += klen;
        if (i + 2 > data.size()) break;
        uint16_t vlen = (uint16_t)data[i] | ((uint16_t)data[i+1]<<8); i+=2;
        if (i + vlen > data.size()) break;
        std::string val((const char*)&data[i], vlen); i += vlen;
        out[key] = val;
    }
    return out;
}
