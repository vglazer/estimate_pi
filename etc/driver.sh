#!/bin/bash

set -e # exit on error

script_path=$(realpath -s "${BASH_SOURCE[0]}")
script_dir=$(dirname "${script_path}")
script_name=$(basename "${script_path}")

[[ $# -ne 2 ]] && { echo "usage: ${script_name} <num_threads> <max_num_points>"; exit 1; }

max_num_points=$2
min_num_points=1000 # dividisble by 8
[[ ${max_num_points} -lt ${min_num_points}  ]] && { echo "use at least ${min_num_points} points"; exit 1; }

data_dir="${script_dir}"/../data
mkdir -p "${data_dir}"
cd "${data_dir}"

num_threads=$1
i=${min_num_points}
while [[ ${i} -le ${max_num_points} ]];
do
   echo "${i}"

   "${script_dir}"/../build/Release/estimate_pi "${num_threads}" "${i}"

   i=$((i * 2))
done

"${script_dir}"/plot.py
