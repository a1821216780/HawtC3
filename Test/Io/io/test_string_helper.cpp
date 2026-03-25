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

TEST(StringHelperTest, Split_LeadingDelimiter) {
	auto result = StringHelper::Split(",a,b", ',');
	ASSERT_EQ(result.size(), 2);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
}

//测试std::vector<std::string> StringHelper::Split(const std::string& str, const char* delimiter)
TEST(StringHelperTest, Split_MultipleDelimiters) {
	auto result = StringHelper::Split("a,b;c", ",;");
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

// ============================================================================
// TRIM
// ============================================================================
		// static std::string Trim(const std::string& str);
		// static std::string TrimStart(const std::string& str);
		// static std::string TrimEnd(const std::string& str);

TEST(StringHelperTest, Trim)
{
	EXPECT_EQ(StringHelper::Trim("  hello  "), "hello");
	EXPECT_EQ(StringHelper::Trim("\t\n  hello world  \n\t"), "hello world");
	EXPECT_EQ(StringHelper::Trim("   "), "");
	EXPECT_EQ(StringHelper::Trim(""), "");

}

TEST(StringHelperTest, TrimStart)
{
	EXPECT_EQ(StringHelper::TrimStart("  hello  "), "hello  ");
	EXPECT_EQ(StringHelper::TrimStart("\t\n  hello world  \n\t"), "hello world  \n\t");
	EXPECT_EQ(StringHelper::TrimStart("   "), "");
	EXPECT_EQ(StringHelper::TrimStart(""), "");
}

TEST(StringHelperTest, TrimEnd)
{
	EXPECT_EQ(StringHelper::TrimEnd("  hello  "), "  hello");
	EXPECT_EQ(StringHelper::TrimEnd("\t\n  hello world  \n\t"), "\t\n  hello world");
	EXPECT_EQ(StringHelper::TrimEnd("   "), "");
	EXPECT_EQ(StringHelper::TrimEnd(""), "");
}



// ============================================================================
// Replace
// ============================================================================
		// static std::string Replace(const std::string& str, const std::string& oldstr, const std::string& newstr);
		// static std::string Replace(const std::string& str, const char& oldstr, const char& newstr);
		// static std::string Replace(const std::string& str, const char& oldstr, const std::string& newstr);
		// static std::string Replace(const std::string& str, const std::string& oldstr, const char& newstr);

TEST(StringHelperTest, Replace_Substring)
{
	EXPECT_EQ(StringHelper::Replace("hello world", "world", "there"), "hello there");
	EXPECT_EQ(StringHelper::Replace("abcabc", "abc", "x"), "xx");
	EXPECT_EQ(StringHelper::Replace("aaaaa", "aa", "b"), "bba");
	EXPECT_EQ(StringHelper::Replace("hello", "x", "y"), "hello");
}

TEST(StringHelperTest, Replace_CharToChar)
{
	EXPECT_EQ(StringHelper::Replace("hello world", 'o', 'x'), "hellx wxrld");
	EXPECT_EQ(StringHelper::Replace("abcabc", 'a', 'x'), "xbcxbc");
	EXPECT_EQ(StringHelper::Replace("aaaaa", 'a', 'b'), "bbbbb");
	EXPECT_EQ(StringHelper::Replace("hello", 'x', 'y'), "hello");
}

TEST(StringHelperTest, Replace_CharToString)
{
	EXPECT_EQ(StringHelper::Replace("hello world", 'o', "oo"), "helloo woorld");
	EXPECT_EQ(StringHelper::Replace("abcabc", 'a', "x"), "xbcxbc");
	EXPECT_EQ(StringHelper::Replace("aaaaa", 'a', "b"), "bbbbb");
	EXPECT_EQ(StringHelper::Replace("hello", 'x', "y"), "hello");
}

TEST(StringHelperTest, Replace_StringToChar)
{
	EXPECT_EQ(StringHelper::Replace("hello world", "world", 'x'), "hello x");
	EXPECT_EQ(StringHelper::Replace("abcabc", "abc", 'x'), "xx");
	EXPECT_EQ(StringHelper::Replace("aaaaa", "aa", 'b'), "bba");
	EXPECT_EQ(StringHelper::Replace("hello", "x", 'y'), "hello");
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

// ============================================================================
// 文本文件的操作
// ============================================================================

TEST(StringHelperTest, ReadAllLines)
{
	std::vector<std::string> lines = StringHelper::ReadAllLines("./Test/IO/IO/ReadAllLines.txt");
	ASSERT_EQ(lines.size(), 6);
	EXPECT_EQ(lines[0], "1");
	EXPECT_EQ(lines[1], "2");
	EXPECT_EQ(lines[2], "3");
	EXPECT_EQ(lines[3], "4");
	EXPECT_EQ(lines[4], "  /n");
	EXPECT_EQ(lines[5], "  /t");
}
//异常测试
TEST(StringHelperTest, ReadAllLines_FileNotFound)
{
	EXPECT_THROW(StringHelper::ReadAllLines("./Test/IO/IO/NonExistentFile.txt"), std::runtime_error);
}

