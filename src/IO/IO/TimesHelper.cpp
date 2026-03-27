#pragma once
//**********************************************************************************************************************************
// LICENSING
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

#include "TimesHelper.h"
#include "../IO/OtherHelper.h"
#include "../Log/LogHelper.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
// #include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace Qahse::IO::IO
{

    // 静态成员变量初始化
    std::chrono::steady_clock::time_point TimesHelper::stopwatch_start; ///< 秒表起始时间点（调用 Tic() 时记录）
    bool TimesHelper::stopwatch_running = false;                        ///< 秒表运行状态标志，true 表示正在计时

    /**
     * @brief 获取系统当前年份（基于本地时区）。
     * @return 当前年份的四位整数，例如 2025。
     * @code
     * int year = TimesHelper::GetCurrentYear();
     * // year == 2025（当前年份）
     * @endcode
     */
    int TimesHelper::GetCurrentYear()
    {
        auto now = std::chrono::system_clock::now();                    ///< 当前系统时间点
        std::time_t time_t = std::chrono::system_clock::to_time_t(now); ///< 转换为 C 时间戳
        std::tm tm = OtherHelper::GetSafeLocalTime(time_t);             ///< 本地时间结构体（线程安全版本）
        return tm.tm_year + 1900;                                       // tm_year 为自 1900 年起的偏移量
    }

    /**
     * @brief 获取系统当前月份（基于本地时区）。
     * @return 当前月份的整数，1 对应一月，12 对应十二月。
     * @code
     * int month = TimesHelper::GetCurrentMonth();
     * // month 范围 [1, 12]
     * @endcode
     */
    int TimesHelper::GetCurrentMonth()
    {
        auto now = std::chrono::system_clock::now();                    ///< 当前系统时间点
        std::time_t time_t = std::chrono::system_clock::to_time_t(now); ///< 转换为 C 时间戳
        std::tm tm = OtherHelper::GetSafeLocalTime(time_t);             ///< 本地时间结构体（线程安全版本）
        return tm.tm_mon + 1;                                           // tm_mon 从 0 开始，故加 1
    }

    /**
     * @brief 获取系统当前小时（24 小时制，基于本地时区）。
     * @note 返回值依赖于机器的本地时区及时钟设置。
     * @return 当前小时的整数，范围 [0, 23]。
     * @code
     * int hour = TimesHelper::GetCurrentHour();
     * // hour 范围 [0, 23]
     * @endcode
     */
    int TimesHelper::GetCurrentHour()
    {
        auto now = std::chrono::system_clock::now();                    ///< 当前系统时间点
        std::time_t time_t = std::chrono::system_clock::to_time_t(now); ///< 转换为 C 时间戳
        std::tm tm = OtherHelper::GetSafeLocalTime(time_t);             ///< 本地时间结构体（线程安全版本）
        return tm.tm_hour;                                              // 24 小时制小时值
    }

    /**
     * @brief 获取系统当前分钟数（基于本地时区）。
     * @return 当前分钟的整数，范围 [0, 59]。
     * @code
     * int minute = TimesHelper::GetCurrentMinute();
     * // minute 范围 [0, 59]
     * @endcode
     */
    int TimesHelper::GetCurrentMinute()
    {
        auto now = std::chrono::system_clock::now();                    ///< 当前系统时间点
        std::time_t time_t = std::chrono::system_clock::to_time_t(now); ///< 转换为 C 时间戳
        std::tm tm = OtherHelper::GetSafeLocalTime(time_t);             ///< 本地时间结构体（线程安全版本）
        return tm.tm_min;                                               // 分钟值 [0, 59]
    }

    /**
     * @brief 获取格式化的当前日期时间字符串。
     * @details 支持以下格式：
     *  - \"E\"（默认）：输出格式为 \"yyyy-MM-dd-HH-mm\"，例如 \"2025-06-01-14-30\"。
     *  - \"C\"、\"ch\" 或 \"CH\"：输出格式为 \"yyyy年-MM月-dd日-mm分\"，例如 \"2025年-06月-01日-30分\"。
     *  - 若 format 为空字符串或不可识，则使用默认 \"E\" 格式。
     * @param format 格式说明符，支持 \"E\"（英文日期格式）或 \"C\"/\"ch\"（中文日期格式）。
     * @return 当前日期时间的格式化字符串。
     * @code
     * std::string t1 = TimesHelper::GetTime("E");
     * // t1 == "2025-06-01-14-30"
     * std::string t2 = TimesHelper::GetTime("C");
     * // t2 == "2025年-06月-01日-30分"
     * @endcode
     */
    std::string TimesHelper::GetTime(const std::string &format)
    {
        auto now = std::chrono::system_clock::now();                    ///< 当前系统时间点
        std::time_t time_t = std::chrono::system_clock::to_time_t(now); ///< 转换为 C 时间戳
        std::tm tm = OtherHelper::GetSafeLocalTime(time_t);             ///< 本地时间结构体（线程安全版本）

        std::string lowerFormat = ToLower(format); ///< 格式参数的小写版本，用于大小写不敏感比较

        if (format.empty() || lowerFormat == "e")
        {
            std::stringstream ss;
            ss << std::setfill('0')
               << std::setw(4) << (tm.tm_year + 1900) << "-"
               << std::setw(2) << (tm.tm_mon + 1) << "-"
               << std::setw(2) << tm.tm_mday << "-"
               << std::setw(2) << tm.tm_hour << "-"
               << std::setw(2) << tm.tm_min;
            return ss.str();
        }
        else if (lowerFormat == "c" || lowerFormat == "ch")
        {
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

    /**
     * @brief 计算仿真剩余时间，并实时更新控制台输出。
     * @details 根据当前仿真进度推算剩余计算时间，并通过 `\\r` 刷新当前行输出。
     * @param elapsedTime       仿真已流逝的模拟时间（秒）。
     * @param currentIndex      当前迭代步索引。
     * @param totalIterations   总迭代步数。
     * @param startTime         仿真计算启动的时间点（`std::chrono::steady_clock::time_point`）。
     * @param lastElapsedTime   上次记录的已流逝时间（秒），用于减少输出频率。
     * @param lastRemainingTime 上次计算的剩余时间（秒），用于减少输出频率。
     * @return `std::tuple<int, int>`，第一项为更新后的已流逝时间（秒），第二项为更新后的剩余时间（秒）。
     * @code
     * auto start = std::chrono::steady_clock::now();
     * int lastElapsed = 0, lastRemaining = 0;
     * auto [elapsed, remaining] = TimesHelper::CalculateRemainingTime(
     *     10.0, 100, 1000, start, lastElapsed, lastRemaining);
     * @endcode
     */
    std::tuple<int, int> TimesHelper::CalculateRemainingTime(double elapsedTime, int currentIndex, int totalIterations,
                                                             const std::chrono::steady_clock::time_point &startTime,
                                                             int lastElapsedTime, int lastRemainingTime)
    {
        if (elapsedTime - lastElapsedTime > 0)
        {
            auto now = std::chrono::steady_clock::now();                                       ///< 当前计时时间点
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime); ///< 自仿真开始已经过的实际计算时间（秒）

            double term = (totalIterations - currentIndex) * duration.count() / static_cast<double>(currentIndex); ///< 通过线性推算的剩余计算时长（秒）
            double remainingTime = term / 60.0;                                                                    ///< 剩余计算时间（分钟）

            if (lastElapsedTime == 0)
            {

                // Unix/Linux console manipulation
                std::cout << "\r";
                std::cout << "Simulation time elapsed: " << std::round(elapsedTime)
                          << " secs. Computation time left: " << std::round(remainingTime)
                          << " minutes.     ";
                std::cout.flush();
            }
            else
            {
                int roundedRemainingTime = static_cast<int>(std::round(remainingTime));
                if (roundedRemainingTime != 0)
                {

                    std::cout << "\r";
                    std::cout << "Simulation time elapsed: " << std::round(elapsedTime)
                              << " secs. Computation time left: " << std::round(remainingTime)
                              << " minutes.        ";
                    std::cout.flush();
                }
                else
                {

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

    /**
     * @brief 启动（或重置）内部秒表，开始计时。
     * @details 调用此方法后，`stopwatch_start` 将被设置为当前时间，
     *          `stopwatch_running` 被置为 true。通常与 Toc() 配合使用测量代码段耗时。
     * @code
     * TimesHelper::Tic();
     * // ... 执行某些操作 ...
     * std::string elapsed = TimesHelper::Toc(true, true, "s");
     * // 输出如："0.23 s"
     * @endcode
     */
    void TimesHelper::Tic()
    {
        stopwatch_start = std::chrono::steady_clock::now();
        stopwatch_running = true;
    }

    /**
     * @brief 将毫秒时长转为人类可读的时间字符串。
     * @details 根据时长大小自动选择适当的精度：
     *  - 包含小时：格式为 \"HH hours MM mins SS secs\"
     *  - 仅包含分钟：格式为 \"MM mins SS secs\"
     *  - 不足一分钟：格式为 \"SS secs\"
     *  所有分量均用两位数输出（补前导零）。
     * @param milliseconds 要转换的毫秒时长，应为非负值。
     * @param form         格式参数（当前保留未使用）。
     * @return 人类可读的时间字符串。
     * @code
     * std::string t = TimesHelper::MillisecondsToTime(3723000.0);
     * // t == "01 hours 02 mins 03 secs"
     * @endcode
     */
    std::string TimesHelper::MillisecondsToTime(double milliseconds, const std::string &form)
    {
        // 计算总共的秒数
        double totalSeconds = milliseconds / 1000.0; ///< 总秒数（由毫秒换算）

        // 计算小时数、分钟数和秒数
        int hours = static_cast<int>(totalSeconds / 3600);                          ///< 完整小时数
        int minutes = static_cast<int>((totalSeconds - hours * 3600) / 60);         ///< 去除小时后的完整分钟数
        int seconds = static_cast<int>(totalSeconds - hours * 3600 - minutes * 60); ///< 剩余不足一分钟的秒数

        // 返回格式化后的字符串
        if (hours > 0)
        {
            return FormatWithLeadingZeros(hours, 2) + " hours " +
                   FormatWithLeadingZeros(minutes, 2) + " mins " +
                   FormatWithLeadingZeros(seconds, 2) + " secs";
        }
        else if (minutes > 0)
        {
            return FormatWithLeadingZeros(minutes, 2) + " mins " +
                   FormatWithLeadingZeros(seconds, 2) + " secs";
        }
        else
        {
            return FormatWithLeadingZeros(seconds, 2) + " secs";
        }
    }

    /**
     * @brief 停止秒表并返回经过的时间字符串，可选择打印至控制台。
     * @details 计算自上次调用 Tic() 以来经过的时间，按 Control 参数指定的单位格式化输出。
     * @param show   是否将格式化时间输出到控制台/日志，true 表示打印，false 仅返回字符串。
     * @param 换行吗 是否在日志中换行输出。true 使用 WriteLog（带换行），false 使用 WriteLogO（不换行，覆盖当前行）。
     * @param Control 时间单位控制字符串，可选值为：
     *                - \"ms\"：毫秒
     *                - \"s\"：秒（默认）
     *                - \"min\"：分钟
     *                - \"h\"：小时
     * @param level  日志缩进级别（每级缩进一个空格）。
     * @return 格式化后的经过时间字符串，例如 \"3.14 s\"、\"150.00 ms\"。
     * @code
     * TimesHelper::Tic();
     * // ... 执行某些操作 ...
     * std::string t = TimesHelper::Toc(true, true, "s", 0);
     * // 控制台打印如 "0.23 s"，并返回 "0.23 s"
     * @endcode
     */
    std::string TimesHelper::Toc(bool show, bool 换行吗, const std::string &Control, int level)
    {
        if (!stopwatch_running)
        {
            return "0 s"; // 秒表未启动时直接返回
        }

        auto end = std::chrono::steady_clock::now(); ///< 停止计时时的时间点
        auto elapsed = end - stopwatch_start;        ///< 总经过时长（chrono duration 类型）
        stopwatch_running = false;                   ///< 将秒表置为停止状态

        std::string ms = ""; ///< 用于缩进的空格字符串（每个缩进级别添加一个空格）
        for (int i = 0; i < level; i++)
        {
            ms += " ";
        }

        std::string control = ToLower(Control); ///< 时间单位控制字符串的小写版本
        std::string timeString = "";            ///< 最终格式化后的时间字符串

        if (control == "ms")
        {
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << milliseconds.count() << " ms";
            timeString = ss.str();
        }
        else if (control == "s")
        {
            auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << seconds.count() << " s";
            timeString = ss.str();
        }
        else if (control == "min")
        {
            auto minutes = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<60>>>(elapsed);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << minutes.count() << " min";
            timeString = ss.str();
        }
        else if (control == "h")
        {
            auto hours = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(elapsed);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << hours.count() << " h";
            timeString = ss.str();
        }
        else
        { // default to seconds
            Qahse::IO::Log::LogHelper::ErrorLog("错误的时间参数!");
            auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << seconds.count() << " s";
            timeString = ss.str();
        }

        if (show)
        {
            if (换行吗)
            {
                Qahse::IO::Log::LogHelper::WriteLog(timeString, "", true, "[Message]",
                                                     Qahse::IO::System::ConsoleColor::White, true, level);
            }
            else
            {
                Qahse::IO::Log::LogHelper::WriteLogO(timeString);
                std::cout << '\r';
                std::cout << ms << timeString;
            }
        }
        else
        {
            Qahse::IO::Log::LogHelper::WriteLogO(timeString);
        }

        return timeString;
    }

    /**
     * @brief 将字符串转换为全小写（私有辅助方法）。
     * @param str 待转换的输入字符串。
     * @return 全小写版本的字符串副本。
     * @code
     * std::string s = TimesHelper::ToLower("Hello World");
     * // s == "hello world"
     * @endcode
     */
    std::string TimesHelper::ToLower(const std::string &str)
    {
        std::string result = str; ///< 将输入内容拷贝一份，避免修改原字符串
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
        return result;
    }

    /**
     * @brief 将整数格式化为指定宽度的字符串，不足位数时补前导零（私有辅助方法）。
     * @param value 待格式化的整数值。
     * @param width 输出字符串的最小宽度，不足时用 '0' 填充。
     * @return 已补前导零的格式化字符串，例如 `FormatWithLeadingZeros(5, 2)` 返回 `"05"`。
     * @code
     * std::string s = TimesHelper::FormatWithLeadingZeros(5, 2);
     * // s == "05"
     * @endcode
     */
    std::string TimesHelper::FormatWithLeadingZeros(int value, int width)
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(width) << value;
        return ss.str();
    }

} // namespace Qahse::IO::Math