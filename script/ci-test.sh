#!/usr/bin/env sh
# Usage:
# CC={C compiler} CXX={CXX Compiler} [CXXSTD={11,14,17,20}] ci-test.sh [build_dir]

set -ex

# check that CC and CXX are set
test -n "$CC"
test -n "$CXX"

build_dir="build"
if test -n "$1"; then
  build_dir="$1"
  echo "Using custom build directory: $build_dir" 1>&2
fi

if test -n "$CXXSTD"; then
  cxx_std_flag="-DCMAKE_CXX_STANDARD=$CXXSTD"
else
  cxx_std_flag=""
fi

mkdir -p "$build_dir" && cd "$build_dir"

# When CXXSTD is not set, use cmake default, and cxx_std_flag should
# not be double-quoted.
# shellcheck disable=SC2086
cmake .. -DCMAKE_BUILD_TYPE=Release \
  $cxx_std_flag \
  -DCPPBIND_BUILD_UNITTESTS=TRUE \
  -DCPPBIND_ENABLE_ASSERTION=TRUE \
  -DCPPBIND_BUILD_EXAMPLE=TRUE \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_CXX_COMPILER="$CXX"

make -j"$(nproc)"
make run-tests

# run examples
make run-examples
exit 0
