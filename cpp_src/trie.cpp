#include <string>
#include <array>
#include <memory>
#include <vector>

// Simple Trie (prefix tree) for strings with insert/search operations.
class TrieNode {
public:
    std::array<TrieNode*, 26> child{};
    bool terminal = false;
    TrieNode() { child.fill(nullptr); }
    ~TrieNode() { for (auto c : child) if (c) delete c; }
};

class Trie {
public:
    Trie() : root_(new TrieNode()) {}
    ~Trie() { delete root_; }
    void insert(const std::string& s) {
        TrieNode* p = root_;
        for (unsigned char ch : s) {
            if (ch < 'a' || ch > 'z') continue;
            int idx = ch - 'a';
            if (!p->child[idx]) p->child[idx] = new TrieNode();
            p = p->child[idx];
        }
        p->terminal = true;
    }
    bool search(const std::string& s) const {
        TrieNode* p = root_;
        for (unsigned char ch : s) {
            if (ch < 'a' || ch > 'z') return false;
            int idx = ch - 'a';
            if (!p->child[idx]) return false;
            p = p->child[idx];
        }
        return p->terminal;
    }
private:
    TrieNode* root_;
};
