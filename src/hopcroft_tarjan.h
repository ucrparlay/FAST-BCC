#include <stack>
#include <unordered_set>

#include "graph.h"

struct Tarjan {
  const Graph &G;
  sequence<NodeId> low;
  sequence<NodeId> dfn;
  sequence<NodeId> label;
  // unordered_set<NodeId> articulation_point;
  stack<pair<NodeId, NodeId>> sk;
  // set<pair<NodeId, NodeId>> bridge;
  NodeId idx, k, root;
  Tarjan(const Graph &_G) : G(_G) {
    idx = k = 0;
    low = sequence<NodeId>(G.n);
    dfn = sequence<NodeId>(G.n, 0);
    label = sequence<NodeId>(G.n);
  }
  void dfs(NodeId u, NodeId f = UINT_MAX) {
    dfn[u] = low[u] = ++idx;
    // int cnt = 0;
    for (size_t j = G.offset[u]; j < G.offset[u + 1]; j++) {
      auto v = G.E[j];
      if (!dfn[v]) {
        sk.push({u, v});
        dfs(v, u);
        low[u] = min(low[u], low[v]);
        if (low[v] >= dfn[u]) {
          k++;
          while (true) {
            auto [f, s] = sk.top();
            sk.pop();
            label[f] = label[s] = k;
            if (f == u && s == v) {
              break;
            }
          }
          // cnt++;
          // if (u != root || (u == root && cnt >= 2)) {
          // articulation_point.insert(u);
          //}
          // if (low[v] > dfn[u]) {
          // if (u < v) {
          // bridge.insert({u, v});
          //} else {
          // bridge.insert({v, u});
          //}
          //}
        }
      } else if (v != f) {
        low[u] = min(low[u], dfn[v]);
      }
    }
  }
  void clear() {
    idx = k = 0;
    for (size_t i = 0; i < G.n; i++) {
      dfn[i] = 0;
    }
    sk = stack<pair<NodeId, NodeId>>();
  }

  void bcc() {
    for (size_t i = 0; i < G.n; i++) {
      if (!dfn[i]) {
        root = i;
        dfs(i);
      }
    }
    // printf("# bridge: %zu\n", bridge.size());
  }
};
