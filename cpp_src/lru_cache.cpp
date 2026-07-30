#include <unordered_map>
#include <list>
#include <utility>

// Simple LRU cache implementation (templated in header style but placed
// here as an implementation file to expand substrate size).
template<typename K, typename V>
class LRUCache {
public:
    explicit LRUCache(size_t cap) : cap_(cap) {}
    void put(const K& k, const V& v) {
        auto it = map_.find(k);
        if (it != map_.end()) { items_.erase(it->second.second); map_.erase(it); }
        items_.push_front(k);
        map_[k] = {v, items_.begin()};
        if (map_.size() > cap_) { auto last = items_.back(); items_.pop_back(); map_.erase(last); }
    }
    bool get(const K& k, V& out) {
        auto it = map_.find(k);
        if (it == map_.end()) return false;
        out = it->second.first;
        items_.splice(items_.begin(), items_, it->second.second);
        return true;
    }
private:
    size_t cap_;
    std::list<K> items_;
    std::unordered_map<K, std::pair<V, typename std::list<K>::iterator>> map_;
};

// instantiate for common types to produce object code in the binary
static void _lru_instantiate() {
    LRUCache<int,int> c(16);
    c.put(1,2);
    int v; c.get(1,v);
}
