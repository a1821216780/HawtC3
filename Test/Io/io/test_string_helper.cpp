// test_string_helper.cpp - StringHelper 类的单元测试
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../../../src/IO/IO/StringHelper.h"

using namespace HawtC3::IO::IO;

// ============================================================================
// ToUpper
// ============================================================================

TEST(StringHelperTest, ToUpper_Normal) {
	EXPECT_EQ(StringHelper::ToUpper("hello"), "HELLO");
	EXPECT_EQ(StringHelper::ToUpper("Hello World"), "HELLO WORLD");
}

TEST(StringHelperTest, ToUpper_AlreadyUpper) {
	EXPECT_EQ(StringHelper::ToUpper("ABC"), "ABC");
}

TEST(StringHelperTest, ToUpper_Empty) {
	EXPECT_EQ(StringHelper::ToUpper(""), "");
}

TEST(StringHelperTest, ToUpper_MixedWithNumbers) {
	EXPECT_EQ(StringHelper::ToUpper("abc123def"), "ABC123DEF");
}

TEST(StringHelperTest, ToUpper_SpecialChars) {
	EXPECT_EQ(StringHelper::ToUpper("a-b_c.d"), "A-B_C.D");
}


// ============================================================================
// ToLower
// ============================================================================

TEST(StringHelperTest, ToLower_Normal) {
	EXPECT_EQ(StringHelper::ToLower("HELLO"), "hello");
	EXPECT_EQ(StringHelper::ToLower("Hello World"), "hello world");
}

TEST(StringHelperTest, ToLower_AlreadyLower) {
	EXPECT_EQ(StringHelper::ToLower("abc"), "abc");
}

TEST(StringHelperTest, ToLower_Empty) {
	EXPECT_EQ(StringHelper::ToLower(""), "");
}

TEST(StringHelperTest, ToLower_MixedWithNumbers) {
	EXPECT_EQ(StringHelper::ToLower("ABC123DEF"), "abc123def");
}


// ============================================================================
// IsPalindrome
// ============================================================================

TEST(StringHelperTest, IsPalindrome_True) {
	EXPECT_TRUE(StringHelper::IsPalindrome("aba"));
	EXPECT_TRUE(StringHelper::IsPalindrome("abba"));
	EXPECT_TRUE(StringHelper::IsPalindrome("racecar"));
	EXPECT_TRUE(StringHelper::IsPalindrome("a"));
}

TEST(StringHelperTest, IsPalindrome_False) {
	EXPECT_FALSE(StringHelper::IsPalindrome("abc"));
	EXPECT_FALSE(StringHelper::IsPalindrome("hello"));
}

TEST(StringHelperTest, IsPalindrome_Empty) {
	EXPECT_TRUE(StringHelper::IsPalindrome(""));
}


// ============================================================================
// Split
// ============================================================================

TEST(StringHelperTest, Split_Normal) {
	auto result = StringHelper::Split("a,b,c", ',');
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

TEST(StringHelperTest, Split_SingleElement) {
	auto result = StringHelper::Split("hello", ',');
	ASSERT_EQ(result.size(), 1);
	EXPECT_EQ(result[0], "hello");
}

TEST(StringHelperTest, Split_Empty) {
	auto result = StringHelper::Split("", ',');
	ASSERT_EQ(result.size(), 0);
}

TEST(StringHelperTest, Split_TabDelimiter) {
	auto result = StringHelper::Split("a\tb\tc", '\t');
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

TEST(StringHelperTest, Split_SpaceDelimiter) {
	auto result = StringHelper::Split("hello world foo", ' ');
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result[0], "hello");
	EXPECT_EQ(result[1], "world");
	EXPECT_EQ(result[2], "foo");
}

TEST(StringHelperTest, Split_TrailingDelimiter) {
	auto result = StringHelper::Split("a,b,", ',');
	ASSERT_EQ(result.size(), 2);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
}


// ============================================================================
// StringToBool
// ============================================================================

TEST(StringHelperTest, StringToBool_True) {
	EXPECT_TRUE(StringHelper::StringToBool("1"));
	EXPECT_TRUE(StringHelper::StringToBool("42"));
	EXPECT_TRUE(StringHelper::StringToBool("-1"));
}

TEST(StringHelperTest, StringToBool_False) {
	EXPECT_FALSE(StringHelper::StringToBool("0"));
}
