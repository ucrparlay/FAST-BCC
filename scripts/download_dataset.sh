#!/usr/bin/env bash

declare -a urls=(
	# Social
  "https://www.dropbox.com/s/scik5vss6hj2le6/com-youtube_sym.bin?dl=0" # YT
  "https://www.dropbox.com/s/2okcj8q7q1z3nkw/com-orkut_sym.bin?dl=0" #OK
  "https://www.dropbox.com/s/6pbryf0wpxgmj7c/soc-LiveJournal1_sym.bin?dl=0" #LJ
  "https://www.dropbox.com/s/vtnfg6e75h0jama/twitter_sym.bin?dl=0" # TW
  "https://www.dropbox.com/s/6xiwui88bacbt6o/friendster_sym.bin?dl=0" # FT"

	# Web
  "https://www.dropbox.com/s/h87elg5i5tij6v5/web-Google_sym.bin?dl=0" # GG
  "https://www.dropbox.com/s/xtkei44fh9v5v73/sd_arc_sym.bin?dl=0" # SD
	# CW	
	# HL14
	# HL12

	# Road
  "https://www.dropbox.com/s/6a1k1tzu7tnhker/roadNet-CA_sym.bin?dl=0" # CA
  "https://www.dropbox.com/s/a6dhyw9ec0d3hlh/RoadUSA_sym.bin?dl=0" # USA
  "https://www.dropbox.com/s/42s5gb1gjs36x3z/Germany_sym.bin?dl=0" # GE

  # k-NN
  "https://www.dropbox.com/s/281aia4r73owtjk/Household.lines_5_sym.bin?dl=0" # HH5
  "https://www.dropbox.com/s/r0c1smelm4lllbz/CHEM_5_sym.bin?dl=0" # CH5
  "https://www.dropbox.com/s/1rmnfd09nf3hj4b/GeoLifeNoScale_2_sym.bin?dl=0" # GL2
  "https://www.dropbox.com/s/32cb0d645i2qf30/GeoLifeNoScale_5_sym.bin?dl=0" # GL5
  "https://www.dropbox.com/s/0gm8neagiunzczn/GeoLifeNoScale_10_sym.bin?dl=0" # GL10
  "https://www.dropbox.com/s/u2mm44c7637n9xg/GeoLifeNoScale_15_sym.bin?dl=0" # GL15
  "https://www.dropbox.com/s/hs2iqpowq99ivj1/GeoLifeNoScale_20_sym.bin?dl=0" # GL20
  "https://www.dropbox.com/s/8oxqut1ff7l73ws/Cosmo50_5_sym.bin?dl=0" # COS5

  # Synthetic
  "https://www.dropbox.com/s/79f8aeufjpf7q34/grid_10000_10000_sym.bin?dl=0" # SQR
  "https://www.dropbox.com/s/rvfnovitlvlu7tq/grid_1000_100000_sym.bin?dl=0" # REC
  "https://www.dropbox.com/s/gwqug9pwt2y0u3j/grid_10000_10000_03_sym.bin?dl=0" # SQR'
  "https://www.dropbox.com/s/whiw2dc8511t3vp/grid_1000_100000_03_sym.bin?dl=0" # REC'
  "https://www.dropbox.com/s/qa9zfyj7xw1w7oh/chain_1e7_sym.bin?dl=0" # Chn7
  "https://www.dropbox.com/s/vdyk92px0pm2fgi/chain_1e8_sym.bin?dl=0" # Chn8
)

declare -a graphs=(
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


for i in "${!urls[@]}"; do
  url=${urls[$i]}
  name=${graphs[$i]}
  cmd="wget -O ../data/${name} -c ${url}"
  echo -e "Downloading from "$url"...\n"
  eval $cmd
done 
