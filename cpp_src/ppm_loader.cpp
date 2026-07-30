#include <string>
#include <vector>
#include <sstream>
#include <cctype>

// Minimal PPM (P6) loader that reads header and returns raw RGB bytes.
std::vector<uint8_t> load_ppm_p6(const std::string& data) {
    std::istringstream ss(data);
    std::string magic; ss >> magic;
    if (magic != "P6") return {};
    int w, h, maxv; ss >> w >> h >> maxv; ss.get();
    size_t pix = (size_t)w * (size_t)h * 3;
    std::vector<uint8_t> out(pix);
    ss.read(reinterpret_cast<char*>(out.data()), (std::streamsize)pix);
    return out;
}
