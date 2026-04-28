include_directories(${CPPBIND_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
add_library(_ffi SHARED _ffi.cc)
target_link_libraries(myabc cppbind ${PYTHON_LIBRARY})

add_custom_target(eg-ffi
  COMMAND  ln -sf lib_ffi.so _ffi.so && ln -sf "${CMAKE_CURRENT_SOURCE_DIR}/test.py" test.py && python3 < test.py
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS _ffi "${CMAKE_CURRENT_SOURCE_DIR}/test.py")

add_dependencies(run-examples eg-ffi)
