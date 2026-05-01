cmake_minimum_required(VERSION 3.22)

# Another C++/Python3 Binding
project(cppbind HOMEPAGE_URL "https://github.com/Fudanyrd/cppbind")


# Compiler Flags
# we build position independent code by default, so that the library can be 
# linked into shared libraries and executables without issues.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD "11" CACHE STRING "C++ standard to enforce")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS_DEBUG "-Wpedantic -Wall -g -g3 -O0 -Wno-address")

# XXX: Read these options.
option(CPPBIND_BUILD_UNITTESTS "Build the unittest runner." TRUE)
option(CPPBIND_BUILD_EXAMPLE "Build the example programs." TRUE)
option(CPPBIND_ENABLE_ASSERTION "Enable assertion checks." FALSE)

if(CPPBIND_ENABLE_ASSERTION)
  add_definitions(-D_GLIBCXX_ASSERTIONS)
endif()

# only system dependency is python3-dev
find_package(Python3)
find_package(PythonLibs)
set(CPPBIND_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include ${PYTHON_INCLUDE_DIRS})

# Determine install paths
# default to gnu standard installation directories (lib, bin, include)
# https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)
set(INSTALL_LIB_DIR
    ${CMAKE_INSTALL_LIBDIR}
    CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR
    ${CMAKE_INSTALL_BINDIR}
    CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR
    ${CMAKE_INSTALL_INCLUDEDIR}
    CACHE PATH "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR cmake)
else()
  set(DEF_INSTALL_CMAKE_DIR ${INSTALL_LIB_DIR}/cmake/cppbind)
endif()
set(INSTALL_CMAKE_DIR
    ${DEF_INSTALL_CMAKE_DIR}
    CACHE PATH "Installation directory for CMake files")


add_subdirectory(3rdparty)
add_subdirectory(bin)
add_subdirectory(src)

add_custom_target(format
  COMMAND sh ./script/format.sh
  DEPENDS script/format.sh
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(CPPBIND_BUILD_UNITTESTS)
  add_subdirectory(test)
else()
endif()

if(CPPBIND_BUILD_EXAMPLE)
  add_subdirectory(example)
else()
endif()
