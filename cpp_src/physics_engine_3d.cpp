#include "physics_engine_3d.h"
#include <algorithm>

namespace temene {

PhysicsWorld::PhysicsWorld(const Vector3& gravity) : gravity_(gravity) {}

void PhysicsWorld::addBody(std::shared_ptr<RigidBody> body) {
  bodies_.push_back(body);
}

void PhysicsWorld::step(double dt) {
  for (auto& body : bodies_) {
    body->applyForce(gravity_ * body->mass);
    body->integrate(dt);
  }
  
  resolveCollisions();
}

bool PhysicsWorld::checkSphereCollision(const std::shared_ptr<RigidBody>& a, const std::shared_ptr<RigidBody>& b, double radiusA, double radiusB, Vector3& normal, double& depth) {
  Vector3 diff = b->position - a->position;
  double dist = diff.length();
  double sumRadii = radiusA + radiusB;
  
  if (dist < sumRadii) {
    if (dist == 0.0) {
      normal = {0, 1, 0};
      depth = sumRadii;
    } else {
      normal = diff / dist;
      depth = sumRadii - dist;
    }
    return true;
  }
  return false;
}

void PhysicsWorld::resolveCollisions() {
  // Simple O(N^2) sphere collision detection and resolution
  double radius = 1.0; // Assume all bodies are spheres of radius 1.0 for this mock
  
  for (size_t i = 0; i < bodies_.size(); ++i) {
    for (size_t j = i + 1; j < bodies_.size(); ++j) {
      auto& a = bodies_[i];
      auto& b = bodies_[j];
      
      if (a->mass <= 0.0 && b->mass <= 0.0) continue;
      
      Vector3 normal;
      double depth;
      if (checkSphereCollision(a, b, radius, radius, normal, depth)) {
        // Positional correction
        double invMassA = (a->mass > 0.0) ? 1.0 / a->mass : 0.0;
        double invMassB = (b->mass > 0.0) ? 1.0 / b->mass : 0.0;
        double massSum = invMassA + invMassB;
        
        Vector3 correction = normal * (depth / massSum);
        a->position -= correction * invMassA;
        b->position += correction * invMassB;
        
        // Velocity resolution
        Vector3 relVel = b->velocity - a->velocity;
        double velAlongNormal = relVel.dot(normal);
        
        if (velAlongNormal > 0) continue;
        
        double e = std::min(a->restitution, b->restitution);
        double jImpulse = -(1.0 + e) * velAlongNormal;
        jImpulse /= massSum;
        
        Vector3 impulse = normal * jImpulse;
        a->velocity -= impulse * invMassA;
        b->velocity += impulse * invMassB;
      }
    }
  }
}

} // namespace temene
