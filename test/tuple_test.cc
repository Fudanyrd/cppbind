#include <cppbind.h>
#include <gtest/gtest.h>

namespace cppbind {

TEST(Tuple, Create) {
  Tuple empty{};
  ASSERT_EQ(empty.size(), 0);

  {
    Long one(1l);
    Long two(2l);
    Object none(Py_None);
    none.inc_ref();

    Tuple a(one);
    Tuple b(one, two);
    Tuple c(one, two, one);
    Tuple d(none, one, none);

    ASSERT_EQ(d.size(), 3);
  }
}

} /* namespace cppbind */
