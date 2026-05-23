# cppbind — agent guide

## Build & test

```sh
# Standard build (C++11 default, sets -std=C++11 if CMAKE_CXX_STANDARD unset)
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)

# Full CI sequence (requires CC & CXX env vars):
CC=gcc CXX=g++ [CXXSTD=17] sh script/ci-test.sh [build_dir]

# Run a single test (built executables land in build/test/):
cmake .. -DCPPBIND_BUILD_UNITTESTS=TRUE && make && build/test/num_test

# Format (clang-format based on .clang-format, LLVM style, 2-space indent, 80-col):
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

## CMake options

| Flag | Default | Notes |
|------|---------|-------|
| `CPPBIND_BUILD_UNITTESTS` | TRUE | |
| `CPPBIND_BUILD_EXAMPLE` | TRUE | |
| `CPPBIND_ENABLE_ASSERTION` | FALSE | Enables `_GLIBCXX_ASSERTIONS` |
| `CMAKE_CXX_STANDARD` | 11 | C++11 min, CI also tests 17 |

## Project layout

- `include/` — header-only core. `cppbind.h` is the umbrella include.
- `src/` — static lib `libcppbind.a` (except translation, STL helpers, static init).
- `test/` — one GTest binary per `.cc`, each linked individually to `cppbind` + `gtest` + `gmock_main` + `fini` (a C object file).
- `example/{abc,ffi,msan}/` — runnable examples (invoked via `make run-examples`).
- `script/gen-hdr.sh` — generates `include/cppbind-sys.h`, a single-file amalgamation of all headers (system includes + cppbind).
- `bin/config.cc` — utility binary, not part of the library.

## Key conventions

- **Style enforced by clang-tidy-as-errors**: `WarningsAsErrors: '*'`. Disabled checks: `bugprone-narrowing-conversions`, `modernize-avoid-c-arrays`, `modernize-use-nodiscard`, `modernize-use-trailing-return-type`.
- **Naming**: classes/structs `CamelCase`, functions/variables `snake_case`, global constants `UPPER_CASE`, namespaces `lower_case`.
- **Header guards**: `__<RELATIVE_PATH_SLASHES_TO_UNDERSCORES_UPPER>__` (e.g. `__CONTAINER_LIST_H__`).

## Dependencies

Only system dependency: `python3-dev` (for `Python.h`). GTest is bundled in `3rdparty/`.
