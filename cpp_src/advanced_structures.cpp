#include "advanced_structures.h"

namespace temene {

// SkipList implementation
SkipList::SkipList(int maxLevel, float p)
  : maxLevel_(maxLevel), p_(p) {
  header_ = new Node(-1, -1, maxLevel_);
}

SkipList::~SkipList() {
  Node* curr = header_;
  while (curr) {
    Node* next = curr->forward[0];
    delete curr;
    curr = next;
  }
}

int SkipList::randomLevel() {
  int lvl = 0;
  static std::mt19937 rng(1337);
  static std::uniform_real_distribution<float> dist(0.0, 1.0);
  while (dist(rng) < p_ && lvl < maxLevel_) {
    lvl++;
  }
  return lvl;
}

void SkipList::insert(int key, int value) {
  std::vector<Node*> update(maxLevel_ + 1, nullptr);
  Node* curr = header_;

  for (int i = level_; i >= 0; i--) {
    while (curr->forward[i] && curr->forward[i]->key < key) {
      curr = curr->forward[i];
    }
    update[i] = curr;
  }

  curr = curr->forward[0];

  if (curr && curr->key == key) {
    curr->value = value;
    return;
  }

  int rlevel = randomLevel();
  if (rlevel > level_) {
    for (int i = level_ + 1; i <= rlevel; i++) {
      update[i] = header_;
    }
    level_ = rlevel;
  }

  Node* nNode = new Node(key, value, rlevel);
  for (int i = 0; i <= rlevel; i++) {
    nNode->forward[i] = update[i]->forward[i];
    update[i]->forward[i] = nNode;
  }
}

bool SkipList::search(int key, int& outValue) const {
  Node* curr = header_;
  for (int i = level_; i >= 0; i--) {
    while (curr->forward[i] && curr->forward[i]->key < key) {
      curr = curr->forward[i];
    }
  }
  curr = curr->forward[0];
  if (curr && curr->key == key) {
    outValue = curr->value;
    return true;
  }
  return false;
}

bool SkipList::erase(int key) {
  std::vector<Node*> update(maxLevel_ + 1, nullptr);
  Node* curr = header_;

  for (int i = level_; i >= 0; i--) {
    while (curr->forward[i] && curr->forward[i]->key < key) {
      curr = curr->forward[i];
    }
    update[i] = curr;
  }

  curr = curr->forward[0];
  if (!curr || curr->key != key) return false;

  for (int i = 0; i <= level_; i++) {
    if (update[i]->forward[i] != curr) break;
    update[i]->forward[i] = curr->forward[i];
  }

  delete curr;
  while (level_ > 0 && !header_->forward[level_]) {
    level_--;
  }

  return true;
}

// DisjointSetUnion implementation
DisjointSetUnion::DisjointSetUnion(size_t n)
  : parent_(n), rank_(n, 0), numSets_(n) {
  for (size_t i = 0; i < n; ++i) parent_[i] = static_cast<int>(i);
}

int DisjointSetUnion::find(int i) {
  if (parent_[i] == i) return i;
  return parent_[i] = find(parent_[i]);
}

bool DisjointSetUnion::unite(int i, int j) {
  int rootI = find(i);
  int rootJ = find(j);
  if (rootI == rootJ) return false;

  if (rank_[rootI] < rank_[rootJ]) std::swap(rootI, rootJ);
  parent_[rootJ] = rootI;
  if (rank_[rootI] == rank_[rootJ]) rank_[rootI]++;
  numSets_--;
  return true;
}

bool DisjointSetUnion::isSameSet(int i, int j) {
  return find(i) == find(j);
}

// FenwickTree implementation
FenwickTree::FenwickTree(size_t n) : n_(n), tree_(n + 1, 0) {}

void FenwickTree::add(size_t idx, int64_t val) {
  for (size_t i = idx + 1; i <= n_; i += i & -i) {
    tree_[i] += val;
  }
}

int64_t FenwickTree::query(size_t idx) const {
  int64_t sum = 0;
  for (size_t i = idx + 1; i > 0; i -= i & -i) {
    sum += tree_[i];
  }
  return sum;
}

int64_t FenwickTree::rangeQuery(size_t l, size_t r) const {
  if (l == 0) return query(r);
  return query(r) - query(l - 1);
}

// SegmentTree implementation
SegmentTree::SegmentTree(const std::vector<int64_t>& data)
  : n_(data.size()), tree_(4 * data.size(), 0), lazy_(4 * data.size(), 0) {
  if (!data.empty()) {
    build(data, 1, 0, n_ - 1);
  }
}

void SegmentTree::build(const std::vector<int64_t>& data, size_t node, size_t start, size_t end) {
  if (start == end) {
    tree_[node] = data[start];
    return;
  }
  size_t mid = (start + end) / 2;
  build(data, 2 * node, start, mid);
  build(data, 2 * node + 1, mid + 1, end);
  tree_[node] = tree_[2 * node] + tree_[2 * node + 1];
}

void SegmentTree::push(size_t node, size_t start, size_t end) {
  if (lazy_[node] != 0) {
    tree_[node] += lazy_[node] * (end - start + 1);
    if (start != end) {
      lazy_[2 * node] += lazy_[node];
      lazy_[2 * node + 1] += lazy_[node];
    }
    lazy_[node] = 0;
  }
}

void SegmentTree::updateRange(size_t node, size_t start, size_t end, size_t l, size_t r, int64_t val) {
  push(node, start, end);
  if (start > end || start > r || end < l) return;
  if (start >= l && end <= r) {
    lazy_[node] += val;
    push(node, start, end);
    return;
  }
  size_t mid = (start + end) / 2;
  updateRange(2 * node, start, mid, l, r, val);
  updateRange(2 * node + 1, mid + 1, end, l, r, val);
  tree_[node] = tree_[2 * node] + tree_[2 * node + 1];
}

void SegmentTree::updateRange(size_t l, size_t r, int64_t val) {
  updateRange(1, 0, n_ - 1, l, r, val);
}

int64_t SegmentTree::queryRange(size_t node, size_t start, size_t end, size_t l, size_t r) {
  if (start > end || start > r || end < l) return 0;
  push(node, start, end);
  if (start >= l && end <= r) return tree_[node];
  size_t mid = (start + end) / 2;
  return queryRange(2 * node, start, mid, l, r) + queryRange(2 * node + 1, mid + 1, end, l, r);
}

int64_t SegmentTree::queryRange(size_t l, size_t r) {
  return queryRange(1, 0, n_ - 1, l, r);
}

} // namespace temene
