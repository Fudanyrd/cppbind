#include <cinttypes>
#include <cstdio>

#include <cppbind.h>
#include <stl.h>

#include <gtest/gtest.h>

typedef long int i64;
typedef unsigned long long u64;

#define pri_i64 "%" PRId64
#define pri_u64 "%" PRIu64

#define gen_test_for_inplace_op(name, operator_)                               \
  static void CONCAT(test_, name)(i64 a, i64 b) {                              \
    i64 expected = a;                                                          \
    expected operator_ b; /* expected result */                                \
    cppbind::Long la(a), lb(b);                                                \
    la operator_ lb;                                                           \
    i64 actual = (i64)la; /* actual op result */                               \
    if (actual != expected) {                                                  \
      fprintf(stderr, "In function %s (" pri_i64 ", " pri_i64 ")\n", __func__, \
              a, b);                                                           \
      fflush(stderr);                                                          \
      FAIL() << "Got " << actual;                                              \
    }                                                                          \
  }

#define gen_decl_for_inplace_op(name, operator_)                               \
  static void CONCAT(test_, name)(i64 a, i64 b);

#define gen_test_for_op(name, operator_)                                       \
  static void CONCAT(test_, name)(i64 a, i64 b) {                              \
    i64 expected = a operator_ b; /* expected result */                        \
    cppbind::Long la(a), lb(b);                                                \
    cppbind::Long lc = la operator_ lb;                                        \
    i64 actual = (i64)lc; /* actual op result */                               \
    if (actual != expected) {                                                  \
      fprintf(stderr, "In function %s (" pri_i64 ", " pri_i64 ")\n", __func__, \
              a, b);                                                           \
      fflush(stderr);                                                          \
      FAIL() << "Got " << actual;                                              \
    }                                                                          \
  }

#define foreach_inplace(X)                                                     \
  X(inplace_add, +=)                                                           \
  X(inplace_sub, -=)                                                           \
  X(inplace_rsh, >>=)                                                          \
  X(inplace_lsh, <<=)                                                          \
  X(inplace_mul, *=)                                                           \
  X(inplace_div, /=)                                                           \
  X(inplace_rem, %=)                                                           \
  X(inplace_and, &=)                                                           \
  X(inplace_or, |=)                                                            \
  X(inplace_xor, ^=)

foreach_inplace(gen_decl_for_inplace_op);

namespace cppbind {

TEST(Long, ToString) {
  Long long_obj(1234l);
  ASSERT_EQ(stringify(long_obj), "1234");

  long_obj = Integer(-54321l);
  ASSERT_EQ(stringify(long_obj), "-54321");

  long_obj = Long(0l);
  ASSERT_EQ(stringify(long_obj), "0");
}

TEST(Long, InplaceArith) {
  /* first thing -- test convertion to and from C-long */ {
    i64 magic = 0xc0ffee;
    Long value(magic);
    ASSERT_EQ((i64)value, magic);
  }

  /* simply test add, substract */ {
    test_inplace_add(1, 2);
    test_inplace_sub(1, 2);
    test_inplace_add(-1, -2);
    test_inplace_sub(-1, -2);
  }

  /* test shift op, for both positive and negative. */ {
    test_inplace_rsh(1, 2);
    test_inplace_rsh(-12, 3);
    test_inplace_lsh(1, 2);
    test_inplace_lsh(-12, 3);

    /* do not test shifting by negative. in C++, it is undefined behavior. */
  }

  /* test mul/div/rem, division/rem by zero is undefined */ {
    i64 b = 0xc0ffee;
    i64 a = 0xbad0beef;
    test_inplace_mul(a, b);
    test_inplace_mul(a, 0);
    test_inplace_mul(-a, b);

    test_inplace_div(a, b);
    test_inplace_rem(a, b);
  }

  /* logic ops: and/or/xor */ {
    i64 b = 0xc0ffee;
    i64 a = 0xbad0beef;
    test_inplace_and(a, b);
    test_inplace_or(a, b);
    test_inplace_xor(a, b);
  }
}

} /* namespace cppbind */

foreach_inplace(gen_test_for_inplace_op)
