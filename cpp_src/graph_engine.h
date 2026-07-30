#ifndef GRAPH_ENGINE_H
#define GRAPH_ENGINE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <limits>
#include <algorithm>
#include <iostream>

namespace temene {

struct GraphEdge {
  int targetNode;
  double weight;
};

class GraphEngine {
public:
  void addEdge(int u, int v, double weight = 1.0, bool directed = true);

  std::vector<int> dijkstraShortestPath(int startNode, int targetNode, double& outDistance) const;
  std::vector<int> aStarSearch(int startNode, int targetNode, const std::unordered_map<int, std::pair<double, double>>& coordinates, double& outDistance) const;

  std::vector<std::vector<int>> findStronglyConnectedComponents() const;
  std::vector<int> topologicalSort() const;

  double maxFlowEdmondsKarp(int source, int sink) const;

  size_t nodeCount() const { return adjacencyList_.size(); }

private:
  std::unordered_map<int, std::vector<GraphEdge>> adjacencyList_;

  void tarjanScc(int u, int& index, std::unordered_map<int, int>& indices, std::unordered_map<int, int>& lowLink, std::stack<int>& st, std::unordered_set<int>& onStack, std::vector<std::vector<int>>& sccs) const;
};

} // namespace temene

#endif // GRAPH_ENGINE_H
