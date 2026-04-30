cmake_minimum_required(VERSION 3.15)
project(cppbind-demo C CXX)

# Cppbind configuration
set(CPPBIND_BUILD_UNITTESTS FALSE)
set(CPPBIND_BUILD_EXAMPLE FALSE)
set(CPPBIND_ENABLE_ASSERTION FALSE)
add_subdirectory(cppbind)

# Dependencies
find_package(Python3)
find_package(PythonLibs)

include_directories(cppbind/include ${PYTHON_INCLUDE_DIRS})
add_library(_capi SHARED ${CMAKE_CURRENT_SOURCE_DIR}/demo/_capi/lib.cc)
target_link_libraries(_capi cppbind)
install(TARGETS _capi LIBRARY DESTINATION demo)

