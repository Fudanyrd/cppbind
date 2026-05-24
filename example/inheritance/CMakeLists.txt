include_directories(${CPPBIND_INCLUDE_DIRS})
add_library(ext SHARED ext.cc)
target_link_libraries(ext cppbind cppbind-aux ${PYTHON_LIBRARY})

add_custom_target(eg-inheritance
  COMMAND  ln -sf libext.so ext.so && ln -sf "${CMAKE_CURRENT_SOURCE_DIR}/test.py" test.py && python3 < test.py
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS ext "${CMAKE_CURRENT_SOURCE_DIR}/test.py")

add_dependencies(run-examples eg-inheritance)
