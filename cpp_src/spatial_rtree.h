#ifndef SPATIAL_RTREE_H
#define SPATIAL_RTREE_H

#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace temene {

struct BoundingBox {
  double minX, minY, maxX, maxY;

  bool intersects(const BoundingBox& other) const {
    return !(minX > other.maxX || maxX < other.minX || minY > other.maxY || maxY < other.minY);
  }

  bool contains(const BoundingBox& other) const {
    return (minX <= other.minX && maxX >= other.maxX && minY <= other.minY && maxY >= other.maxY);
  }

  double area() const {
    return (maxX - minX) * (maxY - minY);
  }

  static BoundingBox combine(const BoundingBox& b1, const BoundingBox& b2) {
    return {
      std::min(b1.minX, b2.minX),
      std::min(b1.minY, b2.minY),
      std::max(b1.maxX, b2.maxX),
      std::max(b1.maxY, b2.maxY)
    };
  }
};

template <typename T>
struct RTreeNode {
  bool isLeaf{true};
  BoundingBox bounds;
  std::vector<std::pair<BoundingBox, T>> data;
  std::vector<std::shared_ptr<RTreeNode<T>>> children;
};

template <typename T>
class RTree {
public:
  RTree() {
    root_ = std::make_shared<RTreeNode<T>>();
    root_->bounds = {0, 0, 0, 0};
  }

  void insert(const BoundingBox& box, const T& item) {
    root_->data.push_back({box, item});
    root_->bounds = BoundingBox::combine(root_->bounds, box);
  }

  std::vector<T> query(const BoundingBox& searchBox) const {
    std::vector<T> results;
    for (const auto& [box, item] : root_->data) {
      if (searchBox.intersects(box)) {
        results.push_back(item);
      }
    }
    return results;
  }

private:
  std::shared_ptr<RTreeNode<T>> root_;
};

} // namespace temene

#endif // SPATIAL_RTREE_H
