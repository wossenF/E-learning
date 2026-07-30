#ifndef PHYSICS_ENGINE_3D_H
#define PHYSICS_ENGINE_3D_H

#include <vector>
#include <cmath>
#include <memory>

namespace temene {

struct Vector3 {
  double x{0.0}, y{0.0}, z{0.0};
  
  Vector3 operator+(const Vector3& o) const { return {x + o.x, y + o.y, z + o.z}; }
  Vector3 operator-(const Vector3& o) const { return {x - o.x, y - o.y, z - o.z}; }
  Vector3 operator*(double s) const { return {x * s, y * s, z * s}; }
  Vector3 operator/(double s) const { return {x / s, y / s, z / s}; }
  
  Vector3& operator+=(const Vector3& o) { x += o.x; y += o.y; z += o.z; return *this; }
  
  double dot(const Vector3& o) const { return x * o.x + y * o.y + z * o.z; }
  Vector3 cross(const Vector3& o) const {
    return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
  }
  double length() const { return std::sqrt(dot(*this)); }
  Vector3 normalize() const {
    double l = length();
    if (l > 0) return (*this) / l;
    return {0, 0, 0};
  }
};

class RigidBody {
public:
  Vector3 position;
  Vector3 velocity;
  Vector3 force;
  
  double mass{1.0};
  double restitution{0.5}; // bounciness
  
  void applyForce(const Vector3& f) { force += f; }
  void integrate(double dt) {
    if (mass <= 0.0) return;
    Vector3 acceleration = force / mass;
    velocity += acceleration * dt;
    position += velocity * dt;
    force = {0, 0, 0};
  }
};

class PhysicsWorld {
public:
  PhysicsWorld(const Vector3& gravity = {0, -9.81, 0});
  
  void addBody(std::shared_ptr<RigidBody> body);
  void step(double dt);

private:
  Vector3 gravity_;
  std::vector<std::shared_ptr<RigidBody>> bodies_;
  
  void resolveCollisions();
  bool checkSphereCollision(const std::shared_ptr<RigidBody>& a, const std::shared_ptr<RigidBody>& b, double radiusA, double radiusB, Vector3& normal, double& depth);
};

} // namespace temene

#endif // PHYSICS_ENGINE_3D_H
