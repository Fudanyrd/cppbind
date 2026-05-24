# cppbind — agent guide

## Build & test

```sh
# Standard build (C++11 default)
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)

# Full CI sequence (requires CC & CXX):
CC=gcc CXX=g++ [CXXSTD=17] sh script/ci-test.sh [build_dir]

# Run all tests via cmake target:
make run-tests

# Run all examples:
make run-examples

# Run a single test (after cmake configure):
cmake .. -DCPPBIND_BUILD_UNITTESTS=TRUE && make && build/test/num_test
# Or via cmake target: make run-num_test

# Format (LLVM style, 2-space indent, 80-col):
sh script/format.sh

# Check formatting only:
sh script/ci-check-fmt.sh

# clang-tidy (needs build/compile_commands.json first):
sh script/ci-tidy.sh

# Shellcheck all scripts:
sh script/ci-lint-sh.sh

# Doxygen (WARN_AS_ERROR=YES — warnings are fatal):
sh script/docs.sh doxygen
```

## CI workflow (`.github/workflows/main.yml`)

Runs on **self-hosted** Ubuntu 24.04 runners. Order: `ci-init.sh` (install deps) → `ci-check-fmt.sh` → `ci-test.sh` (build, test, run examples, install) → `ci-tidy.sh` → `ci-lint-sh.sh` → `docs.sh doxygen`.

Matrix: GCC C++11, Clang C++11, Clang C++17.

## CMake options

| Flag | Default | Notes |
|------|---------|-------|
| `CPPBIND_BUILD_UNITTESTS` | TRUE | |
| `CPPBIND_BUILD_EXAMPLE` | TRUE | |
| `CPPBIND_ENABLE_ASSERTION` | FALSE | Enables `_GLIBCXX_ASSERTIONS` |
| `CMAKE_CXX_STANDARD` | 11 | C++11 min, CI also tests 17 |

## Project layout

- `include/` — header-only core. `cppbind.h` is the umbrella include.
- `include/cppbind-sys.h` — **generated** amalgamation (by `script/gen-hdr.sh`). Do not edit by hand.
- `src/` — static lib `libcppbind.a` (except.cc, static.cc, stl.cc).
- `test/` — one GTest binary per `.cc`, each linked to `cppbind` + `gtest` + `gmock_main` + **`fini`** (a C object file).
- `example/{abc,ffi,inheritance,msan}/` — runnable examples (`make run-examples`). `example/custom/` is scikit-build, not part of cmake build.
- `bin/config.cc` — utility binary, not part of the library.

## Key conventions

- **Style enforced as errors by clang-tidy**: `WarningsAsErrors: '*'`. Disabled checks: `bugprone-narrowing-conversions`, `modernize-avoid-c-arrays`, `modernize-use-nodiscard`, `modernize-use-trailing-return-type`.
- **Naming**: classes/structs `CamelCase`, functions/variables `snake_case`, global constants `UPPER_CASE`, namespaces `lower_case`.
- **Header guards**: `__<RELATIVE_PATH_SLASHES_TO_UNDERSCORES_UPPER>__` (e.g. `__CONTAINER_LIST_H__`).
- **CMake custom variables**: prefix with `CPPBIND_`.
- **Clang-format**: LLVM style, IndentWidth=2, ColumnLimit=80, PointerAlignment=Right.

## Dependencies

Only system dependency: `python3-dev` (for `Python.h`). GTest is bundled in `3rdparty/gtest/`.
