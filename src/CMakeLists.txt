include_directories(${CPPBIND_INCLUDE_DIRS})
file(GLOB CPPBIND_CPP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_library(cppbind STATIC ${CPPBIND_CPP_SRC})

