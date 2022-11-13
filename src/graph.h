#pragma once

#include <fcntl.h>
#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/random.h>
#include <parlay/sequence.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <vector>

using namespace std;
using namespace parlay;

#define ELONG

#ifdef NLONG
using NodeId = uint64_t;
#else
using NodeId = uint32_t;
#endif

#ifdef ELONG
using EdgeId = uint64_t;
#else
using EdgeId = uint32_t;
#endif

constexpr NodeId UINT_N_MAX = numeric_limits<NodeId>::max();
constexpr EdgeId UINT_E_MAX = numeric_limits<EdgeId>::max();

struct edge {
  NodeId u;
  NodeId v;
  edge() : u(0), v(0) {}
  edge(NodeId f, NodeId s) : u(f), v(s) {}
  bool operator<(const edge& rhs) const {
    if (u != rhs.u) {
      return u < rhs.u;
    } else {
      return v < rhs.v;
    }
  }
  bool operator!=(const edge& rhs) const { return u != rhs.u || v != rhs.v; }
};

struct Graph {
  size_t n;
  size_t m;
  bool symmetric;
  sequence<EdgeId> offset;
  sequence<NodeId> E;
};

// For spanning forest
struct Graph2 {
  size_t n;
  size_t m;
  sequence<NodeId> offset;
  sequence<NodeId> E;
};

struct Forest {
  size_t num_trees;
  Graph2 G;
  sequence<NodeId> vertex;
  sequence<NodeId> offset;
};

bool is_space(char c) {
  switch (c) {
    case '\r':
    case '\t':
    case '\n':
    case ' ':
    case 0:
      return true;
    default:
      return false;
  }
}

Graph read_pbbs(const char* const filename) {
  ifstream file(filename, ifstream::in | ifstream::binary | ifstream::ate);
  if (!file.is_open()) {
    cerr << "Error: Cannot open file " << filename << '\n';
    abort();
  }
  long end = file.tellg();
  file.seekg(0, file.beg);
  long length = end - file.tellg();

  sequence<char> strings(length + 1);
  file.read(strings.begin(), length);
  file.close();

  sequence<bool> flag(length);
  parallel_for(0, length, [&](size_t i) {
    if (is_space(strings[i])) {
      strings[i] = 0;
    }
  });
  flag[0] = strings[0];
  parallel_for(1, length, [&](size_t i) {
    if (is_space(strings[i - 1]) && !is_space(strings[i])) {
      flag[i] = true;
    } else {
      flag[i] = false;
    }
  });

  auto offset_seq = pack_index(flag);
  sequence<char*> words(offset_seq.size());
  parallel_for(0, offset_seq.size(),
               [&](size_t i) { words[i] = strings.begin() + offset_seq[i]; });

  Graph graph;
  size_t n = atol(words[1]);
  size_t m = atol(words[2]);
  graph.n = n;
  graph.m = m;
  graph.offset = sequence<EdgeId>(n + 1);
  graph.E = sequence<NodeId>(m);
  parallel_for(0, n, [&](size_t i) { graph.offset[i] = atol(words[i + 3]); });
  graph.offset[n] = m;
  parallel_for(0, m, [&](size_t i) { graph.E[i] = atol(words[n + i + 3]); });
  return graph;
}

Graph read_binary(const char* const filename, bool enable_mmap = true) {
  Graph graph;
  if (enable_mmap) {
    struct stat sb;
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
      cerr << "Error: Cannot open file " << filename << '\n';
      abort();
    }
    if (fstat(fd, &sb) == -1) {
      cerr << "Error: Unable to acquire file stat" << filename << '\n';
      abort();
    }
    void* memory = mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    char* data = static_cast<char*>(memory);
    size_t len = sb.st_size;
    size_t n = reinterpret_cast<uint64_t*>(data)[0];
    size_t m = reinterpret_cast<uint64_t*>(data)[1];
    size_t sizes = reinterpret_cast<uint64_t*>(data)[2];
    assert(sizes == (n + 1) * 8 + m * 4 + 3 * 8);
    graph.n = n, graph.m = m;
    graph.offset = sequence<EdgeId>::uninitialized(n + 1);
    graph.E = sequence<NodeId>::uninitialized(m);
    parallel_for(0, n + 1, [&](size_t i) {
      graph.offset[i] = reinterpret_cast<uint64_t*>(data + 3 * 8)[i];
    });
    parallel_for(0, m, [&](size_t i) {
      graph.E[i] = reinterpret_cast<uint32_t*>(data + 3 * 8 + (n + 1) * 8)[i];
    });

    if (memory != MAP_FAILED) {
      if (munmap(memory, len) != 0) {
        cerr << "Error: munmap failed" << '\n';
        abort();
      }
    }
  } else {
    ifstream ifs(filename);
    if (!ifs.is_open()) {
      cerr << "Error: Cannot open file " << filename << '\n';
      abort();
    }
    size_t n, m, sizes;
    ifs.read(reinterpret_cast<char*>(&n), sizeof(size_t));
    ifs.read(reinterpret_cast<char*>(&m), sizeof(size_t));
    ifs.read(reinterpret_cast<char*>(&sizes), sizeof(size_t));
    assert(sizes == (n + 1) * 8 + m * 4 + 3 * 8);

    graph.n = n;
    graph.m = m;
    sequence<uint64_t> offset(n + 1);
    sequence<uint32_t> edge(m);
    ifs.read(reinterpret_cast<char*>(offset.begin()), (n + 1) * 8);
    ifs.read(reinterpret_cast<char*>(edge.begin()), m * 4);
    graph.offset = sequence<EdgeId>(n + 1);
    graph.E = sequence<NodeId>(m);
    parallel_for(0, n + 1, [&](size_t i) { graph.offset[i] = offset[i]; });
    parallel_for(0, m, [&](size_t i) { graph.E[i] = edge[i]; });
    if (ifs.peek() != EOF) {
      cerr << "Error: Bad data\n";
      abort();
    }
    ifs.close();
  }
  return graph;
}

void make_symmetric(Graph& graph) {
  sequence<edge> edge_list(2 * graph.m);
  parallel_for(0, graph.n, [&](long i) {
    parallel_for(graph.offset[i], graph.offset[i + 1], [&](long j) {
      edge_list[j * 2] = edge(i, graph.E[j]);
      edge_list[j * 2 + 1] = edge(graph.E[j], i);
    });
  });
  sort_inplace(edge_list);
  auto unique_id = delayed_seq<bool>(edge_list.size(), [&](size_t i) {
    return (i == 0 || edge_list[i] != edge_list[i - 1]) &&
           edge_list[i].u != edge_list[i].v;
  });
  auto unique_edges = pack(edge_list, unique_id);
  edge_list.clear();

  graph.m = unique_edges.size();
  graph.E = sequence<NodeId>(graph.m);
  parallel_for(0, graph.n + 1, [&](size_t i) { graph.offset[i] = 0; });
  parallel_for(0, graph.m, [&](size_t i) {
    NodeId u = unique_edges[i].u;
    NodeId v = unique_edges[i].v;
    graph.E[i] = v;
    if (i == 0 || unique_edges[i - 1].u != u) {
      graph.offset[u] = i;
    }
    if (i == graph.m - 1 || unique_edges[i + 1].u != u) {
      size_t end = (i == graph.m - 1 ? (graph.n + 1) : unique_edges[i + 1].u);
      parallel_for(u + 1, end, [&](size_t j) { graph.offset[j] = i + 1; });
    }
  });
}

Graph generate_synthetic_graph(size_t n, size_t k) {
  Graph graph;
  graph.n = n;
  graph.m = n * k;
  printf("n=%zu, m=%zu\n", graph.n, graph.m);
  graph.symmetric = true;
  graph.offset = sequence<EdgeId>(graph.n + 1);
  graph.E = sequence<NodeId>(graph.m);
  auto perm = random_permutation(graph.n);
  parallel_for(0, n + 1, [&](size_t i) { graph.offset[i] = i * k; });
  parallel_for(0, n, [&](size_t i) {
    NodeId u = perm[i];
    assert(graph.offset[u + 1] - graph.offset[u] == k);
    for (size_t j = graph.offset[u]; j < graph.offset[u + 1]; j++) {
      graph.E[j] = perm[(i + 1 + j - graph.offset[u]) % n];
    }
  });
  return graph;
}

Graph generate_synthetic_cycle(size_t n) {
  Graph graph;
  graph.n = n;
  graph.m = n * 2;
  printf("n=%zu, m=%zu\n", graph.n, graph.m);
  graph.symmetric = true;
  graph.offset = sequence<EdgeId>(graph.n + 1);
  graph.E = sequence<NodeId>(graph.m);
  auto perm = random_permutation(graph.n);
  parallel_for(0, n + 1, [&](size_t i) { graph.offset[i] = i * 2; });
  parallel_for(0, n, [&](size_t i) {
    NodeId u = perm[i];
    graph.E[graph.offset[u]] = perm[(i - 1 + n) % n];
    graph.E[graph.offset[u] + 1] = perm[(i + 1) % n];
  });
  return graph;
}

Graph read_graph(char* filename, bool enable_mmap = true,
                 bool symmetric = false) {
  if (strcmp(filename, "synthetic") == 0) {
    size_t n = 3563602789, k = 63;
    return generate_synthetic_graph(n, k);
  }
  if (strcmp(filename, "cycle") == 0) {
    size_t n = 1e9;
    return generate_synthetic_cycle(n);
  }
  // cout << "enable_mmap " << enable_mmap << endl;
  // cout << "symmetric " << symmetric << endl;
  string str_filename = string(filename);
  size_t idx = str_filename.find_last_of('.');
  if (idx == string::npos) {
    cerr << "Error: No graph extension provided\n";
    abort();
  }
  string subfix = str_filename.substr(idx + 1);
  Graph graph;
  if (subfix == "adj") {
    graph = read_pbbs(filename);
  } else if (subfix == "bin") {
    graph = read_binary(filename, enable_mmap);
  } else {
    cerr << "Error: Invalid graph extension\n";
  }
  if (str_filename.find("sym") != string::npos) {
    graph.symmetric = true;
  } else if (symmetric) {
    make_symmetric(graph);
    graph.symmetric = true;
  } else {
    cerr << "Please pass a symmetri graph (contains sym in the filename) or "
            "symmetrize the graph (pass -s flag)"
         << '\n';
    abort();
  }
  return graph;
}

Graph read_large_sym_graph(char* filename) {
  ifstream ifs(filename);
  if (!ifs.is_open()) {
    cerr << "Error: Cannot open file " << filename << '\n';
    abort();
  }
  size_t n, m, sizes;
  ifs.read(reinterpret_cast<char*>(&n), sizeof(size_t));
  ifs.read(reinterpret_cast<char*>(&m), sizeof(size_t));
  ifs.read(reinterpret_cast<char*>(&sizes), sizeof(size_t));
  assert(sizes == (n + 1) * 8 + m * 4 + 3 * 8);

  Graph graph;
  graph.n = n;
  graph.m = m;
  graph.offset = sequence<EdgeId>(n + 1);
  graph.E = sequence<NodeId>(m);
  ifs.read(reinterpret_cast<char*>(graph.offset.begin()), (n + 1) * 8);
  ifs.read(reinterpret_cast<char*>(graph.E.begin()), m * 4);
  graph.symmetric = true;

  if (ifs.peek() != EOF) {
    cerr << "Error: Bad data\n";
    abort();
  }
  ifs.close();
  return graph;
}
