#include <vector>
#include <algorithm>
#include <utility>

// Very small B-tree-like ordered container (not a full B-tree) for demo.
class BTreeStub {
public:
    void insert(int k, int v) { data_.push_back({k,v}); std::sort(data_.begin(), data_.end()); }
    bool find(int k, int& v) const { for (auto &p : data_) if (p.first==k) { v=p.second; return true; } return false; }
    void erase(int k) { data_.erase(std::remove_if(data_.begin(), data_.end(), [&](auto &p){return p.first==k;}), data_.end()); }
private:
    std::vector<std::pair<int,int>> data_;
};
