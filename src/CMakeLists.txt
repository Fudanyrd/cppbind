include_directories(${CPPBIND_INCLUDE_DIRS})
file(GLOB CPPBIND_CPP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_library(cppbind STATIC ${CPPBIND_CPP_SRC})
install(TARGETS cppbind EXPORT cppbind-targets ARCHIVE DESTINATION ${INSTALL_LIB_DIR})

# install config.cmake => cppbind-config.cmake
configure_file(config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cppbind-config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cppbind-config.cmake
        DESTINATION ${INSTALL_CMAKE_DIR})

