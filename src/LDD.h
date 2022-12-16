#pragma once
#include <algorithm>
#include <cmath>
#include <iostream>

#include "graph.h"
#include "hash_bag.h"
#include "parlay/parallel.h"
#include "parlay/random.h"

using namespace std;
// NOTE: BLOCK_SIZE was set to be 1 << 13
constexpr int LOCAL_QUEUE_SIZE = 1024;
constexpr bool local = true;

class LDD {
 private:
  const Graph& G;
  sequence<NodeId> front;
  sequence<bool> in_frontier;
  sequence<bool> in_next_frontier;
  hashbag<NodeId> bag;
  bool sparse_pre;
  size_t frontier_size;
  size_t threshold;
  function<bool(NodeId, NodeId)> pred;
  size_t sparse_update(sequence<NodeId>& label, sequence<NodeId>& parent);
  size_t dense_update(sequence<NodeId>& label, sequence<NodeId>& parent);
  EdgeId dense_sample(NodeId rand_seed);
  void sparse2dense();
  void dense2sparse();
  bool judge();

 public:
  tuple<sequence<NodeId>, sequence<NodeId>> ldd(double beta,
                                                bool spanning_forest);
  size_t num_round;
  LDD() = delete;
  LDD(
      const Graph& _G, function<bool(NodeId, NodeId)> _pred =
                           [](NodeId, NodeId) { return true; })
      : G(_G), bag(G.n), pred(_pred) {
    // front = sequence<NodeId>(G.n);
    front = sequence<NodeId>::uninitialized(G.n);
    // in_frontier = sequence<bool>(G.n);
    in_frontier = sequence<bool>::uninitialized(G.n);
    // in_next_frontier = sequence<bool>(G.n);
    in_next_frontier = sequence<bool>::uninitialized(G.n);
    sparse_pre = true;
    frontier_size = 0;
    threshold = G.m / 20;
    num_round = 0;
#ifdef BREAKDOWN
    num_visited = sequence<EdgeId>(G.n);
#endif
  };
#ifdef BREAKDOWN
  sequence<EdgeId> num_visited;
  internal::timer t_sparse_update;
  internal::timer t_dense_update;
  internal::timer t_sparse2dense;
  internal::timer t_dense2sparse;
  internal::timer t_judge;
  internal::timer t_total_time;

  void reset_internal::timer() {
    t_sparse_update.reset();
    t_dense_update.reset();
    t_sparse2dense.reset();
    t_dense2sparse.reset();
    t_judge.reset();
    t_total_time.reset();
  }

  void output_breakdown() {
#define print_time(name)                                                     \
  printf("%-15s running time: %10f, %10f%%\n", #name, t_##name.total_time(), \
         100. * t_##name.total_time() / t_total_time.total_time());
    print_time(sparse_update);
    print_time(dense_update);
    print_time(sparse2dense);
    print_time(dense2sparse);
    print_time(judge);
  }
#endif
};

size_t LDD::sparse_update(sequence<NodeId>& label, sequence<NodeId>& parent) {
#ifdef BREAKDOWN
  t_sparse_update.start();
  parallel_for(0, frontier_size, [&](size_t i) { num_visited[i] = 0; });
#endif
  parallel_for(
      0, frontier_size,
      [&](size_t i) {
        NodeId f = front[i];
        size_t deg_f = G.offset[f + 1] - G.offset[f];
        if (!local || deg_f > BLOCK_SIZE) {
#ifdef BREAKDOWN
          num_visited[i] += G.offset[f + 1] - G.offset[f];
#endif
          parallel_for(
              G.offset[f], G.offset[f + 1],
              [&](size_t j) {
                NodeId v = G.E[j];
                if (pred(f, v)) {
                  if (compare_and_swap(&label[v], UINT_N_MAX, label[f])) {
                    if (parent.size()) {
                      parent[v] = f;
                    }
                    bag.insert(v);
                  }
                }
              },
              BLOCK_SIZE);
        } else {
          NodeId local_queue[LOCAL_QUEUE_SIZE];
          // sequence<NodeId> local_queue(LOCAL_QUEUE_SIZE);
          size_t head = 0, tail = 0;
          local_queue[tail++] = f;
          while (head < tail && tail != LOCAL_QUEUE_SIZE) {
            NodeId u = local_queue[head++];
            size_t deg_u = G.offset[u + 1] - G.offset[u];
            if (deg_u > BLOCK_SIZE) {
              bag.insert(u);
              // break;
            } else {
#ifdef BREAKDOWN
              num_visited[i] += deg_u;
#endif
              for (size_t j = G.offset[u]; j < G.offset[u + 1]; j++) {
                NodeId v = G.E[j];
                if (pred(u, v)) {
                  if (compare_and_swap(&label[v], UINT_N_MAX, label[u])) {
                    if (parent.size()) {
                      parent[v] = u;
                    }
                    if (tail < LOCAL_QUEUE_SIZE) {
                      local_queue[tail++] = v;
                    } else {
                      bag.insert(v);
                    }
                  }
                }
              }
            }
          }
          for (size_t j = head; j < tail; j++) {
            bag.insert(local_queue[j]);
          }
        }
      },
      1);
  auto ret = bag.pack_into(make_slice(front));
#ifdef BREAKDOWN
  size_t tot_visited = reduce(num_visited.cut(0, frontier_size));
  printf("tot_visited: %zu, ", tot_visited);
  t_sparse_update.stop();
#endif
  return ret;
}

size_t LDD::dense_update(sequence<NodeId>& label, sequence<NodeId>& parent) {
#ifdef BREAKDOWN
  t_dense_update.start();
  parallel_for(0, frontier_size, [&](size_t i) { num_visited[i] = 0; });
#endif
  // parallel_for(0, G.n, [&](size_t i) {
  // in_frontier[i] = false;
  // if(in_frontier[i]) {
  // parallel_for(G.offset[i], G.offset[i + 1], [&](size_t j) {
  // NodeId v = G.E[j];
  // if(pred(i, v)) {
  // if(label[v] == UINT_N_MAX) {
  // if(parent.size()) {
  // parent[v] = i;
  //}
  // label[v] = label[i];
  // in_next_frontier[i] = true;
  //}
  //}
  //});
  //}
  //});
  parallel_for(
      0, G.n,
      [&](NodeId i) {
        in_next_frontier[i] = false;
        if (label[i] == UINT_N_MAX) {
          const auto& offset = G.offset;
          const auto& E = G.E;
#ifdef BREAKDOWN
          size_t deg = offset[i + 1] - offset[i];
          num_visited[i] = deg;
#endif
          // size_t num_block = (deg + BLOCK_SIZE - 1) / BLOCK_SIZE;
          // parallel_for(0, num_block, [&](size_t j) {
          // size_t st = j * num_block, ed = min((j + 1) * num_block,
          // static_cast<size_t>(offset[i + 1]));
          ////if (!in_frontier[i]) {
          // for (size_t k = st; k < ed; k++) {
          // if(in_frontier[i]) {
          // break;
          //}
          // NodeId v = E[k + offset[i]];
          // if (pred(i, v)) {
          // if (in_frontier[v]) {
          // if (parent.size()) {
          // parent[i] = v;
          //}
          // label[i] = label[v];
          // in_next_frontier[i] = true;
          // break;
          //}
          //}
          //}
          ////}
          //});
          for (size_t j = offset[i]; j < offset[i + 1]; j++) {
            NodeId v = E[j];
            if (pred(i, v)) {
              if (in_frontier[v]) {
                if (parent.size()) {
                  parent[i] = v;
                }
                label[i] = label[v];
                in_frontier[i] = in_next_frontier[i] = true;
#ifdef BREAKDOWN
                num_visited[i] = j - offset[i] + 1;
#endif
                break;
              }
            }
          }
        }
      },
      BLOCK_SIZE);

  swap(in_frontier, in_next_frontier);
  auto ret = count(in_frontier, true);
#ifdef BREAKDOWN
  size_t tot_visited = reduce(num_visited);
  printf("tot_visited: %zu, ", tot_visited);
  t_dense_update.stop();
#endif
  return ret;
}

EdgeId LDD::dense_sample(NodeId rand_seed) {
  constexpr int NUM_SAMPLES = 50;
  NodeId count = 0;
  EdgeId out_edges = 0;
  int i = 0;
  while (count < NUM_SAMPLES) {
    i++;
    NodeId index = hash32(rand_seed + i) % G.n;
    if (in_frontier[index]) {
      count++;
      out_edges += G.offset[index + 1] - G.offset[index];
    }
  }
  return frontier_size * (out_edges / count);
}

void LDD::sparse2dense() {
#ifdef BREAKDOWN
  t_sparse2dense.start();
#endif
  parallel_for(0, G.n, [&](size_t i) { in_frontier[i] = false; });
  parallel_for(0, frontier_size,
               [&](size_t i) { in_frontier[front[i]] = true; });
#ifdef BREAKDOWN
  t_sparse2dense.stop();
#endif
}

void LDD::dense2sparse() {
#ifdef BREAKDOWN
  t_dense2sparse.start();
#endif
  auto identity = delayed_seq<NodeId>(G.n, [&](size_t i) { return i; });
  pack_into_uninitialized(identity, in_frontier, front);
#ifdef BREAKDOWN
  t_dense2sparse.stop();
#endif
}

bool LDD::judge() {
#ifdef BREAKDOWN
  t_judge.start();
#endif
  size_t front_out_edges = 0;
  bool both_sparse_dense = false;
  if (!sparse_pre && frontier_size < (1 << 14)) {
    dense2sparse();
    both_sparse_dense = true;
  }
  if (sparse_pre || both_sparse_dense) {
    auto degree_seq = delayed_seq<size_t>(frontier_size, [&](size_t i) {
      NodeId u = front[i];
      return G.offset[u + 1] - G.offset[u];
    });
    front_out_edges = reduce(degree_seq);
  } else {
    front_out_edges = dense_sample(hash32(num_round));
  }
#if defined(BREAKDOWN)
  // cout << "frontier_size: " << frontier_size << ", front_out_edges: "
  //<< front_out_edges << endl;
#endif
  bool sparse_now = ((frontier_size + front_out_edges) < threshold);
  if (!both_sparse_dense && sparse_pre != sparse_now) {
    if (!sparse_now) {
      sparse2dense();
    } else {
      dense2sparse();
    }
  }
#ifdef BREAKDOWN
  t_judge.stop();
#endif
  return sparse_now;
}

tuple<sequence<NodeId>, sequence<NodeId>> LDD::ldd(double beta,
                                                   bool spanning_tree = false) {
#ifdef BREAKDOWN
  reset_internal::timer();
  t_total_time.start();
#endif
  size_t n = G.n;
  sequence<NodeId> label(n, UINT_N_MAX);
  sequence<NodeId> parent;
  if (spanning_tree) {
    parent = tabulate(n, [&](NodeId i) { return i; });
  }
  internal::timer t0;
  t0.start();
  // auto perm = parlay::random_permutation<NodeId>(n);
  // t0.stop();
  // cout << "perm time " << t0.total_time()<<endl;
  // BEGIN SAMPLE
  size_t sample_number = 1024;
  // sequence<NodeId> perm = sequence<NodeId>(sample_number);
  auto perm = sequence<NodeId>::uninitialized(sample_number);
  for (size_t i = 0; i < sample_number; i++) {
    perm[i] = hash32(sample_number + i) % n;
  }
  t0.stop();
#if defined(BREAKDOWN)
  cout << "sample time " << t0.total_time() << endl;
#endif
  // size_t num_added = 0;
  size_t num_sampled = 0;
  num_round = 0;
  frontier_size = 0;
  sparse_pre = true;
  // NOTE: num_added  < 0.8*n ?
  //
  internal::timer t_round;
  while (frontier_size > 0 || num_sampled < sample_number) {
    t_round.start();
    num_round++;
    size_t step_size = floor(exp(num_round * beta));
    size_t work_size = min(step_size, sample_number - num_sampled);
    size_t num_new_centers = 0;
    if (sparse_pre && work_size > 0) {
      auto centers = filter(perm.cut(num_sampled, num_sampled + work_size),
                            [&](NodeId u) { return label[u] == UINT_N_MAX; });
      num_new_centers = centers.size();
      parallel_for(0, num_new_centers, [&](size_t i) {
        front[frontier_size + i] = centers[i];
        label[centers[i]] = centers[i];
      });
    } else if (work_size > 0) {
      num_new_centers =
          count(perm.cut(num_sampled, num_sampled + work_size), UINT_N_MAX);
      parallel_for(num_sampled, num_sampled + work_size, [&](NodeId i) {
        NodeId u = perm[i];
        if (label[u] == UINT_N_MAX) {
          label[u] = u;
          in_frontier[u] = true;
        }
      });
    }
    frontier_size += num_new_centers;
    num_sampled += work_size;
    // step_size = ceil(step_size * beta);
    bool sparse_now = judge();
#if defined(BREAKDOWN)
    cout << "Round " << num_round << ": n_front: " << frontier_size << " ";
    if (sparse_now)
      cout << "sparse, ";
    else
      cout << "dense, ";
#endif
    if (sparse_now) {
      frontier_size = sparse_update(label, parent);
    } else {
      frontier_size = dense_update(label, parent);
    }
    sparse_pre = sparse_now;
#if defined(BREAKDOWN)
    cout << "round_time " << t_round.stop() << endl;
#endif
  }
  parallel_for(0, G.n, [&](size_t i) {
    if (label[i] == UINT_N_MAX) {
      label[i] = i;
    }
  });
#ifdef BREAKDOWN
  t_total_time.stop();
  printf("total_time running time: %10f\n", t_total_time.total_time());
  output_breakdown();
#endif
  return {label, parent};
}
