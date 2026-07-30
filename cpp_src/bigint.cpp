#include <vector>
#include <string>
#include <algorithm>

// Very small non-negative big integer to exercise bignum code paths.
class BigInt {
public:
    BigInt() { digits.push_back(0); }
    BigInt(uint64_t v) { while (v) { digits.push_back((uint32_t)(v%1000000000)); v/=1000000000; } if (digits.empty()) digits.push_back(0); }
    std::string to_string() const {
        std::string out;
        for (size_t i=0;i<digits.size();++i) {
            uint32_t d = digits[i];
            std::string part = std::to_string(d);
            if (i+1 < digits.size()) while (part.size() < 9) part = '0' + part;
            out = part + out;
        }
        return out;
    }
    void add(const BigInt& o) {
        uint64_t carry = 0; size_t n = std::max(digits.size(), o.digits.size());
        digits.resize(n,0);
        for (size_t i=0;i<n;++i) {
            uint64_t a = digits[i]; uint64_t b = (i<o.digits.size()?o.digits[i]:0);
            uint64_t s = a + b + carry; digits[i] = (uint32_t)(s % 1000000000); carry = s/1000000000;
        }
        if (carry) digits.push_back((uint32_t)carry);
    }
private:
    std::vector<uint32_t> digits; // little-endian base 1e9
};
