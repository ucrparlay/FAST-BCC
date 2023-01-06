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

  # Road
  "roadNet-CA_sym.bin"
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

echo "graph,#BCC,time" > tarjan-vishkin.csv

rm tarjan_vishkin
make tarjan_vishkin
for graph in "${undir_graph[@]}"; do
  echo Running on ${graph}
  echo -n "${graph}," >> tarjan-vishkin.csv
  ${numactl} ./tarjan_vishkin ${graph_path}${graph} 3
  echo
done

mv tarjan-vishkin.csv ../results/

cd ../scripts
