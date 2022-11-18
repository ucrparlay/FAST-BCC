#include "hopcroft_tarjan.h"

#include "parlay/internal/get_time.h"

#include <iostream>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " filename\n";
    abort();
  }
  char* filename = argv[1];
  Graph g = read_graph(filename);
  ofstream ofs("tarjan_bcc.dat", ios::app);
  for (int i = 0; i <= 3; i++) {
    Tarjan solver(g);
    if (i == 0) {
      internal::timer t;
      solver.bcc();
      t.stop();
      printf("%u\n", solver.k);
      ofs << solver.k << '\n';
      printf("Round %d: %f\n", i, t.total_time());
    } else {
      internal::timer t;
      solver.bcc();
      t.stop();
      printf("Round %d: %f\n", i, t.total_time());
      ofs << t.total_time() << '\n';
      ofs.flush();
    }
  }
  return 0;
}
