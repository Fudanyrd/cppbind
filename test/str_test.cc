#include <cstring>

#include <cppbind.h>
#include <gtest/gtest.h>
#include <stl.h>

namespace cppbind {

TEST(Str, ConcatASCII) {
  constexpr char a[] = "abc";
  constexpr char b[] = "def";
  constexpr char result[] = "abcdef";

  Str str(a);
  ASSERT_EQ(strcmp(str.data(), a), 0);

  str += Str(b);
  const char *data = str.data();
  ASSERT_EQ(strcmp(data, result), 0) << data << " " << strlen(data);
}

TEST(Str, ConcatUnicode) {
  constexpr char a[] = "abc";
  constexpr char b[] = "\xf0\x9f\x98\x83";
  constexpr char result[] = "abc"
                            "\xf0\x9f\x98\x83";

  Str str(a);
  ASSERT_EQ(strcmp(str.data(), a), 0);

  str += Str(b);
  Str expected(result);
  ASSERT_TRUE(str == expected) << str.data() << " " << expected.data();
}

TEST(Str, ConcatUnicode2) {
  constexpr char a[] = "\xf0\x9f\x98\xad";
  constexpr char b[] = "\xf0\x9f\x98\x83";
  constexpr char result[] = "\xf0\x9f\x98\xad"
                            "\xf0\x9f\x98\x83";
  Str s1(a);
  Str s2(b);
  Str s3 = s1 + s2;

  Str expected(result);
  ASSERT_TRUE(s3 == expected);
}

TEST(Str, Hashable) {
  constexpr char emoji[] = "\xf0\x9f\x98\x83";
  constexpr char description[] = "laugh";

  Object k = Str(emoji).object();
  Object v = Str(description).object();

  Dict d;
  d[k] = v;

  Str actual = Str{d[k]};
  ASSERT_TRUE(actual == Str(description));
}

TEST(Str, SubStr) {
  constexpr char a[] = "abcde";
  Str str(a);
  Str sub = str.substr(1, 4);
  ASSERT_TRUE(sub == Str("bcd"));

  sub = str[2];
  ASSERT_TRUE(sub == Str("c"));
}

TEST(Str, Stringify) {
  constexpr char a[] = "\xf0\x9f\x98\x83"
                       "abc";

  Str py_str(a);
  std::string cpp_str(a);
  std::string actual = stringify(py_str);

  ASSERT_EQ(cpp_str, actual);
}

} // namespace cppbind
