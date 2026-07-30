#include "bplus_tree.h"
#include <chrono>

namespace temene {

void runBPlusTreeBenchmark() {
  BPlusTree<int, std::string> tree;
  std::cout << "[BPlusTree] Initializing benchmark..." << std::endl;

  for (int i = 0; i < 1000; ++i) {
    tree.insert(i, "val_" + std::to_string(i));
  }

  std::string val;
  if (tree.find(500, val)) {
    std::cout << "[BPlusTree] Found key 500 => " << val << std::endl;
  }

  auto rangeRes = tree.rangeSearch(100, 110);
  std::cout << "[BPlusTree] Range search [100, 110] size: " << rangeRes.size() << std::endl;
}

} // namespace temene
