include_directories(${MY_GLOBL_INCLUDE_DIRS})
file(GLOB MY_CPP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_library(cppbind STATIC ${MY_CPP_SRC})

