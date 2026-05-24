add_custom_target(run-examples COMMAND echo)
add_subdirectory(abc)
add_subdirectory(ffi)

# Work with C++ inheritance.
add_subdirectory(inheritance)

add_subdirectory(msan)
