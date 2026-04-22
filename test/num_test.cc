#include <cppbind.h>
#include <stl.h>

#include <gtest/gtest.h>

using cppbind::Long;
using cppbind::Object;
using cppbind::stringify;

TEST(ToString, Long) {
  Long long_obj(1234l);
  ASSERT_EQ(stringify(long_obj), "1234");
}
