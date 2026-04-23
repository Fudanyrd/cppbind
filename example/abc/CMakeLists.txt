include_directories(${MY_GLOBL_INCLUDE_DIRS})
add_library(myabc SHARED myadd.cc)
target_link_libraries(myabc cppbind ${PYTHON_LIBRARY})

add_custom_target(eg-abc
  COMMAND  ln -sf libmyabc.so myabc.so && ln -sf "${CMAKE_CURRENT_SOURCE_DIR}/test.py" test.py && python3 < test.py
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS myabc "${CMAKE_CURRENT_SOURCE_DIR}/test.py")

add_dependencies(run-examples eg-abc)
