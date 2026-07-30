#include "lsm_storage_engine.h"

namespace temene {

void LsmMemTable::put(const std::string& key, const std::string& value) {
  auto it = memMap_.find(key);
  if (it != memMap_.end()) {
    currentByteSize_ -= (it->first.size() + it->second.size());
  }
  memMap_[key] = value;
  currentByteSize_ += (key.size() + value.size());
}

bool LsmMemTable::get(const std::string& key, std::string& outValue) const {
  auto it = memMap_.find(key);
  if (it != memMap_.end()) {
    outValue = it->second;
    return true;
  }
  return false;
}

void LsmMemTable::remove(const std::string& key) {
  put(key, "__TOMBSTONE__");
}

void LsmMemTable::clear() {
  memMap_.clear();
  currentByteSize_ = 0;
}

// LsmStorageEngine implementation
LsmStorageEngine::LsmStorageEngine(size_t memtableThresholdBytes)
  : thresholdBytes_(memtableThresholdBytes) {}

void LsmStorageEngine::put(const std::string& key, const std::string& value) {
  activeMemTable_.put(key, value);
  if (activeMemTable_.byteSize() >= thresholdBytes_) {
    flushMemTable();
  }
}

void LsmStorageEngine::flushMemTable() {
  if (activeMemTable_.byteSize() == 0) return;
  flushedSstables_.push_back(activeMemTable_.getEntries());
  activeMemTable_.clear();
}

std::string LsmStorageEngine::get(const std::string& key) {
  std::string val;
  if (activeMemTable_.get(key, val)) {
    return val == "__TOMBSTONE__" ? "" : val;
  }

  for (auto it = flushedSstables_.rbegin(); it != flushedSstables_.rend(); ++it) {
    auto entryIt = it->find(key);
    if (entryIt != it->end()) {
      return entryIt->second == "__TOMBSTONE__" ? "" : entryIt->second;
    }
  }

  return "";
}

} // namespace temene
