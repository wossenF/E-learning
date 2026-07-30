#include "unicode_utils.h"
#include <string>

size_t utf8_char_length(uint8_t lead) {
    if (lead < 0x80) return 1;
    if ((lead >> 5) == 0x6) return 2;
    if ((lead >> 4) == 0xE) return 3;
    if ((lead >> 3) == 0x1E) return 4;
    return 0;
}

bool is_valid_utf8_sequence(const uint8_t* data, size_t size) {
    size_t i = 0;
    while (i < size) {
        uint8_t c = data[i];
        size_t len = utf8_char_length(c);
        if (len == 0 || i + len > size) return false;
        for (size_t j = 1; j < len; ++j) if ((data[i+j] >> 6) != 0x2) return false;
        i += len;
    }
    return true;
}

std::string utf8_sanitize(const uint8_t* data, size_t size) {
    std::string out;
    size_t i=0;
    while (i < size) {
        uint8_t c = data[i];
        size_t len = utf8_char_length(c);
        if (len == 0 || i + len > size) { out.push_back('?'); ++i; continue; }
        bool ok = true;
        for (size_t j=1;j<len;++j) if ((data[i+j]>>6) != 0x2) { ok = false; break; }
        if (!ok) { out.push_back('?'); ++i; continue; }
        for (size_t j=0;j<len;++j) out.push_back((char)data[i+j]);
        i += len;
    }
    return out;
}
