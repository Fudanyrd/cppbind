/*
 * Initialize a C/C++ project with Python3 binding in the current directory.
 *
 * Symposis
 *  project [-f,--download-repo] [-v,--debug,--verbose]
 *
 * Description
 *  [-f,--download-repo] Use `git` to fetch `cppbind` repository.
 *  [-v,--debug,--verbose] Prints verbose debug message.
 */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <getopt.h>

static int download_repo;
static int debug;
static void dummy_log_fn(const char *fmt, ...) {}
static void stderr_log_fn(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
}
static void (*log_fn)(const char *fmt, ...) = dummy_log_fn;

/* Contents of lib.cc */
constexpr char lib_cc[] = "\n\
#include <cppbind.h>\n\
\n\
\n\
extern \"C\" PyObject *myadd(PyObject *self, PyObject *args) {\n\
  __static_assert(cppbind::CFunction_flags<decltype(&myadd)>());\n\
  assert(args && PyTuple_Check(args));\n\
  cppbind::Tuple list{cppbind::Object{args}};\n\
  if (list.size() != 2) { \n\
    PyErr_SetString(PyExc_TypeError, \"Two arguments expected\"); \n\
    return nullptr; \n\
  }\n\
  cppbind::Long a(list[0].ptr), b(list[1].ptr);\n\
  a += b;\n\
  return a.object().unwrap();\n\
}\n\
\n\
\n\
/* TODO: edit name of the package. Current value: demo */\n\
gen_modinit_fn_from_fns(\n\
    /* name */ demo,\n\
    nullptr, nullptr, nullptr, nullptr,\n\
    gen_PyMethodDef_doc(myadd, \":returns: sum of two integers\"));\n\
\n\
";

constexpr char test_py[] = "\n\
import demo\n\
\n\
def test_myadd():\n\
    assert demo.myadd(1, 2) == 3\n\
\n\
if __name__ == \"__main__\":\n\
    test_myadd()\n\
";

constexpr char CMakeLists_txt[] = "\n\
cmake_minimum_required(VERSION 3.15)\n\
project(cppbind-demo C CXX)\n\
\n\
# Cppbind configuration\n\
set(CPPBIND_BUILD_UNITTESTS FALSE)\n\
set(CPPBIND_BUILD_EXAMPLE FALSE)\n\
set(CPPBIND_ENABLE_ASSERTION FALSE)\n\
add_subdirectory(cppbind)\n\
# Dependencies\n\
find_package(Python3)\n\
find_package(PythonLibs)\n\
\n\
include_directories(cppbind/include ${PYTHON_INCLUDE_DIRS})\n\
add_library(demo SHARED lib.cc)\n\
target_link_libraries(demo cppbind)\n\
";

/* TODO: */
constexpr char setup_py[] = "\
from setuptools import setup, Extension\n\
\n\
# configure the package name and version here\n\
PACKAGE_NAME = \"demo\"\n\
VERSION = \"0.0.1\"\n\
\n\
\n\
setup(\n\
    name=PACKAGE_NAME,\n\
    version=VERSION,\n\
    ext_modules=[\n\
        Extension(\n\
            name=PACKAGE_NAME,\n\
            sources=[\"lib.cc\"],\n\
            include_dirs=[\"cppbind/include\"]\n\
        )\n\
    ]\n\
)\n\
";

int main(int argc, char **argv, char **envp) {
  static struct option long_options[] = {
      {"download-repo", no_argument, &download_repo, true},
      {"debug", no_argument, &debug, true},
      {"verbose", no_argument, &debug, true},
      {0, 0, 0, 0}};

  while (true) {
    int option_index;
    int c = getopt_long(argc, argv, "fv", long_options, &option_index);
    if (c == -1) {
      break;
    }
  }

  if (debug) {
    log_fn = stderr_log_fn;
  }

  /* Create lib.cc */
  {
    (*log_fn)("Creating lib.cc...");
    std::ofstream lib_cc_file("lib.cc");
    lib_cc_file << lib_cc;
  }

  /* Create test.py */
  {
    (*log_fn)("Creating test.py...");
    std::ofstream test_py_file("test.py");
    test_py_file << test_py;
  }

  /* Create CMakeLists.txt */
  {
    (*log_fn)("Creating CMakeLists.txt ...");
    std::ofstream CMakeLists_txt_file("CMakeLists.txt");
    CMakeLists_txt_file << CMakeLists_txt;
  }

  /* Create setup.py */
  {
    (*log_fn)("Creating setup.py ...");
    std::ofstream setup_py_file("setup.py");
    setup_py_file << setup_py;
  }

  (*log_fn)("Downloading cppbind repository...");
  char cmd[] = "git init && "
               "git submodule add --depth 1 "
               "https://github.com/Fudanyrd/cppbind.git cppbind\n";
  if (download_repo) {
    int ret = system(cmd);
    if (ret != 0) {
      fprintf(stderr,
              "Failed to download cppbind repository with command: %s\n", cmd);
      return 1;
    }
  } else {
    (*log_fn)("Skipping download of cppbind repository. Use -f or "
              "--download-repo to download it.");
  }

  return 0;
}
