#include "BCC.h"

#include <iostream>

#include "graph.h"
#include "hopcroft_tarjan.h"
int NUM_ROUNDS = 10;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " filename\n";
    abort();
  }
  char* filename = argv[1];
  double beta = 2;
  if (argc >= 3) {
    NUM_ROUNDS = atoi(argv[2]);
  }
  if (argc >= 4) {
    beta = atof(argv[3]);
  }
  internal::timer t_g;
  Graph g = read_graph(filename);
  printf("Graph read: %f\n", t_g.total_time());
  double total_time = 0;
  for (int i = 0; i <= NUM_ROUNDS; i++) {
    if (i == 0) {
      BCC solver(g, beta);
      internal::timer t_critical;
      solver.biconnectivity();
      t_critical.stop();
      printf("Warmup round: %f\n", t_critical.total_time());
    } else {
      BCC solver(g, beta);
      internal::timer t_critical;
      solver.biconnectivity();
      t_critical.stop();
      printf("Round %d: %f\n", i, t_critical.total_time());
      total_time += t_critical.total_time();
    }
  }
  printf("Average time: %f\n", total_time / NUM_ROUNDS);
  BCC solver(g);
  auto label = solver.biconnectivity();
  solver.get_num_bcc(label);
  std::ofstream ofs("fast-bcc.tsv", ios_base::app);
  ofs << total_time / NUM_ROUNDS << '\t';
  ofs.close();
  return 0;
}
