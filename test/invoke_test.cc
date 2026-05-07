#include <memory>
#include <vector>
using std::vector;

#include <cppbind.h>
#include <gtest/gtest.h>

/* NOLINTBEGIN(readability-identifier-length) */

static long wierd(long a, long b) { return 2 * a + b; }

static void vec_push(vector<long> &vec, long value) { vec.push_back(value); }

static const long MAGIC_NUM = 42L;
static long magic() { return MAGIC_NUM; }

static std::unique_ptr<long> cxx_uniq_ptr(PyObject *self, PyObject *const *args,
                                          Py_ssize_t nargs) {
#if __cplusplus >= 201402L
  return std::make_unique<long>(MAGIC_NUM);
#else
  return std::unique_ptr<long>(new long(MAGIC_NUM));
#endif /* C++ version >= 14 */
}

template <>
cppbind::Object
cppbind::into<std::unique_ptr<long> &&>(std::unique_ptr<long> &&value) {
  return Long{*value}.object();
}

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

  std::function<long(long, long)> fn = wierd;
  PyArgs<0, long, long, long> py_args(args);

  long actual = py_args.call(fn);
  ASSERT_EQ(actual, wierd(val1, val2));
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

  std::function<long(long, long)> fn = wierd;
  PyVecCallArgs<0, long, long, long> py_args(args, 2);
  long ret = py_args.call(fn);
  ASSERT_EQ(ret, wierd(val1, val2));
  ASSERT_EQ(ret, wierd(val1, val2));
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

TEST(InvokeVec, NonCopy) {
  auto *pt = fastcall_and_into(cxx_uniq_ptr, nullptr, 0);
  ASSERT_TRUE(PyLong_Check(pt));

  Long value{pt};
  ASSERT_EQ((long)value, MAGIC_NUM);
}

} /* namespace cppbind */

/* NOLINTEND(readability-identifier-length) */
