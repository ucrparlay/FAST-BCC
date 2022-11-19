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
  ofstream ofs("hopcroft-tarjan.csv", ios_base::app);
  ofs << filename << ',';
  ofs.close();
  Graph g = read_graph(filename);
  printf("n: %zu\n", g.n);
  double total_time = 0;
  for (int i = 0; i <= NUM_ROUNDS; i++) {
    Tarjan solver(g);
    if (i == 0) {
      internal::timer t;
      solver.bcc();
      t.stop();
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
  Tarjan solver(g);
  solver.bcc();
  printf("#BCC: %u\n", solver.k);
  ofs.open("hopcroft-tarjan.csv", ios_base::app);
  ofs << solver.k << ',' << total_time / NUM_ROUNDS << '\n';
  ofs.close();
  return 0;
}
