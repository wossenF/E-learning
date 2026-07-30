#include <map>
#include <string>
#include <vector>

// Simple inverted index mapping terms to document ids.
class InvertedIndex {
public:
    void add(int docid, const std::vector<std::string>& terms) {
        for (const auto& t : terms) index_[t].push_back(docid);
    }
    std::vector<int> lookup(const std::string& term) const {
        auto it = index_.find(term);
        if (it==index_.end()) return {};
        return it->second;
    }
private:
    std::map<std::string, std::vector<int>> index_;
};
