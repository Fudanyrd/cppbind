add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/cppbind-sys.h
  COMMAND sh script/gen-hdr.h
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/.."
)
