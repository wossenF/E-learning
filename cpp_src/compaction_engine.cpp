#include "compaction_engine.h"
#include <algorithm>
#include <map>

namespace temene {

SstableReader::SstableReader(std::vector<SstableEntry> entries)
  : entries_(std::move(entries)) {
  std::sort(entries_.begin(), entries_.end(), [](const SstableEntry& a, const SstableEntry& b) {
    return a.key < b.key;
  });
}

bool SstableReader::get(const std::string& key, SstableEntry& outEntry) const {
  auto it = std::lower_bound(entries_.begin(), entries_.end(), key, [](const SstableEntry& e, const std::string& k) {
    return e.key < k;
  });
  if (it != entries_.end() && it->key == key) {
    outEntry = *it;
    return true;
  }
  return false;
}

void LevelCompactionEngine::addLevel0Sstable(std::shared_ptr<SstableReader> sstable) {
  level0Tables_.push_back(sstable);
}

std::shared_ptr<SstableReader> LevelCompactionEngine::mergeCompaction(
  const std::vector<std::shared_ptr<SstableReader>>& inputTables
) {
  std::map<std::string, SstableEntry> mergedMap;

  for (const auto& table : inputTables) {
    for (const auto& entry : table->getEntries()) {
      auto it = mergedMap.find(entry.key);
      if (it == mergedMap.end() || entry.timestamp > it->second.timestamp) {
        mergedMap[entry.key] = entry;
      }
    }
  }

  std::vector<SstableEntry> mergedEntries;
  for (const auto& [_, entry] : mergedMap) {
    if (!entry.isTombstone) {
      mergedEntries.push_back(entry);
    }
  }

  return std::make_shared<SstableReader>(mergedEntries);
}

} // namespace temene
