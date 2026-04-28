#!/usr/bin/env sh

set -ex

# check that CC and CXX are set
test -n "$CC"
test -n "$CXX"

mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
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
