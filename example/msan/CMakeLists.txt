include_directories(${CPPBIND_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
add_library(msan SHARED msan.cc)
target_link_libraries(msan cppbind ${PYTHON_LIBRARY})

add_custom_target(eg-msan
  COMMAND  ln -sf libmsan.so msan.so && ln -sf "${CMAKE_CURRENT_SOURCE_DIR}/test.py" test.py && python3 < test.py
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS msan "${CMAKE_CURRENT_SOURCE_DIR}/test.py")

add_dependencies(run-examples eg-msan)
