# FAST-BCC: Provably Fast and Space-Efficient Parallel Biconnectivity
This repository contains code for our paper "Provably Fast and Space-Efficient Parallel Biconnectivity". FAST-BCC (Fencing Arbitrary Spanning Trees) is a parallel biconnectivity algorithm that has optimal work, polylogarithmic span, and is space-efficient.

Prerequisite
--------
+ g++ or clang with C++17 features support (Tested with g++ 7.5.0 and clang 14.0.6) on Linux machines.
+ We use [ParlayLib](https://github.com/cmuparlay/parlaylib) to support fork-join parallelism and some parallel primitives. It is provided as a submodule in our repository. 

Getting Code
--------
The code can be downloaded using git:
```
git clone --recurse-submodules https://github.com/ucrparlay/FAST-BCC.git
```

Compilation
--------
Compilation can be done by using the Makefile in the src/ directory
```
cd src/
make
```
The make command compiles the executable for both FAST-BCC and Hopcroft-Tarjan. It compiles using clang by default. To compile with g++, run:
```
make GCC=1
```
To run our code on very large graphs with more than $2^{31}-1$ vertices (e.g., hyperlink2012), some extra flag needs to be passed in the compilation stage. 
```
make N2LONG=1 STDALLOC=1 
```
To compile our code to run in sequential, use: 
```
make SERIAL=1 
```

Running Code
--------
Simply run
```
./FAST_BCC [filename]
```
Make sure the input graph is a symmetric graph.

Graph Formats
--------
The application can auto-detect the format of the input graph based on the suffix of the filename. Here is a list of supported graph formats: 
+ `.bin` The binary graph format from [GBBS](https://github.com/ParAlg/gbbs). It uses compressed sparse row (CSR) format and organizes as follows:
    + $n$ - number of vertices (64-bit variable)
    + $m$ - number of edges (64-bit variable)
    + sizes - sizes of this file in bytes, which is equals to $3\times8+(n+1)\times8+m\times4$ (64-bit variable)
    + offset[] - offset[ $i$ ] (inclusive) and offset[ $i+1$ ] (exclusive) represents the range of neighbors list of the $i$-th vertex in the edges array (64-bit array of length $n+1$)
    + edges[] - edges list (32-bit array of length $m$) 
+ `.adj` The adjacency graph format from [Problem Based Benchmark suite](http://www.cs.cmu.edu/~pbbs/benchmarks/graphIO.html). 

Some graphs in binary format can be found in our [Google Drive](https://drive.google.com/drive/u/3/folders/1ZuhfaLmdL-EyOiWYqZGD1rOy_oSFRWe4). For storage limit, we cannot provide the largest graphs used in our paper. They can be found in the [Stanford Network Analysis Project](http://snap.stanford.edu/) and [Web Data Commons](http://webdatacommons.org/hyperlinkgraph/). 

Scripts
--------
We also provide scripts on downloading the graphs and benchmarking FAST-BCC and Hopcroft-Tarjan. Please refer to the ```scripts/``` folder. Simply run: 
```
./download_dataset.sh # this script downloads the dataset into data/ 
./run_fastbcc.sh # this script runs FAST-BCC 
./run_fastbcc_sequential.sh # this script runs FAST-BCC in sequential 
./run_hopcroft_tarjan.sh # this script runs Hopcroft-Tarjan 
```
The two benchmark scripts will generate two CSV files in the ```result/``` folder, which report the number of biconnected components and the running times on each graph. 

Contact
--------
If you have any questions, please submit an issue to this repository (recommended) or send an email to the author at xdong038@ucr.edu.

Reference
--------
Xiaojun Dong, Letong Wang, Yan Gu, Yihan Sun. [Provably Fast and Space-Efficient Parallel Biconnectivity](https://doi.org/10.1145/3572848.3577483). In *ACM SIGPLAN Symposium on Principles and Practice of Parallel Programming (PPoPP)*, pp. 52-65, 2023.  

Xiaojun Dong, Letong Wang, Yan Gu, Yihan Sun. [Provably Fast and Space-Efficient Parallel Biconnectivity](https://arxiv.org/abs/2301.01356). *arXiv preprint: 2301.01356*, 2023.  

If you use our code, please cite our paper:
```
@inproceedings{dong2023provably,
  author    = {Dong, Xiaojun and Wang, Letong and Gu, Yan and Sun, Yihan},
  title     = {Provably Fast and Space-Efficient Parallel Biconnectivity},
  booktitle = {ACM SIGPLAN Symposium on Principles and Practice of Parallel Programming (PPoPP)},
  pages     = {52--65},
  year      = {2023},
}
```
