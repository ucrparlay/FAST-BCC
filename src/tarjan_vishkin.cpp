#include "tarjan_vishkin.h"

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
  double total_time = 0;
  for (int i = 0; i <= NUM_ROUNDS; i++) {
    if (i == 0) {
      Tarjan_Vishkin solver(g);
      internal::timer t;
      solver.biconnectivity();
      t.stop();
      printf("Warmup round: %f\n", t.total_time());
    } else {
      Tarjan_Vishkin solver(g);
      internal::timer t;
      solver.biconnectivity();
      t.stop();
      printf("Round %d: %f\n", i, t.total_time());
      total_time += t.total_time();
    }
  }
  Tarjan_Vishkin solver(g);
  auto label = solver.biconnectivity();
  solver.get_num_bcc(label);
  printf("Average time: %f\n", total_time / NUM_ROUNDS);
  std::ofstream ofs("tarjan-vishkin.csv", ios_base::app);
  ofs << total_time / NUM_ROUNDS << '\n';
  ofs.close();
  return 0;
}
