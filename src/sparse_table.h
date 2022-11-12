#pragma once
#include <parlay/monoid.h>
#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>

using namespace std;
using namespace parlay;
constexpr int LOG2_BLOCK = 6;
constexpr size_t ST_BLOCK_SIZE = size_t{1} << LOG2_BLOCK;
constexpr size_t BLOCK_MASK = ST_BLOCK_SIZE - 1;

template <class R, class M>
struct sparse_table {
  size_t n, k;
  M m;
  using O = decltype(m.identity);
  sequence<sequence<O>> table;
  const R &seq;
  sparse_table(R &_seq, M _m) : m(_m), seq(_seq) {
    n = max(size_t{1}, seq.size() / ST_BLOCK_SIZE);
    k = max(size_t{1}, (size_t)ceil(log2(n)));
    table = sequence<sequence<O>>(k, sequence<O>(n));
    parallel_for(0, n, [&](size_t i) {
      O v = m.identity;
      for (size_t offset = 0;
           offset < ST_BLOCK_SIZE && ((i << LOG2_BLOCK) | offset) < seq.size();
           offset++) {
        v = m.f(v, seq[(i << LOG2_BLOCK) | offset]);
      }
      table[0][i] = v;
    });
    for (size_t i = 1; i < k; i++) {
      parallel_for(0, n, [&](size_t j) {
        if (j + (1 << i) <= n) {
          table[i][j] = m.f(table[i - 1][j], table[i - 1][j + (1 << (i - 1))]);
        }
      });
    }
  }
  O query(size_t l, size_t r) {
    size_t block_l = (l >> LOG2_BLOCK) + 1, block_r = r >> LOG2_BLOCK;
    O ret = m.identity;
    if (block_l < block_r) {
      size_t s = 63 - __builtin_clzll(block_r - block_l);
      ret = m.f(ret, m.f(table[s][block_l], table[s][block_r - (1ull << s)]));
      for (size_t i = l; i < min(r, block_l << LOG2_BLOCK); i++) {
        ret = m.f(ret, seq[i]);
      }
      for (size_t i = max(l, block_r << LOG2_BLOCK); i < r; i++) {
        ret = m.f(ret, seq[i]);
      }
    } else {
      for (size_t i = l; i < r; i++) {
        ret = m.f(ret, seq[i]);
      }
    }
    return ret;
  }
};
