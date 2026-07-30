#ifndef BVH_TREE_H
#define BVH_TREE_H

#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>

namespace temene {

struct Vector3f {
  float x{0.0f}, y{0.0f}, z{0.0f};

  Vector3f operator+(const Vector3f& o) const { return {x + o.x, y + o.y, z + o.z}; }
  Vector3f operator-(const Vector3f& o) const { return {x - o.x, y - o.y, z - o.z}; }
  Vector3f operator*(float s) const { return {x * s, y * s, z * s}; }

  float dot(const Vector3f& o) const { return x * o.x + y * o.y + z * o.z; }
  Vector3f cross(const Vector3f& o) const {
    return {
      y * o.z - z * o.y,
      z * o.x - x * o.z,
      x * o.y - y * o.x
    };
  }
};

struct AABB {
  Vector3f min{1e30f, 1e30f, 1e30f};
  Vector3f max{-1e30f, -1e30f, -1e30f};

  void extend(const Vector3f& p) {
    min.x = std::min(min.x, p.x);
    min.y = std::min(min.y, p.y);
    min.z = std::min(min.z, p.z);
    max.x = std::max(max.x, p.x);
    max.y = std::max(max.y, p.y);
    max.z = std::max(max.z, p.z);
  }

  void extend(const AABB& b) {
    extend(b.min);
    extend(b.max);
  }

  Vector3f center() const { return (min + max) * 0.5f; }
  Vector3f extent() const { return max - min; }
  float area() const {
    Vector3f e = extent();
    return 2.0f * (e.x * e.y + e.x * e.z + e.y * e.z);
  }
};

struct Ray3f {
  Vector3f origin;
  Vector3f dir;
  Vector3f invDir;
  int sign[3];

  Ray3f(const Vector3f& o, const Vector3f& d) : origin(o), dir(d) {
    invDir = {1.0f / d.x, 1.0f / d.y, 1.0f / d.z};
    sign[0] = (invDir.x < 0);
    sign[1] = (invDir.y < 0);
    sign[2] = (invDir.z < 0);
  }
};

struct BoundingVolumeHierarchyNode {
  AABB bounds;
  std::shared_ptr<BoundingVolumeHierarchyNode> left{nullptr};
  std::shared_ptr<BoundingVolumeHierarchyNode> right{nullptr};
  int firstPrimitiveOffset{0};
  int numPrimitives{0};
};

class BoundingVolumeHierarchy {
public:
  void build(const std::vector<AABB>& primitives);
  bool intersect(const Ray3f& ray) const;

private:
  std::shared_ptr<BoundingVolumeHierarchyNode> root_;
  std::vector<AABB> primitives_;
  std::vector<int> primitiveIndices_;

  std::shared_ptr<BoundingVolumeHierarchyNode> buildRecursive(int startIndex, int numPrims);
  bool intersectNode(const std::shared_ptr<BoundingVolumeHierarchyNode>& node, const Ray3f& ray) const;
  bool intersectAABB(const AABB& bounds, const Ray3f& ray) const;
};

} // namespace temene

#endif // BVH_TREE_H
