include_directories(${MY_GLOBL_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/gtest/googletest/include)
file(GLOB_RECURSE MY_TEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_library(testbind STATIC ${MY_TEST_SRC})
add_custom_target(run-tests COMMAND echo)

if(BUILD_UNITTESTS)
  foreach(source_file ${MY_TEST_SRC})
    get_filename_component(filename ${source_file} NAME)
    string(REPLACE ".cc" "" testname ${filename})

    add_executable(${testname} ${source_file} fini.c)
    target_link_libraries(${testname} cppbind gtest gmock_main ${PYTHON_LIBRARY})

    add_dependencies(run-tests run-${testname})
    add_custom_target("run-${testname}" COMMAND "${CMAKE_CURRENT_BINARY_DIR}/${testname}")
    set_target_properties(${testname}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
            COMMAND ${testname}
            )
  endforeach()
else()
endif()

