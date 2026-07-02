#include <gtest/gtest.h>

#include "util/string_util.h"

namespace strings {

TEST(StringsTest, StartsWith) {
  EXPECT_TRUE(startsWith("abc", ""));
  EXPECT_TRUE(startsWith("abc", "a"));
  EXPECT_TRUE(startsWith("abc", "ab"));
  EXPECT_TRUE(startsWith("abc", "abc"));

  EXPECT_TRUE(startsWith("", ""));
  EXPECT_TRUE(startsWith("a", "a"));
  EXPECT_TRUE(startsWith("ab", "ab"));

  EXPECT_FALSE(startsWith("", "a"));
  EXPECT_FALSE(startsWith("a", "ab"));
}

TEST(StringsTest, EndsWith) {
  EXPECT_TRUE(endsWith("abc", ""));
  EXPECT_TRUE(endsWith("abc", "c"));
  EXPECT_TRUE(endsWith("abc", "bc"));
  EXPECT_TRUE(endsWith("abc", "abc"));

  EXPECT_TRUE(endsWith("", ""));
  EXPECT_TRUE(endsWith("a", "a"));
  EXPECT_TRUE(endsWith("ab", "ab"));

  EXPECT_FALSE(endsWith("", "a"));
  EXPECT_FALSE(endsWith("b", "ab"));
}

TEST(StringsTest, AsVectorOfStrings_Basic) {
  const char *Argv[] = {"apple", "banana", "cherry"};
  int Argc = 3;
  auto Vec = asVectorOfStrings(Argc, const_cast<char **>(Argv));
  ASSERT_EQ(Vec.size(), 3);
  EXPECT_EQ(Vec[0], "apple");
  EXPECT_EQ(Vec[1], "banana");
  EXPECT_EQ(Vec[2], "cherry");
}

TEST(StringsTest, AsVectorOfStrings_Empty) {
  int Argc = 0;
  char **Argv = nullptr;
  auto Vec = asVectorOfStrings(Argc, Argv);
  ASSERT_TRUE(Vec.empty());
}

TEST(StringsTest, AsVectorOfStrings_Single) {
  const char *Argv[] = {"foo"};
  int Argc = 1;
  auto Vec = asVectorOfStrings(Argc, const_cast<char **>(Argv));
  ASSERT_EQ(Vec.size(), 1);
  EXPECT_EQ(Vec[0], "foo");
}

TEST(StringsTest, AsVectorOfStrings_WithEmptyStrings) {
  const char *Argv[] = {"", "nonempty", ""};
  int Argc = 3;
  auto Vec = asVectorOfStrings(Argc, const_cast<char **>(Argv));
  ASSERT_EQ(Vec.size(), 3);
  EXPECT_EQ(Vec[0], "");
  EXPECT_EQ(Vec[1], "nonempty");
  EXPECT_EQ(Vec[2], "");
}

TEST(StringsTest, TrimWhitespace) {
  std::string Str = " \t\n\r\v ";
  EXPECT_EQ(trimWhitespace(Str), "");

  Str = "  \t\t\r\r\v\v\n\n  this string\thas\vwhitespace  \t\t\r\r\v\v\n\n  ";
  EXPECT_EQ(trimWhitespace(Str), "this string\thas\vwhitespace");

  Str = "abcd";
  EXPECT_EQ(trimWhitespace(Str), Str);
}

} // namespace strings
