#include "utils.h"
#include <sstream>

std::string hexdump(const uint8_t* data, size_t size, size_t max_bytes) {
    std::ostringstream ss;
    size_t n = (size < max_bytes) ? size : max_bytes;
    for (size_t i=0;i<n;++i) {
        unsigned int b = data[i];
        ss << "0123456789abcdef"[(b>>4)&0xF] << "0123456789abcdef"[b&0xF];
        if (i+1 < n) ss << ' ';
    }
    if (size > n) ss << " ...";
    return ss.str();
}

bool is_valid_utf8(const uint8_t* data, size_t size) {
    size_t i = 0;
    while (i < size) {
        uint8_t c = data[i];
        if (c < 0x80) { ++i; continue; }
        else if ((c >> 5) == 0x6) { if (i+1>=size) return false; if ((data[i+1]>>6)!=0x2) return false; i+=2; }
        else if ((c >> 4) == 0xE) { if (i+2>=size) return false; if ((data[i+1]>>6)!=0x2) return false; if ((data[i+2]>>6)!=0x2) return false; i+=3; }
        else if ((c >> 3) == 0x1E) { if (i+3>=size) return false; if ((data[i+1]>>6)!=0x2) return false; if ((data[i+2]>>6)!=0x2) return false; if ((data[i+3]>>6)!=0x2) return false; i+=4; }
        else return false;
    }
    return true;
}
