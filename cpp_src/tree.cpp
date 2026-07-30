#include <vector>
#include <memory>

// Simple tree node with arbitrary children to exercise recursion.
struct TreeNode {
    int value;
    std::vector<std::unique_ptr<TreeNode>> children;
    TreeNode(int v=0): value(v) {}
    void add_child(std::unique_ptr<TreeNode> c) { children.push_back(std::move(c)); }
};

int sum_tree(const TreeNode* n) {
    if (!n) return 0; int s = n->value; for (const auto& c : n->children) s += sum_tree(c.get()); return s;
}
