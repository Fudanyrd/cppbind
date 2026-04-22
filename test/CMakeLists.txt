include_directories(${MY_GLOBL_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/gtest/googletest/include)
file(GLOB_RECURSE MY_TEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_library(testbind STATIC ${MY_TEST_SRC})
