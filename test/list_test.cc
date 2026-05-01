#include <cppbind.h>

#include <gtest/gtest.h>

/* NOLINTBEGIN(readability-magic-numbers) */
/**
 * The magic numbers `0x12345678` is chosen because
 * in python, its reference count is likely to be 1.
 */

namespace cppbind {

TEST(List, Create) {
  List empty{};

  {
    Long one(1L);
    Long two(2L);
    Object none(Py_None);
    none.inc_ref();

    List list_a(one);
    List list_b(one, two);
    List list_c(one, two, one);
    List list_d(none, one, none);
  }
}

TEST(List, GetAndSet) {
  List list;
  constexpr int len = 4;

  {
    Object none(Py_None);
    none.inc_ref();
    for (int i = 0; i < len; i++) {
      list.append(none);
    }
  }

#define VALUE_IDX(i) ((long)(i))

  /* Set the list to certain values. */ {
    for (int i = 0; i < len; i++) {
      Object obj = Long(VALUE_IDX(i)).object();
      list[i] = obj;
    }
  }

  {
    for (int i = 0; i < len; i++) {
      Object obj = list[i];
      Long actual{obj};
      EXPECT_EQ(VALUE_IDX(i), (long)actual);
    }
  }
#undef VALUE_IDX
}

TEST(List, Append) {
  List list;
  EXPECT_EQ(list.size(), 0);

  Long one(1L);
  Long two(2L);
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

TEST(List, SameObjInList) {
  PyObject *obj = Long(0x12345678).object().unwrap();
  const auto refcnt = Py_REFCNT(obj);
  ASSERT_TRUE(refcnt > 0);

  {
    List list;
    Object item(obj);
    list.append(item);
    list.append(item);
    (void)item.unwrap();
    ASSERT_EQ(list.size(), 2);
    ASSERT_EQ(Py_REFCNT(obj), refcnt + 2);
  }
  ASSERT_EQ(Py_REFCNT(obj), refcnt);

  /* append the same item to several lists. */
  {
    List list1;
    List list2;
    Object item(obj);
    list1.append(item);
    list2.append(item);
    /* free the ownership taken by `item` */
    (void)item.unwrap();
    ASSERT_EQ(Py_REFCNT(obj), refcnt + 2);
  }

  /* Free the object. */
  Py_DECREF(obj);
}

/* NOLINTEND(readability-magic-numbers) */
} /* namespace cppbind */
