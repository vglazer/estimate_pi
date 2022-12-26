#!/bin/bash

set -e # exit on error

script_name=$(basename "$0")
[[ $# -ne 2 ]] && { echo "usage: ${script_name} <num_threads> <max_num_points>"; exit 1; }

num_threads=$1
max_num_points=$2
min_num_points=1000 # dividisble by 8
[[ $max_num_points -lt $min_num_points  ]] && { echo "use at least $min_num_points points"; exit 1; }

i=$min_num_points
while [[ ${i} -le ${max_num_points} ]];
do
   echo "${i}"

   ../build/Release/estimate_pi "${num_threads}" "${i}"

   i=$((i * 2))
done

echo "points_${i}"
./plot.py
#rm *.csv
