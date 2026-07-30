#include <string>
#include <vector>

// Utilities to convert between UTF-16 (basic) and UTF-8; small and partial.
std::string utf16_to_utf8(const std::u16string& s) {
    std::string out;
    for (char16_t c : s) {
        if (c < 0x80) out.push_back((char)c);
        else if (c < 0x800) { out.push_back((char)(0xC0 | (c>>6))); out.push_back((char)(0x80 | (c & 0x3F))); }
        else { out.push_back((char)(0xE0 | (c>>12))); out.push_back((char)(0x80 | ((c>>6)&0x3F))); out.push_back((char)(0x80 | (c & 0x3F))); }
    }
    return out;
}
