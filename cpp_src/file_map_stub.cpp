#include <string>
#include <vector>

// Simple file mapping stub that reads a file into memory (no OS mmap used).
std::vector<uint8_t> map_file_to_memory(const std::string& path) {
    std::vector<uint8_t> out;
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return out;
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f,0,SEEK_SET);
    out.resize(sz); fread(out.data(), 1, sz, f); fclose(f);
    return out;
}
