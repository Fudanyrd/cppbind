cmake_minimum_required(VERSION 3.22)

# Another C++/Python3 Binding
project(cppbind)


# Compiler Flags
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD "11" CACHE STRING "C++ standard to enforce")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# XXX: Read these options.
option(BUILD_UNITTESTS "Build the unittest runner." TRUE)


# only system dependency is python3-dev
find_package(Python3)
find_package(PythonLibs)
set(MY_GLOBL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include ${PYTHON_INCLUDE_DIRS})

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

if(BUILD_UNITTESTS)
  add_subdirectory(test)
else()
endif()

