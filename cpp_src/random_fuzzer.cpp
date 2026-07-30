#include <random>
#include <vector>
#include <string>

// Small randomness-based mutation helpers used by fuzz harnesses; keeps state
// local to avoid global side effects.
std::vector<uint8_t> random_mutate(const uint8_t* in, size_t len, size_t mutations) {
    std::vector<uint8_t> out(in, in+len);
    std::mt19937 rng((unsigned)std::random_device{}());
    std::uniform_int_distribution<size_t> idx(0, len?len-1:0);
    std::uniform_int_distribution<int> op(0,2);
    for (size_t i=0;i<mutations && len>0;++i) {
        size_t p = idx(rng);
        int o = op(rng);
        if (o==0) out[p] = (uint8_t)rng();
        else if (o==1) out[p] ^= (uint8_t)(rng());
        else if (o==2) out.insert(out.begin()+p, (uint8_t)(rng()));
    }
    return out;
}
