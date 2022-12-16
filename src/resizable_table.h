// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2010-2016 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <tuple>
#include <unordered_set>
#include <vector>

#include "parlay/primitives.h"
#include "parlay/sequence.h"
#include "utilities.h"

using namespace parlay;

namespace gbbs {
// TODO: see if striding by an entire page improves times further.
constexpr size_t kResizableTableCacheLineSz = 128;

inline size_t hashToRange(const size_t& h, const size_t& mask) {
  return h & mask;
}
inline size_t incrementIndex(const size_t& h, const size_t& mask) {
  return hashToRange(h + 1, mask);
}

template <class K, class KeyHash>
class resizable_table {
 public:
  bool table_full;
  size_t m;
  size_t mask;
  size_t ne;
  K empty;
  sequence<K> table;
  KeyHash key_hash;
  sequence<size_t> cts;

  inline size_t firstIndex(K& k) { return hashToRange(key_hash(k), mask); }

  void init_counts() {
    size_t workers = num_workers();
    cts = sequence<size_t>::uninitialized(kResizableTableCacheLineSz * workers);
    for (size_t i = 0; i < workers; i++) {
      cts[i * kResizableTableCacheLineSz] = 0;
    }
  }

  void update_nelms() {
    size_t workers = num_workers();
    for (size_t i = 0; i < workers; i++) {
      ne += cts[i * kResizableTableCacheLineSz];
      cts[i * kResizableTableCacheLineSz] = 0;
    }
  }

  resizable_table() : table_full(false), m(0), ne(0) {
    mask = 0;
    init_counts();
  }

  resizable_table(size_t _m, K _empty, KeyHash _key_hash)
      // : m((size_t)1 << parlay::log2_up((size_t)(1.1 * _m))),
      : table_full(false),
        m((size_t)1 << parlay::log2_up((size_t)(_m * 1.1))),
        mask(m - 1),
        ne(0),
        empty(_empty),
        key_hash(_key_hash) {
    table = sequence<K>::uninitialized(m);
    clear();
    init_counts();
  }

  bool insert(K k) {
    size_t h = firstIndex(k);
    for (size_t i = 0; i < 0.9 * m; i++) {
      if (table_full) return false;
      if (table[h] == empty && atomic_compare_and_swap(&table[h], empty, k)) {
        size_t wn = worker_id();
        cts[wn * kResizableTableCacheLineSz]++;
        return 1;
      }
      h = incrementIndex(h, mask);
    }
    std::cout << "table full" << std::endl;
    table_full = true;
    return 0;
  }

  void clear() {
    parallel_for(0, m, [&](size_t i) { table[i] = empty; });
  }

  sequence<K> entries() {
    auto pred = [&](K& t) { return t != empty; };
    return filter(make_slice(table), pred);
  }
};

template <class K, class KeyHash>
inline resizable_table<K, KeyHash> make_resizable_table(
    size_t m, K empty, KeyHash key_hash) {
  return resizable_table<K, KeyHash>(m, empty, key_hash);
}

}  // namespace gbbs
