#include <vector>
#include <cstdint>

// Compression stub: simple RLE (run-length encoding) and decode to exercise
// byte-level manipulations.
std::vector<uint8_t> rle_compress(const std::vector<uint8_t>& in) {
    std::vector<uint8_t> out;
    if (in.empty()) return out;
    uint8_t last = in[0]; uint8_t cnt = 1;
    for (size_t i=1;i<in.size();++i) {
        if (in[i] == last && cnt < 255) ++cnt;
        else { out.push_back(last); out.push_back(cnt); last = in[i]; cnt = 1; }
    }
    out.push_back(last); out.push_back(cnt);
    return out;
}

std::vector<uint8_t> rle_decompress(const std::vector<uint8_t>& in) {
    std::vector<uint8_t> out;
    for (size_t i=0;i+1<in.size(); i+=2) for (int j=0;j<in[i+1];++j) out.push_back(in[i]);
    return out;
}
