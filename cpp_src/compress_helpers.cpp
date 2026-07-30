#include <vector>
#include <cstdint>

// Small compression stub: a trivial run-length encoder/decoder for demo.
std::vector<uint8_t> rle_encode(const std::vector<uint8_t>& in) {
    std::vector<uint8_t> out;
    size_t i=0; while (i<in.size()) {
        uint8_t v = in[i++]; size_t cnt = 1; while (i<in.size() && in[i]==v && cnt<255) { ++cnt; ++i; }
        out.push_back((uint8_t)cnt); out.push_back(v);
    }
    return out;
}

std::vector<uint8_t> rle_decode(const std::vector<uint8_t>& in) {
    std::vector<uint8_t> out; size_t i=0; while (i+1<in.size()) { int cnt = in[i++]; uint8_t v = in[i++]; for (int j=0;j<cnt;++j) out.push_back(v);} return out;
}
