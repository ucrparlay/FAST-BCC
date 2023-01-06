#pragma once
#include "BCC.h"
#include "resizable_table.h"
using namespace std;
using namespace parlay;

struct Tarjan_Vishkin : public BCC {
  Tarjan_Vishkin(const Graph &_G, double _beta = 0.2) : BCC(_G, _beta) {}
  auto biconnectivity() {
    F = spanning_forest(G, beta);
    euler_tour_tree();
    compute();

    sequence<pair<NodeId, NodeId>> edgelist(G.m);
    parallel_for(0, G.n, [&](size_t i) {
      parallel_for(G.offset[i], G.offset[i + 1], [&](size_t j) {
        NodeId u = i, v = G.E[j];
        edgelist[j] = make_pair(u, v);
      });
    });
    edgelist = filter(edgelist, [](const pair<NodeId, NodeId> &p) {
      return p.first < p.second;
    });
    edgelist =
        remove_duplicates_ordered(edgelist, less<pair<NodeId, NodeId>>());

    auto parent_edge = sequence<EdgeId>::uninitialized(G.n);
    parallel_for(0, edgelist.size(), [&](size_t i) {
      NodeId u = edgelist[i].first, v = edgelist[i].second;
      if (u < v) {
        if (v == parent[u]) {
          parent_edge[u] = i;
        }
        if (u == parent[v]) {
          parent_edge[v] = i;
        }
      }
    });
    // Use NodeId because there is not enough memory to save larger graphs
    auto table = gbbs::resizable_table<pair<NodeId, NodeId>, hash_k>(
        edgelist.size() * 4, {UINT_N_MAX, UINT_N_MAX}, hash_k());
    auto check_edge = [&](NodeId u, NodeId v, EdgeId j) {
      if (parent[u] != v && parent[v] != u) {
        if (first[v] < first[u]) {
          table.insert({j, parent_edge[u]});
        }
        if (last[v] <= first[u]) {
          table.insert({parent_edge[v], parent_edge[u]});
        }
      } else {
        if (parent[v] == u) {
          swap(v, u);  // enforce (parent[u] = v)
        }
        if (v != parent[v]) {
          if (low[u] < first[v] || high[u] >= last[v]) {
            table.insert({j, parent_edge[v]});
          }
        }
      }
    };
    parallel_for(0, edgelist.size(), [&](size_t i) {
      NodeId u = edgelist[i].first, v = edgelist[i].second;
      if (u < v) {
        check_edge(u, v, i);
        check_edge(v, u, i);
      }
    });
    auto edges = table.entries();
    auto sym_edges_delayed =
        delayed_seq<pair<NodeId, NodeId>>(edges.size() * 2, [&](size_t i) {
          if (i % 2 == 0) {
            return make_pair(get<0>(edges[i / 2]), get<1>(edges[i / 2]));
          } else {
            return make_pair(get<1>(edges[i / 2]), get<0>(edges[i / 2]));
          }
        });
    auto sym_edges = remove_duplicates_ordered(sym_edges_delayed,
                                               less<pair<NodeId, NodeId>>());
    Graph GA;
    GA.n = edgelist.size();
    GA.m = sym_edges.size();
    GA.offset = sequence<EdgeId>(GA.n + 1);
    GA.E = sequence<NodeId>(GA.m);
    parallel_for(0, GA.n + 1, [&](size_t i) { GA.offset[i] = 0; });
    parallel_for(0, GA.m, [&](size_t i) {
      NodeId u = sym_edges[i].first;
      NodeId v = sym_edges[i].second;
      GA.E[i] = v;
      if (i == 0 || sym_edges[i - 1].first != u) {
        GA.offset[u] = i;
      }
      if (i == GA.m - 1 || sym_edges[i + 1].first != u) {
        size_t end = (i == GA.m - 1 ? (GA.n + 1) : sym_edges[i + 1].first);
        parallel_for(u + 1, end, [&](size_t j) { GA.offset[j] = i + 1; });
      }
    });
    auto label = get<0>(connect(GA, beta));
    return label;
  }
  void get_num_bcc(const sequence<NodeId> &label) {
    size_t num_bcc = remove_duplicates_ordered(label).size();
    printf("#BCC: %zu\n", num_bcc);
    std::ofstream ofs("tarjan-vishkin.csv", ios_base::app);
    ofs << num_bcc << '\t';
    ofs.close();
  }
};
