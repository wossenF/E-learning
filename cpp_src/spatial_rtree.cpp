#include "spatial_rtree.h"

namespace temene {

void runRTreeTest() {
  RTree<std::string> tree;
  tree.insert({0, 0, 10, 10}, "LocationA");
  tree.insert({5, 5, 15, 15}, "LocationB");

  auto found = tree.query({2, 2, 8, 8});
  std::cout << "[RTree] Found spatial items: " << found.size() << std::endl;
}

} // namespace temene
