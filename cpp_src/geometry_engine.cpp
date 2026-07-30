#include "geometry_engine.h"
#include <algorithm>

namespace temene {

// Möller–Trumbore ray-triangle intersection algorithm
bool GeometryEngine::rayTriangleIntersection(
  const Ray& ray,
  const Triangle& tri,
  double& outT,
  double& outU,
  double& outV
) {
  const double EPSILON = 1e-7;
  Vector3D edge1 = tri.v1 - tri.v0;
  Vector3D edge2 = tri.v2 - tri.v0;

  Vector3D h = ray.direction.cross(edge2);
  double a = edge1.dot(h);

  if (a > -EPSILON && a < EPSILON) return false; // Ray is parallel to triangle

  double f = 1.0 / a;
  Vector3D s = ray.origin - tri.v0;
  outU = f * s.dot(h);

  if (outU < 0.0 || outU > 1.0) return false;

  Vector3D q = s.cross(edge1);
  outV = f * ray.direction.dot(q);

  if (outV < 0.0 || outU + outV > 1.0) return false;

  outT = f * edge2.dot(q);
  return outT > EPSILON;
}

static double cross2D(const Vector3D& O, const Vector3D& A, const Vector3D& B) {
  return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

std::vector<Vector3D> GeometryEngine::computeConvexHull2D(std::vector<Vector3D> points) {
  size_t n = points.size();
  if (n <= 3) return points;

  std::sort(points.begin(), points.end(), [](const Vector3D& a, const Vector3D& b) {
    return a.x < b.x || (a.x == b.x && a.y < b.y);
  });

  std::vector<Vector3D> hull(2 * n);
  size_t k = 0;

  // Lower hull
  for (size_t i = 0; i < n; ++i) {
    while (k >= 2 && cross2D(hull[k - 2], hull[k - 1], points[i]) <= 0) k--;
    hull[k++] = points[i];
  }

  // Upper hull
  for (size_t i = n - 1, t = k + 1; i > 0; i--) {
    while (k >= t && cross2D(hull[k - 2], hull[k - 1], points[i - 1]) <= 0) k--;
    hull[k++] = points[i - 1];
  }

  hull.resize(k - 1);
  return hull;
}

} // namespace temene
