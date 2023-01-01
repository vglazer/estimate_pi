#!/bin/bash

set -e # exit on error

script_path=$(realpath -s "${BASH_SOURCE[0]}")
script_dir=$(dirname "${script_path}")
script_name=$(basename "${script_path}")

[[ $# -ne 2 ]] && { echo "usage: ${script_name} <num_threads> <max_num_points>"; exit 1; }

build_dir="build/Release"
build_dir_path=$(realpath "${script_dir}/../${build_dir}")
echo "Cleaning up ${build_dir_path}..."
rm -rf "${build_dir_path}"
variant="Release"
generator="Ninja"
echo "Generating build files..."
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING="${variant}" -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++ -S "${script_dir}/.." -B "${build_dir_path}" -G "${generator}"
echo "Building using ${generator}..."
cmake --build "${build_dir_path}"

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
