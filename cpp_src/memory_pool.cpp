#include "memory_pool.h"
#include <cstdlib>
#include <algorithm>

namespace temene {

SlabAllocator::SlabAllocator(size_t objectSize, size_t objectsPerSlab)
  : objectSize_(std::max(objectSize, sizeof(SlabNode))), objectsPerSlab_(objectsPerSlab) {}

SlabAllocator::~SlabAllocator() {
  for (void* slab : slabs_) {
    std::free(slab);
  }
}

void SlabAllocator::allocateNewSlab() {
  size_t slabBytes = objectSize_ * objectsPerSlab_;
  void* rawSlab = std::malloc(slabBytes);
  slabs_.push_back(rawSlab);

  uint8_t* curr = static_cast<uint8_t*>(rawSlab);
  for (size_t i = 0; i < objectsPerSlab_; ++i) {
    SlabNode* node = reinterpret_cast<SlabNode*>(curr + i * objectSize_);
    node->next = freeList_;
    freeList_ = node;
  }
}

void* SlabAllocator::allocate() {
  if (!freeList_) {
    allocateNewSlab();
  }
  SlabNode* node = freeList_;
  freeList_ = node->next;
  allocatedCount_++;
  return static_cast<void*>(node);
}

void SlabAllocator::deallocate(void* ptr) {
  if (!ptr) return;
  SlabNode* node = static_cast<SlabNode*>(ptr);
  node->next = freeList_;
  freeList_ = node;
  if (allocatedCount_ > 0) allocatedCount_--;
}

// TriColorGarbageCollector implementation
GcHeader* TriColorGarbageCollector::allocate(size_t size) {
  GcHeader* header = new GcHeader();
  header->color = GcColor::WHITE;
  header->size = size;
  heapObjects_.push_back(header);
  return header;
}

void TriColorGarbageCollector::addRoot(GcHeader* root) {
  if (root) roots_.insert(root);
}

void TriColorGarbageCollector::removeRoot(GcHeader* root) {
  roots_.erase(root);
}

void TriColorGarbageCollector::markPhase(std::vector<GcHeader*>& greyList) {
  for (GcHeader* root : roots_) {
    root->color = GcColor::GREY;
    greyList.push_back(root);
  }

  while (!greyList.empty()) {
    GcHeader* obj = greyList.back();
    greyList.pop_back();

    obj->color = GcColor::BLACK;
    for (GcHeader* ref : obj->references) {
      if (ref && ref->color == GcColor::WHITE) {
        ref->color = GcColor::GREY;
        greyList.push_back(ref);
      }
    }
  }
}

void TriColorGarbageCollector::sweepPhase() {
  auto it = heapObjects_.begin();
  while (it != heapObjects_.end()) {
    GcHeader* obj = *it;
    if (obj->color == GcColor::WHITE) {
      delete obj;
      it = heapObjects_.erase(it);
    } else {
      obj->color = GcColor::WHITE; // Reset color for next GC cycle
      ++it;
    }
  }
}

void TriColorGarbageCollector::collectGarbage() {
  std::vector<GcHeader*> greyList;
  markPhase(greyList);
  sweepPhase();
}

} // namespace temene
