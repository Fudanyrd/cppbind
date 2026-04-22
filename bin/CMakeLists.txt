include_directories(${MY_GLOBL_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/gtest/googletest/include)

# bin/config[.exe]
add_executable(config config.cc)

if(BUILD_UNITTESTS)
  add_executable(test test.cc)
  target_link_libraries(test cppbind testbind gtest)
else()
endif()

