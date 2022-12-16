#pragma once
#include <queue>

#include "connectivity.h"
#include "graph.h"
#include "spanning_forest.h"
#include "sparse_table.h"
#include "utilities.h"
using namespace std;
using namespace parlay;

//#define N2LONG

#ifdef N2LONG
using NodeId2 = uint64_t;
#else
using NodeId2 = uint32_t;
#endif
constexpr NodeId2 UINT_N2_MAX = numeric_limits<NodeId2>::max();

struct BCC {
  const Graph &G;
  Forest F;
  double beta;
  sequence<NodeId> parent;
  sequence<NodeId> order;
  sequence<NodeId2> first;
  sequence<NodeId2> last;
  sequence<NodeId2> low;
  sequence<NodeId2> high;
  BCC() = delete;
  BCC(const Graph &_G, double _beta = 0.2) : G(_G), beta(_beta) {}

  void euler_tour_tree() {
    internal::timer t_ett;
    ;
    size_t num_trees = F.num_trees;
    size_t m = F.G.m;
    auto edgelist = sequence<pair<NodeId, NodeId>>::uninitialized(m * 2);
    auto perms = sequence<pair<NodeId, NodeId2>>::uninitialized(m * 2);
    assert(F.G.m == F.G.offset[F.G.n]);
    parallel_for(0, F.G.n, [&](size_t i) {
      parallel_for(
          F.G.offset[i], F.G.offset[i + 1],
          [&](size_t j) {
            edgelist[j * 2] = {F.vertex[i], F.G.E[j]};
            edgelist[j * 2 + 1] = {F.G.E[j], F.vertex[i]};
            perms[j * 2] = {F.vertex[i], j * 2};
            perms[j * 2 + 1] = {F.G.E[j], j * 2 + 1};
          },
          BLOCK_SIZE);
    });
    // printf("Initialization time: %f\n", t_init.total_time());
    // printf("perms.size(): %zu\n", perms.size());

    // auto perms = tabulate(
    // m * 2, [&](NodeId2 i) { return make_pair(edgelist[i].first, i); });
    // auto delayed_perms = delayed_seq<pair<NodeId2, NodeId2>>(
    // m * 2, [&](NodeId2 i) { return make_pair(edgelist[i].first, i); });
    // t_ett.next("initialization");
    integer_sort_inplace(make_slice(perms), [](const pair<NodeId, NodeId2> &a) {
      return a.first;
    });
    // sort_inplace(make_slice(perms), [](const pair<NodeId, NodeId2> &a,
    // const pair<NodeId, NodeId2> &b) {
    // return a.first < b.first;
    //});
    // auto perms = integer_sort(delayed_perms,
    //[](pair<NodeId2, NodeId2> a) { return a.first; });
    // printf("Sorting time: %f\n", t_sort.total_time());
    // t_ett.next("sorting");

    auto first_edge = sequence<NodeId2>::uninitialized(G.n);
    parallel_for(0, m * 2, [&](size_t i) {
      if (i == 0 || perms[i - 1].first != perms[i].first) {
        first_edge[perms[i].first] = perms[i].second;
      }
    });
    auto link = sequence<NodeId2>::uninitialized(m * 2);
    parallel_for(0, m * 2, [&](size_t i) {
      if (i + 1 < m * 2 && perms[i].first == perms[i + 1].first) {
        link[perms[i].second ^ 1] = perms[i + 1].second;
      } else {
        link[perms[i].second ^ 1] = first_edge[perms[i].first];
      }
    });
    // t_ett.next("linking");
    // printf("Linking time: %f\n", t_linking.total_time());

    auto samples_offset = sequence<NodeId2>::uninitialized(num_trees + 1);
    parallel_for(0, num_trees, [&](size_t i) {
      size_t tree_size =
          (F.G.offset[F.offset[i + 1]] - F.G.offset[F.offset[i]]) + 1;
      size_t edges_size = 2 * (tree_size - 1);
      samples_offset[i] = sqrt(edges_size);
    });
    samples_offset[num_trees] = 0;
    size_t num_samples = scan_inplace(make_slice(samples_offset));
    auto samples = sequence<NodeId2>::uninitialized(num_samples);
    auto skip_to = sequence<pair<NodeId2, NodeId2>>::uninitialized(num_samples);
    sequence<NodeId2> idx(m * 2, UINT_N2_MAX);
    parallel_for(0, num_trees, [&](size_t i) {
      size_t tree_size =
          (F.G.offset[F.offset[i + 1]] - F.G.offset[F.offset[i]]) + 1;
      size_t edges_size = 2 * (tree_size - 1);
      for (size_t j = samples_offset[i]; j < samples_offset[i + 1]; j++) {
        if (j == samples_offset[i]) {
          samples[j] = 2 * (NodeId2)F.G.offset[F.offset[i]];
        } else {
          uint64_t seed = hash64(j);
          NodeId2 pos = seed % edges_size;
          while (idx[pos + 2 * (NodeId2)F.G.offset[F.offset[i]]] !=
                 UINT_N2_MAX) {
            pos = (pos + 1) % edges_size;
          }
          samples[j] = pos + 2 * (NodeId2)F.G.offset[F.offset[i]];
        }
        idx[samples[j]] = j;
      }
    });
    // t_ett.next("sampling");

    internal::timer t_ranking;
    parallel_for(0, num_trees, [&](size_t i) {
      parallel_for(samples_offset[i], samples_offset[i + 1], [&](size_t j) {
        NodeId2 node = samples[j];
        skip_to[j].second = 0;
        do {
          node = link[node];
          skip_to[j].second++;
        } while (idx[node] == UINT_N2_MAX);
        skip_to[j].first = idx[node];
      });
    });

    auto offset = sequence<NodeId2>::uninitialized(num_samples);
    parallel_for(0, num_trees, [&](size_t i) {
      size_t sum = 0;
      NodeId2 cur_idx = samples_offset[i];
      for (size_t j = samples_offset[i]; j < samples_offset[i + 1]; j++) {
        offset[cur_idx] = sum;
        sum += skip_to[cur_idx].second;
        cur_idx = skip_to[cur_idx].first;
      }
    });

    auto sizes = sequence<NodeId2>::uninitialized(num_trees + 1);
    parallel_for(0, num_trees, [&](size_t i) {
      sizes[i] =
          2 * (NodeId2)(F.G.offset[F.offset[i + 1]] - F.G.offset[F.offset[i]]) +
          1;
    });
    sizes[num_trees] = 0;
    scan_inplace(make_slice(sizes));
    // printf("Initializing order\n");
    order = sequence<NodeId>::uninitialized(sizes[num_trees]);

    parallel_for(0, num_trees, [&](size_t i) {
      parallel_for(samples_offset[i], samples_offset[i + 1], [&](size_t j) {
        NodeId2 node = samples[j];
        NodeId2 id = idx[node];
        NodeId2 cur_offset = offset[id];
        do {
          order[sizes[i] + cur_offset] = edgelist[node].first;
          cur_offset++;
          node = link[node];
        } while (idx[node] == UINT_N2_MAX);
      });
      if (samples_offset[i] != samples_offset[i + 1]) {
        order[sizes[i + 1] - 1] = edgelist[samples[samples_offset[i]]].first;
      } else {
        order[sizes[i + 1] - 1] = F.vertex[F.offset[i]];
      }
    });
    // t_ett.next("ranking");
  }
  void compute() {
    internal::timer t;
    size_t n = G.n;
    internal::timer t_first;
    first = sequence<NodeId2>(n, UINT_N2_MAX);
    last = sequence<NodeId2>(n, 0);
    parallel_for(0, order.size(), [&](NodeId2 i) {
      NodeId v = order[i];
      write_min(&first[v], i, less<NodeId2>());
      write_max(&last[v], i, less<NodeId2>());
    });
    // printf("first/last time: %f\n", t_first.total_time());
    parent = tabulate(n, [&](NodeId i) { return i; });
    parallel_for(0, F.G.n, [&](size_t i) {
      NodeId u = F.vertex[i];
      parallel_for(
          F.G.offset[i], F.G.offset[i + 1],
          [&](size_t j) {
            NodeId v = F.G.E[j];
            if (first[u] < first[v]) {
              parent[v] = u;
            } else {
              parent[u] = v;
            }
          },
          BLOCK_SIZE);
    });

    internal::timer t_w, t_copy;
    auto w = tabulate(first.size(),
                      [&](size_t i) { return make_pair(first[i], first[i]); });
    // printf("copy first time: %f\n", t_copy.total_time());
    parallel_for(0, n, [&](size_t i) {
      parallel_for(
          G.offset[i], G.offset[i + 1],
          [&](size_t j) {
            NodeId u = i, v = G.E[j];
            if (u < v && parent[u] != v && parent[v] != u) {
              if (first[u] < first[v]) {
                write_min(&w[v].first, first[u], less<NodeId2>());
                write_max(&w[u].second, first[v], less<NodeId2>());
              } else {
                write_min(&w[u].first, first[v], less<NodeId2>());
                write_max(&w[v].second, first[u], less<NodeId2>());
              }
            }
          },
          BLOCK_SIZE);
    });
    // printf("w time: %f\n", t_w.total_time());

    internal::timer t_st;
    auto seq = tabulate(order.size(), [&](size_t i) { return w[order[i]]; });
    sparse_table st(seq, minmaxm<NodeId2>());

    low = sequence<NodeId2>::uninitialized(n);
    high = sequence<NodeId2>::uninitialized(n);
    parallel_for(0, n, [&](size_t i) {
      std::tie(low[i], high[i]) = st.query(first[i], last[i] + 1);
    });
  }
  auto biconnectivity() {
    internal::timer t;
    F = spanning_forest(G, beta);
    // t.next();
    euler_tour_tree();
    // t.next();
    compute();
    // t.next();
    auto critical = [&](NodeId u, NodeId v) {
      if (first[u] <= low[v] && last[u] >= high[v]) {
        return true;
      }
      return false;
    };
    auto backward = [&](NodeId u, NodeId v) {
      return first[u] <= first[v] && last[u] >= first[v];
    };
    auto pred = [&](NodeId u, NodeId v) {
      if (parent[v] == u) {
        if (!critical(u, v)) {
          return true;
        }
      } else if (parent[u] == v) {
        if (!critical(v, u)) {
          return true;
        }
      } else {
        if (!backward(u, v) && !backward(v, u)) {
          return true;
        }
      }
      return false;
    };
    auto label = get<0>(connect(G, beta, pred));
    get_component_head(label);
    // t.next();
    return label;
  }
  void get_component_head(const sequence<NodeId> &label) {
    internal::timer t;
    size_t n = G.n;
    auto component_head = sequence<NodeId>::uninitialized(n);
    parallel_for(0, n, [&](NodeId i) {
      NodeId p = parent[i];
      if (label[p] != label[i]) {
        component_head[label[i]] = p;
      }
    });
  }
  auto get_articulation_point(const sequence<NodeId> &label) {
    internal::timer t;
    size_t n = G.n;
    auto component_head = sequence<NodeId>::uninitialized(n);
    sequence<bool> articulation_point(n, false);
    sequence<NodeId> component_label(n, UINT_N_MAX);
    parallel_for(0, n, [&](NodeId i) {
      NodeId p = parent[i];
      if (label[p] != label[i]) {
        component_head[label[i]] = p;
        if (!articulation_point[p]) {
          if (parent[p] != p) {
            articulation_point[p] = true;
          } else {
            while (true) {
              if (component_label[p] == UINT_N_MAX) {
                compare_and_swap(&component_label[p], UINT_N_MAX, label[i]);
              } else if (component_label[p] == label[i]) {
                break;
              } else {
                articulation_point[p] = true;
                break;
              }
            }
          }
        }
      }
    });
    t.next("articulation_point");
    auto ret = pack_index<NodeId>(articulation_point);
    return ret;
  }
  void get_num_bcc(const sequence<NodeId> &label) {
    auto cc_label = get<0>(connect(G, beta));
    auto unique_cc_label = remove_duplicates_ordered(cc_label, less<NodeId>());
    auto unique_bcc_label = remove_duplicates_ordered(label, less<NodeId>());
    auto sorted_label = sort(label);
    size_t n = sorted_label.size();
    auto unique_index = pack_index(delayed_seq<bool>(n + 1, [&](size_t i) {
      return i == 0 || i == n || sorted_label[i] != sorted_label[i - 1];
    }));
    auto largest_bcc = reduce(
        delayed_seq<size_t>(
            unique_index.size() - 1,
            [&](size_t i) { return unique_index[i + 1] - unique_index[i]; }),
        maxm<size_t>());
    ofstream ofs("fast-bcc.tsv", ios_base::app);
    ofs << unique_cc_label.size() << '\t'
        << unique_bcc_label.size() - unique_cc_label.size() << '\t'
        << largest_bcc << '\t';
    ofs.close();
    printf("#CC: %zu\n", unique_cc_label.size());
    printf("#BCC: %zu\n", unique_bcc_label.size() - unique_cc_label.size());
    printf("Largest_BCC: %zu\n", largest_bcc);
  }
};
