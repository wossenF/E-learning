#include "bvh_tree.h"

namespace temene {

void BoundingVolumeHierarchy::build(const std::vector<AABB>& primitives) {
  primitives_ = primitives;
  primitiveIndices_.resize(primitives_.size());
  for (size_t i = 0; i < primitives_.size(); ++i) {
    primitiveIndices_[i] = static_cast<int>(i);
  }

  root_ = buildRecursive(0, static_cast<int>(primitives_.size()));
}

std::shared_ptr<BoundingVolumeHierarchyNode> BoundingVolumeHierarchy::buildRecursive(int startIndex, int numPrims) {
  auto node = std::make_shared<BoundingVolumeHierarchyNode>();

  AABB bounds;
  for (int i = 0; i < numPrims; ++i) {
    bounds.extend(primitives_[primitiveIndices_[startIndex + i]]);
  }
  node->bounds = bounds;

  if (numPrims <= 2) {
    node->firstPrimitiveOffset = startIndex;
    node->numPrimitives = numPrims;
    return node;
  }

  AABB centroidBounds;
  for (int i = 0; i < numPrims; ++i) {
    centroidBounds.extend(primitives_[primitiveIndices_[startIndex + i]].center());
  }

  int dim = 0;
  Vector3f ext = centroidBounds.extent();
  if (ext.y > ext.x && ext.y > ext.z) dim = 1;
  else if (ext.z > ext.x && ext.z > ext.y) dim = 2;

  float splitPos = centroidBounds.center().x;
  if (dim == 1) splitPos = centroidBounds.center().y;
  else if (dim == 2) splitPos = centroidBounds.center().z;

  int mid = startIndex;
  for (int i = startIndex; i < startIndex + numPrims; ++i) {
    float pos = primitives_[primitiveIndices_[i]].center().x;
    if (dim == 1) pos = primitives_[primitiveIndices_[i]].center().y;
    else if (dim == 2) pos = primitives_[primitiveIndices_[i]].center().z;

    if (pos < splitPos) {
      std::swap(primitiveIndices_[i], primitiveIndices_[mid]);
      mid++;
    }
  }

  if (mid == startIndex || mid == startIndex + numPrims) {
    mid = startIndex + numPrims / 2;
  }

  node->left = buildRecursive(startIndex, mid - startIndex);
  node->right = buildRecursive(mid, startIndex + numPrims - mid);
  return node;
}

bool BoundingVolumeHierarchy::intersectAABB(const AABB& bounds, const Ray3f& ray) const {
  float tmin = (bounds.min.x - ray.origin.x) * ray.invDir.x;
  float tmax = (bounds.max.x - ray.origin.x) * ray.invDir.x;
  if (tmin > tmax) std::swap(tmin, tmax);

  float tymin = (bounds.min.y - ray.origin.y) * ray.invDir.y;
  float tymax = (bounds.max.y - ray.origin.y) * ray.invDir.y;
  if (tymin > tymax) std::swap(tymin, tymax);

  if ((tmin > tymax) || (tymin > tmax)) return false;
  if (tymin > tmin) tmin = tymin;
  if (tymax < tmax) tmax = tymax;

  float tzmin = (bounds.min.z - ray.origin.z) * ray.invDir.z;
  float tzmax = (bounds.max.z - ray.origin.z) * ray.invDir.z;
  if (tzmin > tzmax) std::swap(tzmin, tzmax);

  if ((tmin > tzmax) || (tzmin > tmax)) return false;
  return true;
}

bool BoundingVolumeHierarchy::intersectNode(const std::shared_ptr<BoundingVolumeHierarchyNode>& node, const Ray3f& ray) const {
  if (!node) return false;
  if (!intersectAABB(node->bounds, ray)) return false;

  if (node->numPrimitives > 0) {
    // Leaf node
    return true; // Simple boolean hit test
  }

  bool hitL = intersectNode(node->left, ray);
  bool hitR = intersectNode(node->right, ray);
  return hitL || hitR;
}

bool BoundingVolumeHierarchy::intersect(const Ray3f& ray) const {
  return intersectNode(root_, ray);
}

} // namespace temene
