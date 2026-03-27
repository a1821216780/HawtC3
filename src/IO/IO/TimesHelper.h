
//**********************************************************************************************************************************
//LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of Qahse.IO.
//
// Licensed under the Boost Software License - Version 1.0 - August 17th, 2003
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.HawtC.cn/licenses.txt
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//**********************************************************************************************************************************

#pragma once
#ifndef TIMESHELPER_H
#define TIMESHELPER_H

#include <string>
#include <chrono>
#include <tuple>
#include <iostream>

namespace Qahse::IO::IO {

	/// <summary>
	/// Provides utility methods for working with time and date-related operations.
	/// </summary>
	/// <remarks>This class includes methods for retrieving the current date and time components, formatting
	/// time, calculating elapsed and remaining time, and converting time units. It is designed to simplify common
	/// time-related tasks in applications.</remarks>
	class TimesHelper {
	public:
		/// <summary>
		/// Gets the current year based on the system's local date and time.
		/// </summary>
		/// <returns>The current year as a four-digit integer.</returns>
		static int GetCurrentYear();

		/// <summary>
		/// Gets the current month as an integer.
		/// </summary>
		/// <returns>An integer representing the current month, where 1 corresponds to January and 12 corresponds to December.</returns>
		static int GetCurrentMonth();

		/// <summary>
		/// Gets the current hour of the day based on the system's local time.
		/// </summary>
		/// <remarks>The returned value is derived from the system's local time and may vary depending on
		/// the machine's time zone and clock settings.</remarks>
		/// <returns>An integer representing the current hour in 24-hour format (0 to 23).</returns>
		static int GetCurrentHour();

		/// <summary>
		/// Retrieves the current minute of the hour based on the system's local time.
		/// </summary>
		/// <returns>The current minute as an integer, ranging from 0 to 59.</returns>
		static int GetCurrentMinute();

		/// <summary>
		/// Returns the current date and time as a formatted string based on the specified format.
		/// </summary>
		/// <remarks>The method supports two primary formats: <list type="bullet"> <item> <description>"E"
		/// (default): Outputs the date and time in the format "yyyy-MM-dd-HH-mm".</description> </item> <item>
		/// <description>"C", "ch", or "CH": Outputs the date and time in the format "yyyy年-MM月-dd日-mm分".</description>
		/// </item> </list> If the provided format is null, empty, or does not match any of the supported formats, the
		/// default "E" format is used.</remarks>
		/// <param name="format">An optional format specifier that determines the output format of the date and time. Use "E" (default) for
		/// "yyyy-MM-dd-HH-mm" format, or "C" (or "ch"/"CH") for "yyyy年-MM月-dd日-mm分" format. If the format is null,
		/// empty, or unrecognized, the default "E" format is used.</param>
		/// <returns>A string representing the current date and time in the specified format.</returns>
		static std::string GetTime(const std::string& format = "E");

		/// <summary>
		/// Calculates the remaining computation time for a simulation based on the elapsed time,  current progress, and
		/// the start time of the simulation.
		/// </summary>
		/// <remarks>This method estimates the remaining computation time by extrapolating the progress
		/// made so far.  It updates the console output with the elapsed and remaining time during the
		/// simulation.</remarks>
		/// <param name="elapsedTime">The total elapsed time of the simulation in seconds.</param>
		/// <param name="currentIndex">The current iteration index of the simulation.</param>
		/// <param name="totalIterations">The total number of iterations in the simulation.</param>
		/// <param name="startTime">The std::chrono::steady_clock::time_point when the simulation started.</param>
		/// <param name="lastElapsedTime">The previously recorded elapsed time in seconds.</param>
		/// <param name="lastRemainingTime">The previously calculated remaining time in seconds.</param>
		/// <returns>A tuple containing two integers: <list type="bullet"> <item><description>The updated elapsed time in
		/// seconds.</description></item> <item><description>The updated remaining time in seconds.</description></item>
		/// </list></returns>
		static std::tuple<int, int> CalculateRemainingTime(double elapsedTime, int currentIndex, int totalIterations,
			const std::chrono::steady_clock::time_point& startTime,
			int lastElapsedTime, int lastRemainingTime);

		/// <summary>
		/// Adjusts the console cursor position and clears a character if the cursor is not at the start of a line.
		/// </summary>
		/// <remarks>If the console cursor is not at the beginning of a line, this method moves the cursor
		/// one position to the left and replaces the character at that position with a space.</remarks>
		/// <param name="a">An optional parameter that is currently unused. Defaults to 1.</param>
		static void dela(int a = 1);

		/// <summary>
		/// Resets and starts the internal stopwatch for measuring elapsed time.
		/// </summary>
		/// <remarks>This method restarts the stopwatch, clearing any previously recorded elapsed time. It
		/// is typically used in conjunction with a corresponding method to retrieve the elapsed time.</remarks>
		static void Tic();

		/// <summary>
		/// Converts a duration in milliseconds to a human-readable time format.
		/// </summary>
		/// <remarks>This method is useful for displaying time durations in a user-friendly format.  The
		/// output is always formatted with two-digit components for consistency.</remarks>
		/// <param name="milliseconds">The duration in milliseconds to be converted. Must be a non-negative value.</param>
		/// <param name="form">An optional format specifier. Currently unused and has no effect on the output.</param>
		/// <returns>A string representing the duration in a human-readable format.  The format will include hours, minutes, and
		/// seconds as applicable: - "HH hours MM mins SS secs" if the duration includes hours. - "MM mins SS secs" if
		/// the duration includes minutes but no hours. - "SS secs" if the duration is less than one minute.</returns>
		static std::string MillisecondsToTime(double milliseconds, const std::string& form = "C");

		/// <summary>
		/// Measures the elapsed time since the stopwatch was started and formats it as a string based on the specified
		/// unit.
		/// </summary>
		/// <remarks>The method stops the stopwatch, calculates the elapsed time, and formats it based on
		/// the specified unit.  If <paramref name="show"/> is <see langword="true"/>, the formatted time is logged or
		/// displayed.</remarks>
		/// <param name="show">A value indicating whether the formatted time should be logged or displayed.  If true, the
		/// time is logged or written to the console; otherwise, it is only returned as a string.</param>
		/// <param name="换行吗">A value indicating whether the log output should include a newline.  If true, the log
		/// includes a newline; otherwise, it does not.</param>
		/// <param name="Control">A string specifying the unit of time to use for formatting the elapsed time.  Valid values are: <list
		/// type="bullet"> <item><description>"ms" for milliseconds.</description></item>
		/// <item><description>"s" for seconds (default).</description></item> <item><description>"min"
		/// for minutes.</description></item> <item><description>"h" for hours.</description></item> </list></param>
		/// <param name="level">The indentation level for the log output. This determines the number of leading spaces in the log message.</param>
		/// <returns>A string representing the elapsed time formatted in the specified unit.</returns>
		static std::string Toc(bool show = true, bool 换行吗 = true, const std::string& Control = "s", int level = 0);

	private:
		/// <summary>
		/// Internal stopwatch for measuring elapsed time
		/// </summary>
		static std::chrono::steady_clock::time_point stopwatch_start;
		static bool stopwatch_running;

		/// <summary>
		/// Helper method to convert string to lowercase for comparison
		/// </summary>
		/// <param name="str">Input string to convert</param>
		/// <returns>Lowercase version of the input string</returns>
		static std::string ToLower(const std::string& str);

		/// <summary>
		/// Helper method to format integer with leading zeros
		/// </summary>
		/// <param name="value">Integer value to format</param>
		/// <param name="width">Minimum width of the output string</param>
		/// <returns>Formatted string with leading zeros if necessary</returns>
		static std::string FormatWithLeadingZeros(int value, int width);
	};

} // namespace Qahse::IO::Math

#endif // TIMESHELPER_H