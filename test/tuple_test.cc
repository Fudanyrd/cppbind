#include <cppbind.h>
#include <gtest/gtest.h>

namespace cppbind {
/*
 * NOLINTBEGIN(readability-identifier-length)
 */

TEST(Tuple, Create) {
  Tuple empty{};
  ASSERT_EQ(empty.size(), 0);

  {
    Long one(1L);
    Long two(2L);
    Object none(Py_None);
    none.inc_ref();

    Tuple a(one);
    Tuple b(one, two);
    Tuple c(one, two, one);
    Tuple d(none, one, none);

    ASSERT_EQ(d.size(), 3);
  }
}

/*
 * NOLINTEND(readability-identifier-length)
 */
} /* namespace cppbind */
