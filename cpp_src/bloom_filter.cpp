#include <vector>
#include <functional>
#include <string>

// Simple Bloom filter using double hashing.
class BloomFilter {
public:
    BloomFilter(size_t bits) : bits_(bits), data_((bits+7)/8, 0) {}
    void add(const std::string& s) {
        auto h1 = std::hash<std::string>{}(s);
        auto h2 = std::hash<size_t>{}(h1);
        set_bit((h1 ^ (h2<<1)) % bits_);
        set_bit((h2 ^ (h1<<1)) % bits_);
    }
    bool might_contain(const std::string& s) const {
        auto h1 = std::hash<std::string>{}(s);
        auto h2 = std::hash<size_t>{}(h1);
        return get_bit((h1 ^ (h2<<1)) % bits_) && get_bit((h2 ^ (h1<<1)) % bits_);
    }
private:
    size_t bits_;
    std::vector<uint8_t> data_;
    void set_bit(size_t i) { data_[i/8] |= (1u << (i%8)); }
    bool get_bit(size_t i) const { return (data_[i/8] >> (i%8)) & 1; }
};
