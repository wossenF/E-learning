#include "vm_garbage_collector.h"

namespace temene {

VmGarbageCollector::VmGarbageCollector() {}

VmGarbageCollector::~VmGarbageCollector() {
  for (GcObject* obj : heapObjects_) {
    delete obj;
  }
}

GcObject* VmGarbageCollector::allocateInt(int val) {
  GcObject* obj = new GcObject();
  obj->type = GcObjectType::INT;
  obj->intVal = val;
  heapObjects_.push_back(obj);
  bytesAllocated_ += sizeof(GcObject);
  return obj;
}

GcObject* VmGarbageCollector::allocateFloat(double val) {
  GcObject* obj = new GcObject();
  obj->type = GcObjectType::FLOAT;
  obj->floatVal = val;
  heapObjects_.push_back(obj);
  bytesAllocated_ += sizeof(GcObject);
  return obj;
}

GcObject* VmGarbageCollector::allocateString(const std::string& val) {
  GcObject* obj = new GcObject();
  obj->type = GcObjectType::STRING;
  obj->stringVal = val;
  heapObjects_.push_back(obj);
  bytesAllocated_ += sizeof(GcObject) + val.size();
  return obj;
}

GcObject* VmGarbageCollector::allocateArray() {
  GcObject* obj = new GcObject();
  obj->type = GcObjectType::ARRAY;
  heapObjects_.push_back(obj);
  bytesAllocated_ += sizeof(GcObject);
  return obj;
}

GcObject* VmGarbageCollector::allocateObject() {
  GcObject* obj = new GcObject();
  obj->type = GcObjectType::OBJECT;
  heapObjects_.push_back(obj);
  bytesAllocated_ += sizeof(GcObject);
  return obj;
}

void VmGarbageCollector::addRoot(GcObject* root) {
  if (root) roots_.insert(root);
}

void VmGarbageCollector::removeRoot(GcObject* root) {
  roots_.erase(root);
}

void VmGarbageCollector::markObject(GcObject* obj) {
  if (!obj || obj->isMarked) return;
  
  obj->isMarked = true;
  
  if (obj->type == GcObjectType::ARRAY) {
    for (GcObject* child : obj->arrayVal) {
      markObject(child);
    }
  } else if (obj->type == GcObjectType::OBJECT) {
    for (const auto& [k, v] : obj->objectVal) {
      markObject(v);
    }
  }
}

void VmGarbageCollector::collectGarbage() {
  for (GcObject* root : roots_) {
    markObject(root);
  }
  
  std::vector<GcObject*> liveObjects;
  size_t newBytesAllocated = 0;
  
  for (GcObject* obj : heapObjects_) {
    if (obj->isMarked) {
      obj->isMarked = false; // Reset for next GC cycle
      liveObjects.push_back(obj);
      
      size_t size = sizeof(GcObject);
      if (obj->type == GcObjectType::STRING) size += obj->stringVal.size();
      else if (obj->type == GcObjectType::ARRAY) size += obj->arrayVal.capacity() * sizeof(GcObject*);
      else if (obj->type == GcObjectType::OBJECT) size += obj->objectVal.bucket_count() * sizeof(void*);
      
      newBytesAllocated += size;
    } else {
      delete obj;
    }
  }
  
  heapObjects_ = std::move(liveObjects);
  bytesAllocated_ = newBytesAllocated;
}

} // namespace temene
