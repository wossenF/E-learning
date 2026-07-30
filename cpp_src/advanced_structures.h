#ifndef ADVANCED_STRUCTURES_H
#define ADVANCED_STRUCTURES_H

#include <vector>
#include <random>
#include <memory>
#include <iostream>
#include <algorithm>

namespace temene {

class SkipList {
public:
  explicit SkipList(int maxLevel = 16, float p = 0.5);
  ~SkipList();

  void insert(int key, int value);
  bool search(int key, int& outValue) const;
  bool erase(int key);

private:
  struct Node {
    int key;
    int value;
    std::vector<Node*> forward;
    Node(int k, int v, int level) : key(k), value(v), forward(level + 1, nullptr) {}
  };

  int maxLevel_;
  float p_;
  int level_{0};
  Node* header_;

  int randomLevel();
};

class DisjointSetUnion {
public:
  explicit DisjointSetUnion(size_t n);

  int find(int i);
  bool unite(int i, int j);
  bool isSameSet(int i, int j);

  size_t countSets() const { return numSets_; }

private:
  std::vector<int> parent_;
  std::vector<int> rank_;
  size_t numSets_;
};

class FenwickTree {
public:
  explicit FenwickTree(size_t n);

  void add(size_t idx, int64_t val);
  int64_t query(size_t idx) const;
  int64_t rangeQuery(size_t l, size_t r) const;

private:
  size_t n_;
  std::vector<int64_t> tree_;
};

class SegmentTree {
public:
  explicit SegmentTree(const std::vector<int64_t>& data);

  void updateRange(size_t l, size_t r, int64_t val);
  int64_t queryRange(size_t l, size_t r);

private:
  size_t n_;
  std::vector<int64_t> tree_;
  std::vector<int64_t> lazy_;

  void build(const std::vector<int64_t>& data, size_t node, size_t start, size_t end);
  void push(size_t node, size_t start, size_t end);
  void updateRange(size_t node, size_t start, size_t end, size_t l, size_t r, int64_t val);
  int64_t queryRange(size_t node, size_t start, size_t end, size_t l, size_t r);
};

} // namespace temene

#endif // ADVANCED_STRUCTURES_H
