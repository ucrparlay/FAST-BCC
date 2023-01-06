#!/bin/bash
declare -a undir_graph=(
  # Social
  "com-youtube_sym.bin"
  "com-orkut_sym.bin"
  "soc-LiveJournal1_sym.bin"
  "twitter_sym.bin"
  "friendster_sym.bin"

  # Web
  "web-Google_sym.bin"
  "sd_arc_sym.bin"
  # "clueweb_sym.bin"
  # "hyperlink2014_sym.bin"
  # "hyperlink2012_sym.bin"

  # Road
  "RoadUSA_sym.bin"
  "Germany_sym.bin"

  # k-NN
  "Household.lines_5_sym.bin"
  "CHEM_5_sym.bin"
  "GeoLifeNoScale_2_sym.bin"
  "GeoLifeNoScale_5_sym.bin"
  "GeoLifeNoScale_10_sym.bin"
  "GeoLifeNoScale_15_sym.bin"
  "GeoLifeNoScale_20_sym.bin"
  "Cosmo50_5_sym.bin"

  # Synthetic
  "grid_10000_10000_sym.bin"
  "grid_1000_100000_sym.bin"
  "grid_10000_10000_03_sym.bin"
  "grid_1000_100000_03_sym.bin"
  "chain_1e7_sym.bin"
  "chain_1e8_sym.bin"
)

declare graph_path="../data/"

declare numactl="numactl -i all"

cd ../src

echo "graph,#BCC,time" > hopcroft-tarjan.csv

rm hopcroft_tarjan
make hopcroft_tarjan SERIAL=1
ulimit -s unlimited # Avoid stack overflow
for graph in "${undir_graph[@]}"; do
  echo Running on ${graph}
  echo -n "${graph}," >> hopcroft-tarjan.csv
  ${numactl} ./hopcroft_tarjan ${graph_path}${graph} 3
  echo
done

mv hopcroft-tarjan.csv ../results/

cd ../scripts
