#include <cppbind.h>
#include <gtest/gtest.h>

using cppbind::Dict;
using cppbind::Long;
using cppbind::Object;

/**
 * The parameter `key` and `value` are not easily swappable.
 */
/* NOLINTBEGIN(bugprone-easily-swappable-parameters) */

static void dput(Dict &dict, long int key, long int value) {
  Object key_obj = Long(key).object();
  Object value_obj = Long(value).object();

  dict[key_obj] = value_obj;
}

static bool dget(Dict &dict, long int key, long int *value_ptr) {
  Object key_obj = Long(key).object();
  Object value_obj = dict[key_obj];
  if (value_obj.ptr == Py_None) {
    return false;
  }
  *value_ptr = static_cast<long int>(Long(value_obj));
  return true;
}
/* NOLINTEND(bugprone-easily-swappable-parameters) */

namespace cppbind {
/* NOLINTBEGIN(readability-magic-numbers,readability-function-cognitive-complexity)
 */

TEST(Dict, GetAndSet) {
  Dict dict;
  long int key = 0x1000;
  for (long int value = 0x1000; value < 0x1010; value++) {
    dput(dict, key, value);
    long int got;
    EXPECT_TRUE(dget(dict, key, &got));
    EXPECT_EQ(got, value);
  }
  EXPECT_TRUE(!dget(dict, 0x0, &key));
}

TEST(Dict, ValueLifeTime) {
  Dict dict;
  const long integer_key = 0x12345678;
  Object key_obj = Long(integer_key).object();
  const auto krefcnt = key_obj.ref_count();
  Py_ssize_t vrefcnt;
  {
    const long int integer_value = integer_key + 1;
    Object value = Long(integer_value).object();
    vrefcnt = value.ref_count();
    dict.__setitem__(key_obj, value);
    ASSERT_EQ(value.ref_count(), vrefcnt + 1);
    ASSERT_EQ(key_obj.ref_count(), krefcnt + 1);
  }
  dict.clear();
  ASSERT_EQ(key_obj.ref_count(), krefcnt);
  ASSERT_EQ(vrefcnt, vrefcnt);
}

TEST(Dict, DictLifeTime) {
  const long int integer_key = 0x12345678; /* magic */
  const long int integer_value = integer_key ^ 0xc0ffee;

  Object value = Long(integer_value).object();
  const auto vrefcnt = value.ref_count();
  Object key = Long(integer_key).object();
  const auto krefcnt = key.ref_count();
  {
    Dict dict;
    dict.__setitem__(key, value);
    ASSERT_EQ(value.ref_count(), vrefcnt + 1);
    ASSERT_EQ(key.ref_count(), krefcnt + 1);
  }
  ASSERT_EQ(value.ref_count(), vrefcnt);
  ASSERT_EQ(key.ref_count(), krefcnt);

  /* Try put the same kv twice. */ {
    Dict dict;
    dict.__setitem__(key, value);
    dict.__setitem__(key, value);
    ASSERT_EQ(value.ref_count(), vrefcnt + 1);
    ASSERT_EQ(key.ref_count(), krefcnt + 1);
  }
  ASSERT_EQ(value.ref_count(), vrefcnt);
  ASSERT_EQ(key.ref_count(), krefcnt);

  /* Try put the same kv into two dicts. */ do {
    Dict dict1;
    Dict dict2;
    if (dict1.object().ptr == dict2.object().ptr) {
      FAIL() << "Two dicts should not share the same underlying PyDictObject.";
    }
    dict1.__setitem__(key, value);
    dict2.__setitem__(key, value);
    ASSERT_EQ(value.ref_count(), vrefcnt + 2);
    ASSERT_EQ(key.ref_count(), krefcnt + 2);
  } while (false);
  ASSERT_EQ(value.ref_count(), vrefcnt);
  ASSERT_EQ(key.ref_count(), krefcnt);
}

TEST(Dict, KeyIsValue) {
  Object obj = Long(0x12345678).object();
  const auto refcnt = obj.ref_count();
  {
    Dict dict;
    dict.__setitem__(obj, obj);
    ASSERT_EQ(obj.ref_count(), refcnt + 2);
  }
  ASSERT_EQ(obj.ref_count(), refcnt + 0);
}

/* NOLINTEND(readability-magic-numbers,readability-function-cognitive-complexity)
 */
} /* namespace cppbind */
