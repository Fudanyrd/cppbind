#include <cppbind.h>
#include <stl.h>

#include <gtest/gtest.h>

namespace cppbind {

TEST(Long, ToString) {
  Long long_obj(1234l);
  ASSERT_EQ(stringify(long_obj), "1234");

  long_obj = Integer(-54321l);
  ASSERT_EQ(stringify(long_obj), "-54321");

  long_obj = Long(0l);
  ASSERT_EQ(stringify(long_obj), "0");
}

} /* namespace cppbind */
