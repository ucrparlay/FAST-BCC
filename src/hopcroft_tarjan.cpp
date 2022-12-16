#include "hopcroft_tarjan.h"

#include <iostream>

#include "parlay/internal/get_time.h"
int NUM_ROUNDS = 10;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " filename\n";
    abort();
  }
  char* filename = argv[1];
  if (argc == 3) {
    NUM_ROUNDS = atoi(argv[2]);
  }
  Graph g = read_graph(filename);
  printf("n: %zu\n", g.n);
  double total_time = 0;
  for (int i = 0; i <= NUM_ROUNDS; i++) {
    Tarjan solver(g);
    if (i == 0) {
      internal::timer t;
      solver.bcc();
      t.stop();
      printf("#BCC: %u\n", solver.k);
      std::ofstream ofs("hopcroft-tarjan.tsv", ios_base::app);
      ofs << solver.k << '\t';
      ofs.close();
      printf("Warmup round: %f\n", t.total_time());
    } else {
      internal::timer t;
      solver.bcc();
      t.stop();
      printf("Round %d: %f\n", i, t.total_time());
      total_time += t.total_time();
    }
  }
  printf("Average time: %f\n", total_time / NUM_ROUNDS);
  std::ofstream ofs("hopcroft-tarjan.tsv", ios_base::app);
  ofs << total_time / NUM_ROUNDS << '\n';
  ofs.close();
  return 0;
}
