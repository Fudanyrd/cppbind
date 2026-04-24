#include <cppbind.h>
#include <gtest/gtest.h>

using cppbind::Dict;
using cppbind::Long;
using cppbind::Object;

static void dput(Dict &d, long int k, long int v) {
  Object ko = Long(k).object();
  Object vo = Long(v).object();

  d[ko] = vo;
}

static bool dget(Dict &d, long int k, long int *v) {
  Object ko = Long(k).object();
  Object vo = d[ko];
  if (vo.ptr == Py_None) {
    return false;
  }
  *v = static_cast<long int>(Long(vo));
  return true;
}

namespace cppbind {

TEST(Dict, GetAndSet) {
  Dict d;
  long int k = 0x1000;
  for (long int v = 0x1000; v < 0x1010; v++) {
    dput(d, k, v);
    long int got;
    EXPECT_TRUE(dget(d, k, &got));
    EXPECT_EQ(got, v);
  }
  EXPECT_TRUE(!dget(d, 0x0, &k));
}

TEST(Dict, ValueLifeTime) {
  Dict d;
  const long ki = 0x12345678;
  Object k = Long(ki).object();
  const auto krefcnt = k.ref_count();
  Py_ssize_t vrefcnt;
  {
    const long int vi = ki + 1;
    Object v = Long(vi).object();
    vrefcnt = v.ref_count();
    d.__setitem__(k, v);
    ASSERT_EQ(v.ref_count(), vrefcnt + 1);
    ASSERT_EQ(k.ref_count(), krefcnt + 1);
  }
  d.clear();
  ASSERT_EQ(k.ref_count(), krefcnt);
  ASSERT_EQ(vrefcnt, vrefcnt);
}

TEST(Dict, DictLifeTime) {
  const long int ki = 0x12345678; /* magic */
  const long int vi = ki ^ 0xc0ffee;

  Object value = Long(vi).object();
  const auto vrefcnt = value.ref_count();
  Object k = Long(ki).object();
  const auto krefcnt = k.ref_count();
  {
    Dict d;
    d.__setitem__(k, value);
    ASSERT_EQ(value.ref_count(), vrefcnt + 1);
    ASSERT_EQ(k.ref_count(), krefcnt + 1);
  }
  ASSERT_EQ(value.ref_count(), vrefcnt);
  ASSERT_EQ(k.ref_count(), krefcnt);

  /* Try put the same kv twice. */ {
    Dict d;
    d.__setitem__(k, value);
    d.__setitem__(k, value);
    ASSERT_EQ(value.ref_count(), vrefcnt + 1);
    ASSERT_EQ(k.ref_count(), krefcnt + 1);
  }
  ASSERT_EQ(value.ref_count(), vrefcnt);
  ASSERT_EQ(k.ref_count(), krefcnt);

  /* Try put the same kv into two dicts. */ do {
    Dict d1;
    Dict d2;
    if (d1.object().ptr == d2.object().ptr) {
      FAIL() << "Two dicts should not share the same underlying PyDictObject.";
    }
    d1.__setitem__(k, value);
    d2.__setitem__(k, value);
    ASSERT_EQ(value.ref_count(), vrefcnt + 2);
    ASSERT_EQ(k.ref_count(), krefcnt + 2);
  } while (0);
  ASSERT_EQ(value.ref_count(), vrefcnt);
  ASSERT_EQ(k.ref_count(), krefcnt);
}

TEST(Dict, KeyIsValue) {
  Object obj = Long(0x12345678).object();
  const auto refcnt = obj.ref_count();
  {
    Dict d;
    d.__setitem__(obj, obj);
    ASSERT_EQ(obj.ref_count(), refcnt + 2);
  }
  ASSERT_EQ(obj.ref_count(), refcnt + 0);
}

} /* namespace cppbind */
