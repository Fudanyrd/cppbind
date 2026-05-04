#include <vector>
using std::vector;

#include <cppbind.h>
#include <gtest/gtest.h>

/* NOLINTBEGIN(readability-identifier-length) */

static long wield(long a, long b) { return 2 * a + b; }

static void vec_push(vector<long> &vec, long value) { vec.push_back(value); }

static const long MAGIC_NUM = 42L;
static long magic() { return MAGIC_NUM; }

namespace cppbind {

/**
 * This is not how you should write conversion for your own
 * C++ types. TODO:
 */
template <> std::vector<long> &from<std::vector<long> &>(PyObject *obj) {
  return *reinterpret_cast<std::vector<long> *>(obj);
}

TEST(Invoke, ValueOnly) {
  long val1 = 1L;
  long val2 = 2L;
  Long obj1(val1);
  Long obj2(val2);

  Tuple args(obj1, obj2);

  std::function<long(long, long)> fn = wield;
  PyArgs<0, long, long, long> py_args(args);

  long actual = py_args.call(fn);
  ASSERT_EQ(actual, wield(val1, val2));
}

/**
 * Check specialization for no-argument functions.
 */
TEST(Invoke, NoArg) {
  Tuple args;
  std::function<long()> fn = magic;
  PyArgs<0, long> py_args(args);
  long actual = py_args.call(fn);
  ASSERT_EQ(actual, magic());
}

TEST(InvokeVec, ValueOnly) {
  long val1 = 1L;
  long val2 = 2L;
  Long obj1(val1);
  Long obj2(val2);

  PyObject *args[] = {
      obj1.object().ptr,
      obj2.object().ptr,
  };

  std::function<long(long, long)> fn = wield;
  PyVecCallArgs<0, long, long, long> py_args(args, 2);
  long ret = py_args.call(fn);
  ASSERT_EQ(ret, wield(val1, val2));
  ASSERT_EQ(ret, wield(val1, val2));
}

/**
 * Test that both reference to, and value of, a C++ type can be passed to the
 * function. The test also covers the case that the argument is converted from
 * Python objects to C++ types.
 */
TEST(InvokeVec, RefAndValue) {
  std::vector<long> vec;
  Long value(MAGIC_NUM);

  PyObject *args[] = {
      reinterpret_cast<PyObject *>(&vec),
      value.object().ptr,
  };

  std::function<void(vector<long> &, long)> fn = vec_push;
  PyVecCallArgs<0, void, vector<long> &, long> py_args(args, 2);
  py_args.call(fn);

  ASSERT_EQ(vec.size(), 1);
  ASSERT_EQ(vec[0], MAGIC_NUM);

  py_args.call(fn);
  ASSERT_EQ(vec.size(), 2);
  ASSERT_EQ(vec[0], MAGIC_NUM);
  ASSERT_EQ(vec[1], MAGIC_NUM);
}

TEST(InvokeVec, NoArg) {
  PyObject *args[] = {};
  std::function<long()> fn = magic;
  PyVecCallArgs<0, long> py_args(args, 0);
  long ret = py_args.call(fn);
  ASSERT_EQ(ret, magic());
}

} /* namespace cppbind */

/* NOLINTEND(readability-identifier-length) */
