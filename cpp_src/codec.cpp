#include "codec.h"
#include <string>
#include <vector>
#include <stdexcept>

static const char* b64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const std::vector<uint8_t>& data) {
    std::string out; out.reserve(((data.size()+2)/3)*4);
    size_t i=0;
    while (i < data.size()) {
        uint32_t a = data[i++];
        uint32_t b = (i < data.size()) ? data[i++] : 0;
        uint32_t c = (i < data.size()) ? data[i++] : 0;
        uint32_t triple = (a << 16) | (b << 8) | c;
        out.push_back(b64_chars[(triple >> 18) & 0x3F]);
        out.push_back(b64_chars[(triple >> 12) & 0x3F]);
        out.push_back((i-2) < data.size() ? b64_chars[(triple >> 6) & 0x3F] : '=');
        out.push_back((i-1) < data.size() ? b64_chars[(triple) & 0x3F] : '=');
    }
    return out;
}

static int b64_val(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

std::vector<uint8_t> base64_decode(const std::string& s) {
    std::vector<uint8_t> out;
    size_t i = 0;
    while (i < s.size()) {
        int vals[4] = {0,0,0,0};
        int filled = 0;
        for (int j=0;j<4 && i < s.size(); ++j) {
            char c = s[i++];
            if (c == '=') { vals[j] = 0; }
            else { int v = b64_val(c); if (v<0) throw std::runtime_error("invalid base64"); vals[j]=v; }
            ++filled;
        }
        uint32_t triple = (vals[0]<<18) | (vals[1]<<12) | (vals[2]<<6) | vals[3];
        out.push_back((triple >> 16) & 0xFF);
        if (filled >= 3) out.push_back((triple >> 8) & 0xFF);
        if (filled >= 4) out.push_back(triple & 0xFF);
    }
    return out;
}

std::string to_hex(const std::vector<uint8_t>& data) {
    std::string out; out.reserve(data.size()*2);
    const char* hex = "0123456789abcdef";
    for (uint8_t b : data) { out.push_back(hex[(b>>4)&0xF]); out.push_back(hex[b&0xF]); }
    return out;
}
