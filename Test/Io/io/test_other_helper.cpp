// test_other_helper.cpp - OtherHelper 类的单元测试
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <ctime>
#include "../../../src/IO/IO/OtherHelper.h"

using namespace Qahse::IO::IO;
namespace fs = std::filesystem;

// ============================================================================
// FormortPath
// ============================================================================

TEST(OtherHelperTest, FormortPath_ReturnsAbsolute) {
	std::string result = OtherHelper::FormortPath("test.txt");
	EXPECT_TRUE(fs::path(result).is_absolute());
}

TEST(OtherHelperTest, FormortPath_AbsoluteUnchanged) {
	std::string input = "E:\\Qahse\\src\\main.cpp";
	std::string result = OtherHelper::FormortPath(input);
	EXPECT_TRUE(fs::path(result).is_absolute());
}


// ============================================================================
// FindBestMatch
// ============================================================================

TEST(OtherHelperTest, FindBestMatch_ExactMatch) {
	std::vector<std::string> strings = {"apple", "banana", "cherry"};
	auto [index, match] = OtherHelper::FindBestMatch(strings, "banana");
	EXPECT_EQ(index, 1);
	EXPECT_EQ(match, "banana");
}

TEST(OtherHelperTest, FindBestMatch_ClosestMatch) {
	std::vector<std::string> strings = {"apple", "banana", "cherry"};
	auto [index, match] = OtherHelper::FindBestMatch(strings, "banan");
	EXPECT_EQ(index, 1);
	EXPECT_EQ(match, "banana");
}

TEST(OtherHelperTest, FindBestMatch_EmptyList) {
	std::vector<std::string> strings = {};
	auto [index, match] = OtherHelper::FindBestMatch(strings, "test");
	EXPECT_EQ(index, -1);
	EXPECT_EQ(match, "");
}


// ============================================================================
// GetCurrentProjectName / GetCurrentExeName
// ============================================================================

TEST(OtherHelperTest, GetCurrentProjectName) {
	EXPECT_EQ(OtherHelper::GetCurrentProjectName(), "Qahse");
}

TEST(OtherHelperTest, GetCurrentExeName) {
	EXPECT_EQ(OtherHelper::GetCurrentExeName(), "Qahse.exe");
}


// ============================================================================
// GetCurrentBuildMode
// ============================================================================

TEST(OtherHelperTest, GetCurrentBuildMode) {
	std::string mode = OtherHelper::GetCurrentBuildMode();
	EXPECT_TRUE(mode == "Debug" || mode == "Release");
}


// ============================================================================
// GetBuildMode
// ============================================================================

TEST(OtherHelperTest, GetBuildMode) {
	std::string mode = OtherHelper::GetBuildMode();
	EXPECT_TRUE(mode == "_x64" || mode == "_x32");
}


// ============================================================================
// GetThreadCount
// ============================================================================

TEST(OtherHelperTest, GetThreadCount_Positive) {
	int count = OtherHelper::GetThreadCount();
	EXPECT_GT(count, 0);
}


// ============================================================================
// GetFileExtension
// ============================================================================

TEST(OtherHelperTest, GetFileExtension_Normal) {
	EXPECT_EQ(OtherHelper::GetFileExtension("test.txt"), ".txt");
	EXPECT_EQ(OtherHelper::GetFileExtension("file.cpp"), ".cpp");
	EXPECT_EQ(OtherHelper::GetFileExtension("archive.tar.gz"), ".gz");
}

TEST(OtherHelperTest, GetFileExtension_NoExtension) {
	EXPECT_EQ(OtherHelper::GetFileExtension("noext"), "");
}

TEST(OtherHelperTest, GetFileExtension_ToLower) {
	EXPECT_EQ(OtherHelper::GetFileExtension("FILE.CPP"), ".cpp");
	EXPECT_EQ(OtherHelper::GetFileExtension("DATA.TXT"), ".txt");
}


// ============================================================================
// FillString
// ============================================================================

TEST(OtherHelperTest, FillString_Normal) {
	std::string result = OtherHelper::FillString("abc", "-", 1);
	EXPECT_EQ(result, "a-b-c");
}

TEST(OtherHelperTest, FillString_MultipleRepeat) {
	std::string result = OtherHelper::FillString("ab", "--", 2);
	EXPECT_EQ(result, "a----b");
}

TEST(OtherHelperTest, FillString_Empty) {
	EXPECT_EQ(OtherHelper::FillString("", "-", 1), "");
}

TEST(OtherHelperTest, FillString_SingleChar) {
	EXPECT_EQ(OtherHelper::FillString("a", "-", 1), "a");
}


// ============================================================================
// RandomString
// ============================================================================

TEST(OtherHelperTest, RandomString_CorrectLength) {
	std::string result = OtherHelper::RandomString(10);
	EXPECT_EQ(result.length(), 10);
}

TEST(OtherHelperTest, RandomString_OnlyLetters) {
	std::string result = OtherHelper::RandomString(100);
	for (char c : result) {
		EXPECT_TRUE(std::isalpha(c));
	}
}

TEST(OtherHelperTest, RandomString_ZeroLength) {
	std::string result = OtherHelper::RandomString(0);
	EXPECT_TRUE(result.empty());
}


// ============================================================================
// CenterText
// ============================================================================

TEST(OtherHelperTest, CenterText_Normal) {
	std::string result = OtherHelper::CenterText("Hi", 10, '-');
	EXPECT_EQ(result.length(), 10);
	EXPECT_EQ(result, "----Hi----");
}

TEST(OtherHelperTest, CenterText_OddPadding) {
	std::string result = OtherHelper::CenterText("Hi", 9, '-');
	EXPECT_EQ(result.length(), 9);
	// leftPadding=3, rightPadding=4
	EXPECT_EQ(result, "---Hi----");
}

TEST(OtherHelperTest, CenterText_LongerThanWidth) {
	std::string result = OtherHelper::CenterText("Hello World", 5, '-');
	EXPECT_EQ(result, "Hello World");
}

TEST(OtherHelperTest, CenterText_ExactWidth) {
	std::string result = OtherHelper::CenterText("Hello", 5, '-');
	EXPECT_EQ(result, "Hello");
}


// ============================================================================
// GetSafeLocalTime
// ============================================================================

TEST(OtherHelperTest, GetSafeLocalTime_Valid) {
	std::time_t now = std::time(nullptr);
	std::tm tm = OtherHelper::GetSafeLocalTime(now);
	EXPECT_GE(tm.tm_year + 1900, 2020);
	EXPECT_LE(tm.tm_year + 1900, 2100);
	EXPECT_GE(tm.tm_mon, 0);
	EXPECT_LE(tm.tm_mon, 11);
	EXPECT_GE(tm.tm_mday, 1);
	EXPECT_LE(tm.tm_mday, 31);
}


// ============================================================================
// GetCurrentYear / GetCurrentMonth / GetCurrentDay / GetCurrentHour ...
// ============================================================================

TEST(OtherHelperTest, GetCurrentYear_Int) {
	int year = OtherHelper::GetCurrentYear(true);
	EXPECT_GE(year, 2024);
	EXPECT_LE(year, 2100);
}

TEST(OtherHelperTest, GetCurrentYear_String) {
	std::string year = OtherHelper::GetCurrentYear();
	EXPECT_FALSE(year.empty());
	int y = std::stoi(year);
	EXPECT_GE(y, 2024);
}

TEST(OtherHelperTest, GetCurrentMonth_Int) {
	int month = OtherHelper::GetCurrentMonth(true);
	EXPECT_GE(month, 1);
	EXPECT_LE(month, 12);
}

TEST(OtherHelperTest, GetCurrentMonth_String) {
	std::string month = OtherHelper::GetCurrentMonth();
	EXPECT_FALSE(month.empty());
	int m = std::stoi(month);
	EXPECT_GE(m, 1);
	EXPECT_LE(m, 12);
}


// ============================================================================
// GetMatchingLineIndexes
// ============================================================================

TEST(OtherHelperTest, GetMatchingLineIndexes_Found) {
	std::vector<std::string> input = {"hello world", "foo bar", "hello again"};
	auto result = OtherHelper::GetMatchingLineIndexes(input, "hello", "test.txt", false, false);
	ASSERT_EQ(result.size(), 2);
	EXPECT_EQ(result[0], 0);
	EXPECT_EQ(result[1], 2);
}

TEST(OtherHelperTest, GetMatchingLineIndexes_NotFound) {
	std::vector<std::string> input = {"hello world", "foo bar"};
	auto result = OtherHelper::GetMatchingLineIndexes(input, "xyz", "test.txt", false, false);
	ASSERT_EQ(result.size(), 1);
	EXPECT_EQ(result[0], -1);
}

TEST(OtherHelperTest, GetMatchingLineIndexes_StringContains) {
	EXPECT_TRUE(OtherHelper::GetMatchingLineIndexes("hello world", "hello"));
	EXPECT_FALSE(OtherHelper::GetMatchingLineIndexes("hello world", "xyz"));
}


// ============================================================================
// ReadOutputWord
// ============================================================================

TEST(OtherHelperTest, ReadOutputWord_Normal) {
	std::vector<std::string> data = {"header", "word1", "word2", "END", "ignored"};
	auto result = OtherHelper::ReadOutputWord(data, 1, false);
	ASSERT_EQ(result.size(), 2);
	EXPECT_EQ(result[0], "word1");
	EXPECT_EQ(result[1], "word2");
}

TEST(OtherHelperTest, ReadOutputWord_DeleteSame) {
	std::vector<std::string> data = {"header", "aaa", "bbb", "aaa", "END"};
	auto result = OtherHelper::ReadOutputWord(data, 1, true);
	ASSERT_EQ(result.size(), 2);
}


// ============================================================================
// FindFilesWithExtension
// ============================================================================

TEST(OtherHelperTest, FindFilesWithExtension_NonExistentDir) {
	auto result = OtherHelper::FindFilesWithExtension("C:\\NonExistentDir12345", ".txt");
	EXPECT_TRUE(result.empty());
}
