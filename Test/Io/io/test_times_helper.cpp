// test_times_helper.cpp - TimesHelper 类的单元测试
#include <gtest/gtest.h>
#include <string>
#include <chrono>
#include <thread>
#include "../../../src/IO/IO/TimesHelper.h"

using namespace HawtC3::IO::IO;

// ============================================================================
// GetCurrentYear / GetCurrentMonth / GetCurrentHour / GetCurrentMinute
// ============================================================================

TEST(TimesHelperTest, GetCurrentYear) {
	int year = TimesHelper::GetCurrentYear();
	EXPECT_GE(year, 2024);
	EXPECT_LE(year, 2100);
}

TEST(TimesHelperTest, GetCurrentMonth) {
	int month = TimesHelper::GetCurrentMonth();
	EXPECT_GE(month, 1);
	EXPECT_LE(month, 12);
}

TEST(TimesHelperTest, GetCurrentHour) {
	int hour = TimesHelper::GetCurrentHour();
	EXPECT_GE(hour, 0);
	EXPECT_LE(hour, 23);
}

TEST(TimesHelperTest, GetCurrentMinute) {
	int minute = TimesHelper::GetCurrentMinute();
	EXPECT_GE(minute, 0);
	EXPECT_LE(minute, 59);
}


// ============================================================================
// GetTime
// ============================================================================

TEST(TimesHelperTest, GetTime_DefaultFormat) {
	std::string time = TimesHelper::GetTime();
	// 格式: yyyy-MM-dd-HH-mm
	EXPECT_FALSE(time.empty());
	EXPECT_NE(time.find('-'), std::string::npos);
}

TEST(TimesHelperTest, GetTime_EnglishFormat) {
	std::string time = TimesHelper::GetTime("E");
	EXPECT_FALSE(time.empty());
	// 应包含连字符分隔
	EXPECT_NE(time.find('-'), std::string::npos);
}

TEST(TimesHelperTest, GetTime_ChineseFormat) {
	std::string time = TimesHelper::GetTime("C");
	EXPECT_FALSE(time.empty());
}


// ============================================================================
// MillisecondsToTime
// ============================================================================

TEST(TimesHelperTest, MillisecondsToTime_Seconds) {
	std::string result = TimesHelper::MillisecondsToTime(5000);
	EXPECT_EQ(result, "05 secs");
}

TEST(TimesHelperTest, MillisecondsToTime_Minutes) {
	// 90 seconds = 1 min 30 secs
	std::string result = TimesHelper::MillisecondsToTime(90000);
	EXPECT_EQ(result, "01 mins 30 secs");
}

TEST(TimesHelperTest, MillisecondsToTime_Hours) {
	// 3661 seconds = 1 hour 1 min 1 sec
	std::string result = TimesHelper::MillisecondsToTime(3661000);
	EXPECT_EQ(result, "01 hours 01 mins 01 secs");
}

TEST(TimesHelperTest, MillisecondsToTime_Zero) {
	std::string result = TimesHelper::MillisecondsToTime(0);
	EXPECT_EQ(result, "00 secs");
}

TEST(TimesHelperTest, MillisecondsToTime_LargeValue) {
	// 2 hours 30 min 45 sec = (2*3600+30*60+45)*1000
	std::string result = TimesHelper::MillisecondsToTime(9045000);
	EXPECT_EQ(result, "02 hours 30 mins 45 secs");
}


// ============================================================================
// Tic / Toc
// ============================================================================

TEST(TimesHelperTest, TicToc_Measures) {
	TimesHelper::Tic();
	// 短暂延迟
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	std::string result = TimesHelper::Toc(false, false, "ms");
	EXPECT_FALSE(result.empty());
	EXPECT_NE(result.find("ms"), std::string::npos);
}

TEST(TimesHelperTest, TicToc_Seconds) {
	TimesHelper::Tic();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	std::string result = TimesHelper::Toc(false, false, "s");
	EXPECT_NE(result.find("s"), std::string::npos);
}

TEST(TimesHelperTest, Toc_WithoutTic) {
	// 没有调用 Tic 的情况下调用 Toc
	std::string result = TimesHelper::Toc(false, false, "s");
	EXPECT_EQ(result, "0 s");
}


// ============================================================================
// CalculateRemainingTime
// ============================================================================

TEST(TimesHelperTest, CalculateRemainingTime_Basic) {
	auto start = std::chrono::steady_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	auto [elapsed, remaining] = TimesHelper::CalculateRemainingTime(
		1.0, 50, 100, start, 0, 0);

	EXPECT_GE(elapsed, 0);
	EXPECT_GE(remaining, 0);
}
