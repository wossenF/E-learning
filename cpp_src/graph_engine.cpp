#include "graph_engine.h"
#include <cmath>

namespace temene {

void GraphEngine::addEdge(int u, int v, double weight, bool directed) {
  adjacencyList_[u].push_back({v, weight});
  if (!adjacencyList_.count(v)) {
    adjacencyList_[v] = {};
  }
  if (!directed) {
    adjacencyList_[v].push_back({u, weight});
  }
}

std::vector<int> GraphEngine::dijkstraShortestPath(int startNode, int targetNode, double& outDistance) const {
  std::unordered_map<int, double> dist;
  std::unordered_map<int, int> parent;
  using Pair = std::pair<double, int>;
  std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> pq;

  for (const auto& [node, _] : adjacencyList_) {
    dist[node] = std::numeric_limits<double>::infinity();
  }

  dist[startNode] = 0.0;
  pq.push({0.0, startNode});

  while (!pq.empty()) {
    auto [d, u] = pq.top();
    pq.pop();

    if (d > dist[u]) continue;
    if (u == targetNode) break;

    auto it = adjacencyList_.find(u);
    if (it != adjacencyList_.end()) {
      for (const auto& edge : it->second) {
        int v = edge.targetNode;
        double w = edge.weight;
        if (dist[u] + w < dist[v]) {
          dist[v] = dist[u] + w;
          parent[v] = u;
          pq.push({dist[v], v});
        }
      }
    }
  }

  outDistance = dist[targetNode];
  if (outDistance == std::numeric_limits<double>::infinity()) {
    return {};
  }

  std::vector<int> path;
  for (int curr = targetNode; curr != startNode; curr = parent[curr]) {
    path.push_back(curr);
  }
  path.push_back(startNode);
  std::reverse(path.begin(), path.end());
  return path;
}

static double euclideanDistance(std::pair<double, double> p1, std::pair<double, double> p2) {
  double dx = p1.first - p2.first;
  double dy = p1.second - p2.second;
  return std::sqrt(dx * dx + dy * dy);
}

std::vector<int> GraphEngine::aStarSearch(
  int startNode,
  int targetNode,
  const std::unordered_map<int, std::pair<double, double>>& coordinates,
  double& outDistance
) const {
  auto targetCoordIt = coordinates.find(targetNode);
  if (targetCoordIt == coordinates.end()) {
    return dijkstraShortestPath(startNode, targetNode, outDistance);
  }

  std::pair<double, double> targetCoord = targetCoordIt->second;

  std::unordered_map<int, double> gScore;
  std::unordered_map<int, double> fScore;
  std::unordered_map<int, int> parent;

  for (const auto& [node, _] : adjacencyList_) {
    gScore[node] = std::numeric_limits<double>::infinity();
    fScore[node] = std::numeric_limits<double>::infinity();
  }

  gScore[startNode] = 0.0;
  auto startCoordIt = coordinates.find(startNode);
  double h0 = startCoordIt != coordinates.end() ? euclideanDistance(startCoordIt->second, targetCoord) : 0.0;
  fScore[startNode] = h0;

  using Pair = std::pair<double, int>;
  std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> openSet;
  openSet.push({fScore[startNode], startNode});

  while (!openSet.empty()) {
    auto [f, u] = openSet.top();
    openSet.pop();

    if (u == targetNode) break;

    auto it = adjacencyList_.find(u);
    if (it == adjacencyList_.end()) continue;

    for (const auto& edge : it->second) {
      int v = edge.targetNode;
      double tentativeG = gScore[u] + edge.weight;
      if (tentativeG < gScore[v]) {
        parent[v] = u;
        gScore[v] = tentativeG;

        auto vCoordIt = coordinates.find(v);
        double h = vCoordIt != coordinates.end() ? euclideanDistance(vCoordIt->second, targetCoord) : 0.0;
        fScore[v] = gScore[v] + h;
        openSet.push({fScore[v], v});
      }
    }
  }

  outDistance = gScore[targetNode];
  if (outDistance == std::numeric_limits<double>::infinity()) {
    return {};
  }

  std::vector<int> path;
  for (int curr = targetNode; curr != startNode; curr = parent[curr]) {
    path.push_back(curr);
  }
  path.push_back(startNode);
  std::reverse(path.begin(), path.end());
  return path;
}

std::vector<std::vector<int>> GraphEngine::findStronglyConnectedComponents() const {
  int index = 0;
  std::unordered_map<int, int> indices;
  std::unordered_map<int, int> lowLink;
  std::stack<int> st;
  std::unordered_set<int> onStack;
  std::vector<std::vector<int>> sccs;

  for (const auto& [u, _] : adjacencyList_) {
    if (indices.find(u) == indices.end()) {
      tarjanScc(u, index, indices, lowLink, st, onStack, sccs);
    }
  }

  return sccs;
}

void GraphEngine::tarjanScc(
  int u,
  int& index,
  std::unordered_map<int, int>& indices,
  std::unordered_map<int, int>& lowLink,
  std::stack<int>& st,
  std::unordered_set<int>& onStack,
  std::vector<std::vector<int>>& sccs
) const {
  indices[u] = index;
  lowLink[u] = index;
  index++;
  st.push(u);
  onStack.insert(u);

  auto it = adjacencyList_.find(u);
  if (it != adjacencyList_.end()) {
    for (const auto& edge : it->second) {
      int v = edge.targetNode;
      if (indices.find(v) == indices.end()) {
        tarjanScc(v, index, indices, lowLink, st, onStack, sccs);
        lowLink[u] = std::min(lowLink[u], lowLink[v]);
      } else if (onStack.count(v)) {
        lowLink[u] = std::min(lowLink[u], indices[v]);
      }
    }
  }

  if (lowLink[u] == indices[u]) {
    std::vector<int> scc;
    while (true) {
      int v = st.top();
      st.pop();
      onStack.erase(v);
      scc.push_back(v);
      if (v == u) break;
    }
    sccs.push_back(scc);
  }
}

std::vector<int> GraphEngine::topologicalSort() const {
  std::unordered_map<int, int> inDegree;
  for (const auto& [u, edges] : adjacencyList_) {
    if (inDegree.find(u) == inDegree.end()) inDegree[u] = 0;
    for (const auto& e : edges) {
      inDegree[e.targetNode]++;
    }
  }

  std::queue<int> q;
  for (const auto& [u, deg] : inDegree) {
    if (deg == 0) q.push(u);
  }

  std::vector<int> order;
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    order.push_back(u);

    auto it = adjacencyList_.find(u);
    if (it != adjacencyList_.end()) {
      for (const auto& edge : it->second) {
        int v = edge.targetNode;
        inDegree[v]--;
        if (inDegree[v] == 0) q.push(v);
      }
    }
  }

  return order;
}

double GraphEngine::maxFlowEdmondsKarp(int source, int sink) const {
  std::unordered_map<int, std::unordered_map<int, double>> capacity;
  for (const auto& [u, edges] : adjacencyList_) {
    for (const auto& e : edges) {
      capacity[u][e.targetNode] += e.weight;
      if (!capacity[e.targetNode].count(u)) capacity[e.targetNode][u] = 0.0;
    }
  }

  double maxFlow = 0.0;

  while (true) {
    std::unordered_map<int, int> parent;
    std::queue<int> q;
    q.push(source);

    while (!q.empty() && !parent.count(sink)) {
      int u = q.front();
      q.pop();

      auto it = capacity.find(u);
      if (it == capacity.end()) continue;

      for (const auto& [v, cap] : it->second) {
        if (cap > 0 && !parent.count(v) && v != source) {
          parent[v] = u;
          q.push(v);
        }
      }
    }

    if (!parent.count(sink)) break;

    double pathFlow = std::numeric_limits<double>::infinity();
    for (int curr = sink; curr != source; curr = parent[curr]) {
      int prev = parent[curr];
      pathFlow = std::min(pathFlow, capacity[prev][curr]);
    }

    for (int curr = sink; curr != source; curr = parent[curr]) {
      int prev = parent[curr];
      capacity[prev][curr] -= pathFlow;
      capacity[curr][prev] += pathFlow;
    }

    maxFlow += pathFlow;
  }

  return maxFlow;
}

} // namespace temene
