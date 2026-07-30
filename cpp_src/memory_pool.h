#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstddef>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <cstdint>

namespace temene {

class SlabAllocator {
public:
  SlabAllocator(size_t objectSize, size_t objectsPerSlab = 64);
  ~SlabAllocator();

  void* allocate();
  void deallocate(void* ptr);

  size_t allocatedCount() const { return allocatedCount_; }
  size_t totalCapacity() const { return slabs_.size() * objectsPerSlab_; }

private:
  struct SlabNode {
    void* next{nullptr};
  };

  size_t objectSize_;
  size_t objectsPerSlab_;
  size_t allocatedCount_{0};
  std::vector<void*> slabs_;
  SlabNode* freeList_{nullptr};

  void allocateNewSlab();
};

enum class GcColor { WHITE, GREY, BLACK };

struct GcHeader {
  GcColor color{GcColor::WHITE};
  size_t size{0};
  std::vector<GcHeader*> references;
};

class TriColorGarbageCollector {
public:
  GcHeader* allocate(size_t size);
  void addRoot(GcHeader* root);
  void removeRoot(GcHeader* root);

  void collectGarbage();

  size_t totalAllocatedObjects() const { return heapObjects_.size(); }

private:
  std::unordered_set<GcHeader*> roots_;
  std::vector<GcHeader*> heapObjects_;

  void markPhase(std::vector<GcHeader*>& greyList);
  void sweepPhase();
};

} // namespace temene

#endif // MEMORY_POOL_H
