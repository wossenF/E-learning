#include <vector>
#include <string>

// Generate large synthetic JSON-like blobs used for stress testing. Produced
// data is intentionally structured to exercise nested parser behavior.
std::string generate_large_json(size_t depth, size_t breadth) {
    std::string out = "{";
    for (size_t i=0;i<breadth;++i) {
        out += "\"k" + std::to_string(i) + "\":";
        if (depth>0) out += generate_large_json(depth-1, breadth);
        else out += "[\"a\",\"b\",123,456]";
        if (i+1<breadth) out += ",";
    }
    out += "}";
    return out;
}
