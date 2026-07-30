#ifndef BPLUS_TREE_H
#define BPLUS_TREE_H

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <iostream>

namespace temene {

const int BPLUS_TREE_DEGREE = 4; // Max keys per node = 2*M - 1

template <typename K, typename V>
struct BPlusTreeNode {
  bool isLeaf{true};
  std::vector<K> keys;
  std::vector<V> values; // Only used in leaf nodes
  std::vector<std::shared_ptr<BPlusTreeNode<K, V>>> children; // Used in internal nodes
  std::shared_ptr<BPlusTreeNode<K, V>> next{nullptr}; // Leaf node chaining pointer
};

template <typename K, typename V>
class BPlusTree {
public:
  BPlusTree() {
    root_ = std::make_shared<BPlusTreeNode<K, V>>();
  }

  bool find(const K& key, V& outValue) const {
    auto leaf = findLeaf(root_, key);
    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    if (it != leaf->keys.end() && *it == key) {
      size_t idx = std::distance(leaf->keys.begin(), it);
      outValue = leaf->values[idx];
      return true;
    }
    return false;
  }

  void insert(const K& key, const V& value) {
    auto leaf = findLeaf(root_, key);
    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    size_t idx = std::distance(leaf->keys.begin(), it);

    if (it != leaf->keys.end() && *it == key) {
      leaf->values[idx] = value;
      return;
    }

    leaf->keys.insert(it, key);
    leaf->values.insert(leaf->values.begin() + idx, value);

    if (leaf->keys.size() >= 2 * BPLUS_TREE_DEGREE) {
      splitLeaf(leaf);
    }
  }

  std::vector<V> rangeSearch(const K& minKey, const K& maxKey) const {
    std::vector<V> result;
    auto leaf = findLeaf(root_, minKey);

    while (leaf) {
      for (size_t i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] >= minKey && leaf->keys[i] <= maxKey) {
          result.push_back(leaf->values[i]);
        }
        if (leaf->keys[i] > maxKey) return result;
      }
      leaf = leaf->next;
    }
    return result;
  }

private:
  std::shared_ptr<BPlusTreeNode<K, V>> root_;

  std::shared_ptr<BPlusTreeNode<K, V>> findLeaf(std::shared_ptr<BPlusTreeNode<K, V>> node, const K& key) const {
    if (node->isLeaf) return node;
    size_t idx = 0;
    while (idx < node->keys.size() && key >= node->keys[idx]) {
      idx++;
    }
    return findLeaf(node->children[idx], key);
  }

  void splitLeaf(std::shared_ptr<BPlusTreeNode<K, V>> leaf) {
    auto newLeaf = std::make_shared<BPlusTreeNode<K, V>>();
    size_t mid = leaf->keys.size() / 2;

    newLeaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
    newLeaf->values.assign(leaf->values.begin() + mid, leaf->values.end());

    leaf->keys.erase(leaf->keys.begin() + mid, leaf->keys.end());
    leaf->values.erase(leaf->values.begin() + mid, leaf->values.end());

    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    if (leaf == root_) {
      auto newRoot = std::make_shared<BPlusTreeNode<K, V>>();
      newRoot->isLeaf = false;
      newRoot->keys.push_back(newLeaf->keys[0]);
      newRoot->children.push_back(leaf);
      newRoot->children.push_back(newLeaf);
      root_ = newRoot;
    }
  }
};

void runBPlusTreeBenchmark();

} // namespace temene

#endif // BPLUS_TREE_H
