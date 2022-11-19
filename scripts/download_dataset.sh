#!/usr/bin/env bash

declare -a urls=(
	# Social
	"https://drive.google.com/file/d/1H4tOm3-MCWy6AXXcH7fgI2MgiIwrw8aK/view" # OK
	"https://drive.google.com/file/d/1p4M_zKe_xlkhhVB9Qo1qJZqUBD1CJaH5/view" # LJ
	"https://drive.google.com/file/d/14qrCZ5o-ZDTsQGXeamTL0RLwunf1TbEx/view" # TW
	"https://drive.google.com/file/d/1PvL4Oq9XVjOQkgtsPrlv9-8P9SztErgA/view" # FT

	# Web
	"https://drive.google.com/file/d/1KXl6J8958yKnTaUwDP_di2aRpAs9juYg/view" # SD
	# CW	
	# HL14
	# HL12

	# Road
	"https://drive.google.com/file/d/1FhxSHytXKNxReZITwNK44cFJSKPE_bTX/view" # USA
	"https://drive.google.com/file/d/1lpzxoGRdNjIisAHZTWSaErYDLAnmubHK/view" # GE

  # k-NN
  "https://drive.google.com/file/d/12Pok1x3l3O-0ZFl3WM5CHkGIwzRDkzHh/view" # HH5
  "https://drive.google.com/file/d/192k4lQttPffS6XGQWFLqbSWCqRu8RDOg/view" # CH5
  "https://drive.google.com/file/d/1s8tPRsOuUN4mXwqVDcbocr8wunBuUjyu/view" # GL2
  "https://drive.google.com/file/d/1sCFanz_wzkrIrqhgHq_A0FJ8XQkDi8OS/view" # GL5
  "https://drive.google.com/file/d/1Xbn29-TCIZr3wgPtZ4KXiWWEhZgESmjF/view" # GL10
  "https://drive.google.com/file/d/1x_oaiuKqTPNELHqas1RrIKuT9tJOh5m9/view" # GL15
  "https://drive.google.com/file/d/1aJoEAmCLkx-ffvH--4bRNX1I46tDZgrD/view" # GL20
  "https://drive.google.com/file/d/1JaHYOu9fY0k3hmjZINtBpuGaB67_wLMe/view" # COS5

  # Synthetic
  "https://drive.google.com/file/d/1_15XjjdaHaw0eglLnB4rwXIpBnJwg3sg/view" # SQR
  "https://drive.google.com/file/d/1PI_8F7pNsf9EF-JEmO_8csykHCADPL_1/view" # REC
  "https://drive.google.com/file/d/1eZCd6eUrL5nXOH_b1yx4Wm9sEc07a0sT/view" # SQR'
  "https://drive.google.com/file/d/1WwRq7602vnKiVPGbI1ooCACvpvjD2t4B/view" # REC'
  "https://drive.google.com/file/d/17k1D5KLA9UWA7df-rq0DpcjoEW141qmj/view" # Chn7
  "https://drive.google.com/file/d/1u4JHb8tuJvjeXS1qWMv9aNh4xutRwjSd/view" # Chn8
)

declare -a graphs=(
  # Social
  "com-orkut_sym.bin"
  "soc-LiveJournal1_sym.bin"
  "twitter_sym.bin"
  "friendster_sym.bin"

  # Web
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


for i in "${!urls[@]}"; do
  gURL=${urls[$i]}
  name=${graphs[$i]}
  # match more than 26 word characters  
  ggID=$(echo "$gURL" | egrep -o '(\w|-){26,}')

  ggURL='https://drive.google.com/uc?export=download'

  curl -sc cookie.txt "${ggURL}&id=${ggID}" >/dev/null  
  getcode="$(awk '/_warning_/ {print $NF}' cookie.txt)"

  cmd='curl --insecure -o ../data/${name} -C - -LOJb cookie.txt "${ggURL}&confirm=${getcode}&id=${ggID}"'
  echo -e "Downloading from "$gURL"...\n"
  eval $cmd
done 
