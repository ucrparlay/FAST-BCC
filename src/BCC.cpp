#include "BCC.h"

#include <iostream>

#include "graph.h"
#include "hopcroft_tarjan.h"
int NUM_ROUNDS = 3;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " filename\n";
    abort();
  }
  char* filename = argv[1];
  if (argc >= 3) {
    NUM_ROUNDS = atoi(argv[2]);
  }
  Graph g = read_graph(filename);
  double total_time = 0;
  for (int i = 0; i <= NUM_ROUNDS; i++) {
    if (i == 0) {
      BCC solver(g);
      internal::timer t_critical;
      solver.biconnectivity();
      t_critical.stop();
      printf("Warmup round: %f\n", t_critical.total_time());
    } else {
      BCC solver(g);
      internal::timer t_critical;
      solver.biconnectivity();
      t_critical.stop();
      printf("Round %d: %f\n", i, t_critical.total_time());
      total_time += t_critical.total_time();
    }
  }
  BCC solver(g);
  auto label = solver.biconnectivity();
  solver.get_num_bcc(label);
  printf("Average time: %f\n", total_time / NUM_ROUNDS);
  std::ofstream ofs("fast-bcc.csv", ios_base::app);
  ofs << total_time / NUM_ROUNDS << '\n';
  ofs.close();
  return 0;
}
