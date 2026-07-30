#include "db_core_engine.h"
#include <cstring>
#include <stdexcept>

namespace temene {

StoragePage::StoragePage(uint32_t pageId, size_t pageSize)
  : pageSize_(pageSize), buffer_(pageSize, 0) {
  header_.pageId = pageId;
  header_.freeSpaceOffset = static_cast<uint16_t>(pageSize);
  header_.tupleCount = 0;
  std::memcpy(buffer_.data(), &header_, sizeof(PageHeader));
}

bool StoragePage::insertTuple(const std::vector<uint8_t>& tupleData, uint16_t& outSlot) {
  size_t required = tupleData.size() + sizeof(uint16_t) * 2;
  if (getFreeSpace() < required) return false;

  header_.freeSpaceOffset -= static_cast<uint16_t>(tupleData.size());
  std::memcpy(buffer_.data() + header_.freeSpaceOffset, tupleData.data(), tupleData.size());

  outSlot = header_.tupleCount;
  header_.tupleCount++;

  size_t slotOffset = sizeof(PageHeader) + outSlot * sizeof(uint16_t) * 2;
  uint16_t offsetVal = header_.freeSpaceOffset;
  uint16_t lenVal = static_cast<uint16_t>(tupleData.size());

  std::memcpy(buffer_.data() + slotOffset, &offsetVal, sizeof(uint16_t));
  std::memcpy(buffer_.data() + slotOffset + sizeof(uint16_t), &lenVal, sizeof(uint16_t));

  std::memcpy(buffer_.data(), &header_, sizeof(PageHeader));
  return true;
}

bool StoragePage::readTuple(uint16_t slot, std::vector<uint8_t>& outData) const {
  if (slot >= header_.tupleCount) return false;

  size_t slotOffset = sizeof(PageHeader) + slot * sizeof(uint16_t) * 2;
  uint16_t offsetVal, lenVal;

  std::memcpy(&offsetVal, buffer_.data() + slotOffset, sizeof(uint16_t));
  std::memcpy(&lenVal, buffer_.data() + slotOffset + sizeof(uint16_t), sizeof(uint16_t));

  outData.resize(lenVal);
  std::memcpy(outData.data(), buffer_.data() + offsetVal, lenVal);
  return true;
}

uint16_t StoragePage::getFreeSpace() const {
  size_t slotsEnd = sizeof(PageHeader) + header_.tupleCount * sizeof(uint16_t) * 2;
  if (header_.freeSpaceOffset <= slotsEnd) return 0;
  return header_.freeSpaceOffset - static_cast<uint16_t>(slotsEnd);
}

// BufferPoolManager implementation
BufferPoolManager::BufferPoolManager(size_t poolSize, size_t pageSize)
  : poolSize_(poolSize), pageSize_(pageSize) {}

std::shared_ptr<StoragePage> BufferPoolManager::fetchPage(uint32_t pageId) {
  auto it = pageTable_.find(pageId);
  if (it != pageTable_.end()) {
    pinCounts_[pageId]++;
    return it->second;
  }

  if (pageTable_.size() >= poolSize_) {
    // Evict page with pinCount == 0
    uint32_t evictId = 0;
    bool found = false;
    for (const auto& [pId, pCount] : pinCounts_) {
      if (pCount == 0) {
        evictId = pId;
        found = true;
        break;
      }
    }
    if (!found) throw std::runtime_error("Buffer pool full and all pages are pinned.");

    pageTable_.erase(evictId);
    pinCounts_.erase(evictId);
    dirtyFlags_.erase(evictId);
  }

  auto page = std::make_shared<StoragePage>(pageId, pageSize_);
  pageTable_[pageId] = page;
  pinCounts_[pageId] = 1;
  dirtyFlags_[pageId] = false;

  return page;
}

void BufferPoolManager::unpinPage(uint32_t pageId, bool isDirty) {
  auto it = pinCounts_.find(pageId);
  if (it != pinCounts_.end() && it->second > 0) {
    it->second--;
  }
  if (isDirty) {
    dirtyFlags_[pageId] = true;
  }
}

void BufferPoolManager::flushAllPages() {
  for (const auto& [pId, dirty] : dirtyFlags_) {
    if (dirty) {
      // Flush to disk
    }
  }
  dirtyFlags_.clear();
}

} // namespace temene
