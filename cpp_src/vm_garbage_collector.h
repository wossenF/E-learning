#ifndef VM_GARBAGE_COLLECTOR_H
#define VM_GARBAGE_COLLECTOR_H

#include <vector>
#include <unordered_set>
#include <cstdint>
#include <memory>
#include <iostream>

namespace temene {

enum class GcObjectType { INT, FLOAT, STRING, ARRAY, OBJECT };

struct GcObject {
  GcObjectType type;
  bool isMarked{false};
  
  // payload
  int intVal{0};
  double floatVal{0.0};
  std::string stringVal;
  std::vector<GcObject*> arrayVal;
  std::unordered_map<std::string, GcObject*> objectVal;
};

class VmGarbageCollector {
public:
  VmGarbageCollector();
  ~VmGarbageCollector();
  
  GcObject* allocateInt(int val);
  GcObject* allocateFloat(double val);
  GcObject* allocateString(const std::string& val);
  GcObject* allocateArray();
  GcObject* allocateObject();
  
  void addRoot(GcObject* root);
  void removeRoot(GcObject* root);
  
  void collectGarbage();
  
  size_t objectCount() const { return heapObjects_.size(); }
  size_t totalBytesAllocated() const { return bytesAllocated_; }

private:
  std::vector<GcObject*> heapObjects_;
  std::unordered_set<GcObject*> roots_;
  size_t bytesAllocated_{0};
  
  void markObject(GcObject* obj);
};

} // namespace temene

#endif // VM_GARBAGE_COLLECTOR_H
