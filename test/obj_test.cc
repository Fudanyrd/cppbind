#include <bits/move.h>

#include <cppbind.h>
#include <gtest/gtest.h>

/**
 * <h3>performance-unnecessary-copy-initialization</h3>
 * TEST(Object, Create) should check assignment operator
 * works as expected.
 *
 * <h3>bugprone-use-after-move</h3>
 * TEST(Object, Move) checks that `Object::ptr` set to null
 * after move.
 *
 * <h3>readability-function-cognitive-complexity</h3>
 * We instead try to keep each test <b>easy to understand</b> (though
 * it may still be lengthy).
 */
/**
 * NOLINTBEGIN(performance-unnecessary-copy-initialization,
 *             readability-function-cognitive-complexity,
 *             bugprone-use-after-move)
 */

static PyObject *get_none_obj(void) { Py_RETURN_NONE; }

/* create a PyLong of magic value. */
static PyObject *get_long_obj(void) {
  long magic = 0x12345678;
  return PyLong_FromLong(magic);
}

namespace cppbind {

TEST(Object, Create) {
  auto *zero = get_long_obj();
  if (likely(zero != nullptr)) {
    Object guard(zero);
    auto refcnt = guard.ref_count();
    ASSERT_TRUE(refcnt == 1);
    {
      Object obj = guard;
      /* because of copy, refcount ++ */
      ASSERT_EQ(obj.ref_count(), 1 + refcnt);
      ASSERT_EQ(guard.ref_count(), 1 + refcnt);
      /* obj is dropped, refcount -- */
    }
    ASSERT_EQ(guard.ref_count(), refcnt);
    Py_DECREF(zero);
  } else {
    FAIL() << "Failed to create long object.";
  }
}

TEST(Object, ImmortalObject) {
  auto *none = get_none_obj();
  if (likely(none != nullptr)) {
    Object guard(none);
    auto refcnt = guard.ref_count();
    ASSERT_TRUE(refcnt > 0);
    {
      Object obj = guard;
      /* because of copy, refcount ++ */
      ASSERT_EQ(obj.ref_count(), 1 + refcnt);
      ASSERT_EQ(guard.ref_count(), 1 + refcnt);
      /* obj is dropped, refcount -- */
    }
    ASSERT_EQ(guard.ref_count(), refcnt);

    /* self assignment should have no effect */
    guard = guard;
    ASSERT_EQ(guard.ref_count(), refcnt);
  } else {
    FAIL() << "Failed to create None object.";
  }
}

TEST(Object, Move) {
  auto *lobj = get_long_obj();
  if (likely(lobj != nullptr)) {
    Object guard(lobj);
    auto refcnt = guard.ref_count();
    ASSERT_TRUE(refcnt > 0);

    Object obj1 = guard;
    ASSERT_EQ(obj1.ref_count(), refcnt + 1);

    /*
     * ownership transferred to obj2.
     * NOTE: obj1 cannot be used (including assigning new value)
     */
    Object obj2 = std::move(obj1);
    ASSERT_EQ(obj2.ref_count(), refcnt + 1);
    ASSERT_EQ(obj1.ptr, nullptr);

    guard = std::move(obj2);
    ASSERT_EQ(guard.ref_count(), refcnt);

    guard = std::move(guard);
    ASSERT_EQ(guard.ref_count(), refcnt);
  } else {
    FAIL() << "Failed to create None object.";
  }
}

TEST(Object, InplaceNumOp) {
  PyObject *one = PyLong_FromLong(1);
  PyObject *two = PyLong_FromLong(2);
  if (likely(one && two)) {
    Object guard1(one);
    Object guard2(two);
    auto refcnt = guard2.ref_count();
    Object resguard = guard2;

    /* will generate a new (float) object. */
    resguard.inplace_num_TrueDivide(one);
    ASSERT_TRUE(resguard.ptr != guard2.ptr);
    ASSERT_EQ(guard2.ref_count(), refcnt);
  } else {
    FAIL() << "Failed to create long objects.";
  }
}

} /* namespace cppbind */

/**
 * NOLINTEND(performance-unnecessary-copy-initialization,
 *           readability-function-cognitive-complexity,
 *           bugprone-use-after-move)
 */