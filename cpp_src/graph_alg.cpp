#include <vector>
#include <queue>
#include <limits>

// Basic graph algorithms: BFS and shortest path on unweighted graph

std::vector<int> bfs_distance(const std::vector<std::vector<int>>& adj, int start) {
    int n = (int)adj.size();
    std::vector<int> dist(n, -1);
    std::queue<int> q; dist[start]=0; q.push(start);
    while (!q.empty()) {
        int u=q.front(); q.pop();
        for (int v: adj[u]) if (dist[v]==-1) { dist[v]=dist[u]+1; q.push(v); }
    }
    return dist;
}

int shortest_path_unweighted(const std::vector<std::vector<int>>& adj, int s, int t) {
    auto d = bfs_distance(adj, s);
    return (t < 0 || t >= (int)d.size()) ? -1 : d[t];
}
