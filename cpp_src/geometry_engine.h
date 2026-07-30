#ifndef GEOMETRY_ENGINE_H
#define GEOMETRY_ENGINE_H

#include <vector>
#include <cmath>
#include <iostream>

namespace temene {

struct Vector3D {
  double x{0.0}, y{0.0}, z{0.0};

  Vector3D operator+(const Vector3D& o) const { return {x + o.x, y + o.y, z + o.z}; }
  Vector3D operator-(const Vector3D& o) const { return {x - o.x, y - o.y, z - o.z}; }
  Vector3D operator*(double s) const { return {x * s, y * s, z * s}; }

  double dot(const Vector3D& o) const { return x * o.x + y * o.y + z * o.z; }
  Vector3D cross(const Vector3D& o) const {
    return {
      y * o.z - z * o.y,
      z * o.x - x * o.z,
      x * o.y - y * o.x
    };
  }

  double length() const { return std::sqrt(dot(*this)); }
  Vector3D normalize() const {
    double l = length();
    return l > 0 ? (*this) * (1.0 / l) : Vector3D{0, 0, 0};
  }
};

struct Ray {
  Vector3D origin;
  Vector3D direction;
};

struct Triangle {
  Vector3D v0, v1, v2;
};

class GeometryEngine {
public:
  static bool rayTriangleIntersection(
    const Ray& ray,
    const Triangle& tri,
    double& outT,
    double& outU,
    double& outV
  );

  static std::vector<Vector3D> computeConvexHull2D(std::vector<Vector3D> points);
};

} // namespace temene

#endif // GEOMETRY_ENGINE_H
