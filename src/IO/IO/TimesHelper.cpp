#pragma once  
//**********************************************************************************************************************************
//LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of HawtC3.IO.
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

#include "TimesHelper.h"
#include "../IO/OtherHelper.h"
#include "../Log/LogHelper.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
//#include <windows.h>  
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace HawtC3::IO::IO {

// Static member initialization
std::chrono::steady_clock::time_point TimesHelper::stopwatch_start;
bool TimesHelper::stopwatch_running = false;

/// <summary>
/// Gets the current year based on the system's local date and time.
/// </summary>
/// <returns>The current year as a four-digit integer.</returns>
int TimesHelper::GetCurrentYear() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = OtherHelper::GetSafeLocalTime(time_t);
    return tm.tm_year + 1900;
}

/// <summary>
/// Gets the current month as an integer.
/// </summary>
/// <returns>An integer representing the current month, where 1 corresponds to January and 12 corresponds to December.</returns>
int TimesHelper::GetCurrentMonth() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = OtherHelper::GetSafeLocalTime(time_t);
    return tm.tm_mon + 1;
}

/// <summary>
/// Gets the current hour of the day based on the system's local time.
/// </summary>
/// <remarks>The returned value is derived from the system's local time and may vary depending on
/// the machine's time zone and clock settings.</remarks>
/// <returns>An integer representing the current hour in 24-hour format (0 to 23).</returns>
int TimesHelper::GetCurrentHour() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = OtherHelper::GetSafeLocalTime(time_t);
    return tm.tm_hour;
}

/// <summary>
/// Retrieves the current minute of the hour based on the system's local time.
/// </summary>
/// <returns>The current minute as an integer, ranging from 0 to 59.</returns>
int TimesHelper::GetCurrentMinute() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = OtherHelper::GetSafeLocalTime(time_t);
    return tm.tm_min;
}

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
std::string TimesHelper::GetTime(const std::string& format) {
    auto now = std::chrono::system_clock::now();
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = OtherHelper::GetSafeLocalTime(time_t);
    
    std::string lowerFormat = ToLower(format);
    
    if (format.empty() || lowerFormat == "e") {
        std::stringstream ss;
        ss << std::setfill('0') 
           << std::setw(4) << (tm.tm_year + 1900) << "-"
           << std::setw(2) << (tm.tm_mon + 1) << "-"
           << std::setw(2) << tm.tm_mday << "-"
           << std::setw(2) << tm.tm_hour << "-"
           << std::setw(2) << tm.tm_min;
        return ss.str();
    }
    else if (lowerFormat == "c" || lowerFormat == "ch") {
        std::stringstream ss;
        ss << std::setfill('0') 
           << std::setw(4) << (tm.tm_year + 1900) << "年-"
           << std::setw(2) << (tm.tm_mon + 1) << "月-"
           << std::setw(2) << tm.tm_mday << "日-"
           << std::setw(2) << tm.tm_min << "分";
        return ss.str();
    }
    
    // Default format
    std::stringstream ss;
    ss << std::setfill('0') 
       << std::setw(4) << (tm.tm_year + 1900) << "年-"
       << std::setw(2) << (tm.tm_mon + 1) << "月-"
       << std::setw(2) << tm.tm_mday << "日-"
       << std::setw(2) << tm.tm_min << "分";
    return ss.str();
}

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
std::tuple<int, int> TimesHelper::CalculateRemainingTime(double elapsedTime, int currentIndex, int totalIterations, 
                                                        const std::chrono::steady_clock::time_point& startTime, 
                                                        int lastElapsedTime, int lastRemainingTime) {
    if (elapsedTime - lastElapsedTime > 0) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
        
        double term = (totalIterations - currentIndex) * duration.count() / static_cast<double>(currentIndex);
        double remainingTime = term / 60.0;
        
        if (lastElapsedTime == 0) {

            // Unix/Linux console manipulation
            std::cout << "\r";
            std::cout << "Simulation time elapsed: " << std::round(elapsedTime) 
                      << " secs. Computation time left: " << std::round(remainingTime) 
                      << " minutes.     ";
            std::cout.flush();
        }
        else {
            int roundedRemainingTime = static_cast<int>(std::round(remainingTime));
            if (roundedRemainingTime != 0) {

                std::cout << "\r";
                std::cout << "Simulation time elapsed: " << std::round(elapsedTime) 
                          << " secs. Computation time left: " << std::round(remainingTime) 
                          << " minutes.        ";
                std::cout.flush();
            }
            else {

                std::cout << "\r";
                std::cout << "Simulation time elapsed: " << std::round(elapsedTime) 
                          << " secs. Computation time left: " << std::round(remainingTime) 
                          << " seconds.           ";
                std::cout.flush();
                remainingTime = term;
            }
        }
        
        lastElapsedTime = static_cast<int>(std::round(elapsedTime));
        lastRemainingTime = static_cast<int>(std::round(remainingTime));
    }
    
    return std::make_tuple(lastElapsedTime, lastRemainingTime);
}

/// <summary>
/// Resets and starts the internal stopwatch for measuring elapsed time.
/// </summary>
/// <remarks>This method restarts the stopwatch, clearing any previously recorded elapsed time. It
/// is typically used in conjunction with a corresponding method to retrieve the elapsed time.</remarks>
void TimesHelper::Tic() {
    stopwatch_start = std::chrono::steady_clock::now();
    stopwatch_running = true;
}

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
std::string TimesHelper::MillisecondsToTime(double milliseconds, const std::string& form) {
    // 计算总共的秒数
    double totalSeconds = milliseconds / 1000.0;
    
    // 计算小时数、分钟数和秒数
    int hours = static_cast<int>(totalSeconds / 3600);
    int minutes = static_cast<int>((totalSeconds - hours * 3600) / 60);
    int seconds = static_cast<int>(totalSeconds - hours * 3600 - minutes * 60);
    
    // 返回格式化后的字符串
    if (hours > 0) {
        return FormatWithLeadingZeros(hours, 2) + " hours " + 
               FormatWithLeadingZeros(minutes, 2) + " mins " + 
               FormatWithLeadingZeros(seconds, 2) + " secs";
    }
    else if (minutes > 0) {
        return FormatWithLeadingZeros(minutes, 2) + " mins " + 
               FormatWithLeadingZeros(seconds, 2) + " secs";
    }
    else {
        return FormatWithLeadingZeros(seconds, 2) + " secs";
    }
}

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
std::string TimesHelper::Toc(bool show, bool 换行吗, const std::string& Control, int level) {
    if (!stopwatch_running) {
        return "0 s";
    }
    
    auto end = std::chrono::steady_clock::now();
    auto elapsed = end - stopwatch_start;
    stopwatch_running = false;
    
    std::string ms = "";
    for (int i = 0; i < level; i++) {
        ms += " ";
    }
    
    std::string control = ToLower(Control);
    std::string timeString = "";
    
    if (control == "ms") {
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << milliseconds.count() << " ms";
        timeString = ss.str();
    }
     else   if (control == "s") {
        auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << seconds.count() << " s";
        timeString = ss.str();
    }
    else if (control == "min") {
        auto minutes = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<60>>>(elapsed);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << minutes.count() << " min";
        timeString = ss.str();
    }
    else if (control == "h") {
        auto hours = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(elapsed);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << hours.count() << " h";
        timeString = ss.str();
    }
    else
    { // default to seconds
        HawtC3::IO::Log::LogHelper::ErrorLog("错误的时间参数!");
        auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << seconds.count() << " s";
        timeString = ss.str();
    }
    
    if (show) {
        if (换行吗)
        {
            HawtC3::IO::Log::LogHelper::WriteLog(timeString, "", true, "[Message]", 
                                               HawtC3::IO::System::ConsoleColor::White, true, level);
        }
        else
        {
            HawtC3::IO::Log::LogHelper::WriteLogO(timeString);
            std::cout << '\r';
            std::cout << ms << timeString;
        }
    }
    else
    {
        HawtC3::IO::Log::LogHelper::WriteLogO(timeString);
    }
    
    return timeString;
}

/// <summary>
/// Helper method to convert string to lowercase for comparison
/// </summary>
/// <param name="str">Input string to convert</param>
/// <returns>Lowercase version of the input string</returns>
std::string TimesHelper::ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

/// <summary>
/// Helper method to format integer with leading zeros
/// </summary>
/// <param name="value">Integer value to format</param>
/// <param name="width">Minimum width of the output string</param>
/// <returns>Formatted string with leading zeros if necessary</returns>
std::string TimesHelper::FormatWithLeadingZeros(int value, int width) {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(width) << value;
    return ss.str();
}

} // namespace HawtC3::IO::Math