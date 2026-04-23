#include <cppbind.h>

#include <gtest/gtest.h>

namespace cppbind {

TEST(List, Create) {
  List empty{};

  {
    Long one(1l);
    Long two(2l);
    Object none(Py_None);
    none.inc_ref();

    List a(one);
    List b(one, two);
    List c(one, two, one);
    List d(none, one, none);
  }
}

TEST(List, Append) {
  List list;
  EXPECT_EQ(list.size(), 0);

  Long one(1l);
  Long two(2l);
  Object none(Py_None);
  none.inc_ref();

  list.append(one);
  EXPECT_EQ(list.size(), 1);

  list.append(two);
  EXPECT_EQ(list.size(), 2);

  list.append(none);
  EXPECT_EQ(list.size(), 3);
}

TEST(List, RefCount) {
  List list;
  Py_ssize_t refcnt;
  {
    Long obj(0x12345678);
    ASSERT_EQ(obj.object().ref_count(), 1);
    list.append(obj);

    /* After `.append`, list also shares a reference. */
    refcnt = obj.object().ref_count();
    ASSERT_EQ(refcnt, 2);
  }
  for (int i = 0; i < 2; i++) {
    Object item = list[0];
    /* Reference shared by `list` and `item`. */
    ASSERT_EQ(item.ref_count(), 2);
  }
}

TEST(List, DieEarly) {
  Long integer(0x12345678);
  Object &obj = integer.object();
  auto refcnt = obj.ref_count();

  {
    List lst;
    lst.append(obj);
  }
  ASSERT_EQ(refcnt, obj.ref_count());

  {
    List lst;
    for (int i = 0; i < 4; i++) {
      lst.append(obj);
    }
  }
  ASSERT_EQ(refcnt, obj.ref_count());
}

} /* namespace cppbind */
