#ifndef DB_CORE_ENGINE_H
#define DB_CORE_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

namespace temene {

struct PageHeader {
  uint32_t pageId;
  uint16_t freeSpaceOffset;
  uint16_t tupleCount;
};

class StoragePage {
public:
  explicit StoragePage(uint32_t pageId, size_t pageSize = 4096);

  bool insertTuple(const std::vector<uint8_t>& tupleData, uint16_t& outSlot);
  bool readTuple(uint16_t slot, std::vector<uint8_t>& outData) const;

  uint16_t getFreeSpace() const;

private:
  size_t pageSize_;
  std::vector<uint8_t> buffer_;
  PageHeader header_;
};

class BufferPoolManager {
public:
  BufferPoolManager(size_t poolSize, size_t pageSize = 4096);

  std::shared_ptr<StoragePage> fetchPage(uint32_t pageId);
  void unpinPage(uint32_t pageId, bool isDirty);
  void flushAllPages();

private:
  size_t poolSize_;
  size_t pageSize_;
  std::unordered_map<uint32_t, std::shared_ptr<StoragePage>> pageTable_;
  std::unordered_map<uint32_t, int> pinCounts_;
  std::unordered_map<uint32_t, bool> dirtyFlags_;
};

} // namespace temene

#endif // DB_CORE_ENGINE_H
