#!/bin/bash
declare -a undir_graph=(
  # Social
  "com-orkut_sym.bin"
  "soc-LiveJournal1_sym.bin"
  "twitter_sym.bin"
  "friendster_sym.bin"

  # Web
  "sd_arc_sym.bin"
  # "clueweb_sym.bin"
  # "hyperlink2014_sym.bin"

  #Road
  "RoadUSA_sym.bin"
  "Germany_sym.bin"

  #KNN
  "Household.lines_5_sym.bin"
  "CHEM_5_sym.bin"
  "GeoLifeNoScale_2_sym.bin"
  "GeoLifeNoScale_5_sym.bin"
  "GeoLifeNoScale_10_sym.bin"
  "GeoLifeNoScale_15_sym.bin"
  "GeoLifeNoScale_20_sym.bin"
  "Cosmo50_5_sym.bin"

  #Synthetic
  "grid_10000_10000_sym.bin"
  "grid_1000_100000_sym.bin"
  "grid_10000_10000_03_sym.bin"
  "grid_1000_100000_03_sym.bin"
  "chain_1e7_sym.bin"
  "chain_1e8_sym.bin"

)

declare graph_path="../data/"

declare numactl="numactl -i all"

declare -a undir_graph2=(
  # "hyperlink2012_sym.bin"
)


cd ../src

echo "graph,#CC,#BCC,time" > fast-bcc.csv

make clean
make FAST_BCC
for graph in "${undir_graph[@]}"; do
  echo ${graph_path}${graph}
  ${numactl} ./FAST_BCC ${graph_path}${graph} 10 
done

make clean
make FAST_BCC N2LONG=1 STDALLOC=1
for graph in "${undir_graph2[@]}"; do
  echo ${graph_path}${graph}
  ${numactl} ./FAST_BCC ${graph_path}${graph} 10
done

mv fast-bcc.csv ../result/

cd ../scripts