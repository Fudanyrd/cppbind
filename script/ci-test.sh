#!/usr/bin/env sh

set -e

# check that CC and CXX are set
test -n "$CC"
test -n "$CXX"

mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_UNITTESTS=TRUE \
  -DENABLE_ASSERTION=TRUE \
  -DBUILD_EXAMPLES=TRUE \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_CXX_COMPILER="$CXX"

make -j"$(nproc)"
make run-tests

# run examples
make run-examples
exit 0
