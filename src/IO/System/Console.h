#pragma once
/**
 * @file Console.h
 * @brief 提供一个接近 C# 控制台体验的跨平台 C++17 工具（支持颜色、标题、Eigen 输出）。
 *
 * @details
 * 该文件为纯头文件实现，直接包含即可使用。
 *
 * 功能概览：
 * - 基础类型输出：int/float/double/bool/string/const char* 等。
 * - Eigen 输出：
 *   - 向量输出为 [x, y, z]
 *   - 矩阵输出为 [[a, b],\n [c, d]]
 * - 控制台颜色：前景色、背景色、恢复默认色。
 * - 控制台标题：Windows 与 Linux/macOS 统一接口。
 * - C# 风格接口：Title / ForegroundColor / BackgroundColor，以及 Write/WriteLine 着色重载。
 *
 * 跨平台策略：
 * - Windows 10+：优先启用 ANSI VT100。
 * - 老版本 Windows：自动回退到 Win32 SetConsoleTextAttribute。
 * - Linux/macOS：使用 ANSI 转义序列。
 *
 * @par 完整使用案例
 * @code{.cpp}
 * #include "IO/System/Console.h"
 * #include <Eigen/Dense>
 *
 * int main() {
 *     using namespace System;
 *
 *     // 1) 标题
 *     Console::Title("Qahse 控制台演示");
 *
 *     // 2) 基础输出
 *     Console::Write("步骤 = ");
 *     Console::WriteLine(42);
 *     Console::WriteLine(true);          // 输出 True
 *     Console::WriteLine(3.1415926);
 *
 *     // 3) 仅本次调用着色（最接近 C# 使用习惯）
 *     Console::WriteLine("信息", ConsoleColor::Cyan);
 *     Console::WriteLine("警告", ConsoleColor::Yellow);
 *     Console::WriteLine("错误", ConsoleColor::White, ConsoleColor::DarkRed);
 *
 *     // 4) 持续着色 + 手动恢复
 *     Console::SetForegroundColor(ConsoleColor::Green);
 *     Console::WriteLine("运行中...");
 *     Console::ResetColor();
 *
 *     // 5) Eigen 向量/矩阵输出
 *     Eigen::Vector3d v(1.0, 2.0, 3.0);
 *     Eigen::Matrix2d m;
 *     m << 1.0, 2.0,
 *          3.0, 4.0;
 *     Console::Write("v = ");
 *     Console::WriteLine(v, ConsoleColor::Magenta);  // [1, 2, 3]
 *     Console::Write("m = ");
 *     Console::WriteLine(m);
 *
 *     // 6) RAII 自动恢复颜色
 *     {
 *         ConsoleColorScope scope(ConsoleColor::Yellow);
 *         Console::WriteLine("作用域颜色块");
 *     } // 离开作用域自动 ResetColor
 *
 *     return 0;
 * }
 * @endcode
 */

#include <iostream>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <type_traits>

// ── 平台相关头文件 ─────────────────────────────────────────────────────────────
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <conio.h> // Windows 单键读取
#endif

// ── Eigen 支持 ─────────────────────────────────────────────────────────────────
// 策略：
//   1. 若已包含 Eigen 头文件，可通过其宏判断。
//   2. 否则尝试使用 __has_include 自动包含 Eigen/Dense。
//   3. 若环境无 Eigen，则关闭 Eigen 输出能力（HC3_CONSOLE_EIGEN=0）。
#if defined(EIGEN_CORE_H) || defined(EIGEN_WORLD_VERSION) || \
    defined(EIGEN_MATRIXBASE_H) || defined(EIGEN_DENSE_H)
#define HC3_CONSOLE_EIGEN 1
#elif defined(__has_include) && __has_include(<Eigen/Dense>)
#include <Eigen/Dense>
#define HC3_CONSOLE_EIGEN 1
#else
#define HC3_CONSOLE_EIGEN 0
#endif

namespace Qahse::IO::System
{

    // ══════════════════════════════════════════════════════════════════════════════
    //  ConsoleColor  — 对齐 C# 的 ConsoleColor，增加 Default 表示恢复默认色
    // ══════════════════════════════════════════════════════════════════════════════
    /**
     * @brief 控制台颜色枚举。
     * @note 取值布局与 C# ConsoleColor 保持一致，便于迁移已有调用习惯。
     */
    enum class ConsoleColor : int
    {
        Default = -1, ///< 恢复终端默认颜色（配合 ResetColor 使用）
        Black = 0,
        DarkBlue = 1,
        DarkGreen = 2,
        DarkCyan = 3,
        DarkRed = 4,
        DarkMagenta = 5,
        DarkYellow = 6,
        Gray = 7,
        DarkGray = 8,
        Blue = 9,
        Green = 10,
        Cyan = 11,
        Red = 12,
        Magenta = 13,
        Yellow = 14,
        White = 15
    };

    // ══════════════════════════════════════════════════════════════════════════════
    //  Console
    // ══════════════════════════════════════════════════════════════════════════════
    /**
     * @brief C# 风格控制台工具类。
     * @details
     * - 提供 Write/WriteLine 多类型重载。
     * - 提供颜色控制与按次着色输出。
     * - 提供标题、清屏、输入、stderr 输出等能力。
     */
    class Console
    {
    public:
        // ── 标题 ────────────────────────────────────────────────────────────────

        /**
         * @brief 设置控制台标题。
         * @param title 标题文本。
         */
        static void SetTitle(const std::string &title)
        {
            s_title = title;
#ifdef _WIN32
            ::SetConsoleTitleA(title.c_str());
#else
            // 使用 XTerm OSC-0 序列设置标题（多数 Linux/macOS 终端支持）
            std::cout << "\033]0;" << title << "\007";
            std::cout.flush();
#endif
        }

        /**
         * @brief C# 风格别名：设置标题。
         * @param title 标题文本。
         */
        static void Title(const std::string &title) { SetTitle(title); }

        /**
         * @brief C# 风格别名：获取最近一次设置的标题文本。
         */
        static const std::string &Title() { return s_title; }

        // ── 颜色 ─────────────────────────────────────────────────────────────────

        /**
         * @brief 设置前景色（持续生效，直到再次修改或 ResetColor）。
         */
        static void SetForegroundColor(ConsoleColor c)
        {
            ensureInit();
            s_fg = c;
            applyColor();
        }

        /**
         * @brief 获取当前记录的前景色状态。
         */
        static ConsoleColor GetForegroundColor() { return s_fg; }

        /**
         * @brief C# 风格别名：设置前景色。
         */
        static void ForegroundColor(ConsoleColor c) { SetForegroundColor(c); }

        /**
         * @brief C# 风格别名：获取前景色。
         */
        static ConsoleColor ForegroundColor() { return GetForegroundColor(); }

        /**
         * @brief 设置背景色（持续生效，直到再次修改或 ResetColor）。
         */
        static void SetBackgroundColor(ConsoleColor c)
        {
            ensureInit();
            s_bg = c;
            applyColor();
        }

        /**
         * @brief 获取当前记录的背景色状态。
         */
        static ConsoleColor GetBackgroundColor() { return s_bg; }

        /**
         * @brief C# 风格别名：设置背景色。
         */
        static void BackgroundColor(ConsoleColor c) { SetBackgroundColor(c); }

        /**
         * @brief C# 风格别名：获取背景色。
         */
        static ConsoleColor BackgroundColor() { return GetBackgroundColor(); }

        /**
         * @brief 恢复前景/背景到终端默认颜色。
         */
        static void ResetColor()
        {
            ensureInit();
            s_fg = ConsoleColor::Default;
            s_bg = ConsoleColor::Default;
#ifdef _WIN32
            if (s_useWin32)
            {
                ::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), s_defaultAttrib);
                return;
            }
#endif
            std::cout << "\033[0m";
            std::cout.flush();
        }

        // ── Write（不自动换行） ─────────────────────────────────────────────────

        /**
         * @brief 输出 bool，格式与 C# 一致（True/False）。
         */
        template <typename T, std::enable_if_t<std::is_same_v<T, bool>, int> = 0>
        static void Write(T v)
        {
            ensureInit();
            std::cout << (v ? "True" : "False");
            std::cout.flush();
        }

        /**
         * @brief 输出单个字符，不自动换行。
         */
        static void Write(char v)
        {
            ensureInit();
            std::cout.put(v);
            std::cout.flush();
        }

        /**
         * @brief 输出 C 字符串，不自动换行。
         * @note 当传入 nullptr 时不输出任何内容。
         */
        static void Write(const char *v)
        {
            ensureInit();
            if (v)
                std::cout << v;
            std::cout.flush();
        }

        /**
         * @brief 输出字符串，不自动换行。
         */
        static void Write(const std::string &v)
        {
            ensureInit();
            std::cout << v;
            std::cout.flush();
        }

        /**
         * @brief 本次输出使用指定前景色，输出后自动恢复先前颜色状态。
         */
        template <typename T>
        static void Write(const T &v, ConsoleColor fg)
        {
            const ColorState prev = saveColorState();
            SetForegroundColor(fg);
            Write(v);
            restoreColorState(prev);
        }

        /**
         * @brief 本次输出使用指定前景色和背景色，输出后自动恢复先前颜色状态。
         */
        template <typename T>
        static void Write(const T &v, ConsoleColor fg, ConsoleColor bg)
        {
            const ColorState prev = saveColorState();
            SetForegroundColor(fg);
            SetBackgroundColor(bg);
            Write(v);
            restoreColorState(prev);
        }

#if HC3_CONSOLE_EIGEN
        /**
         * @brief 输出 Eigen 表达式（向量/矩阵），不自动换行。
         * @details
         * - 向量输出： [x, y, z]
         * - 矩阵输出： [[a, b],\n [c, d]]
         */
        template <typename Derived>
        static void Write(const Eigen::DenseBase<Derived> &expr)
        {
            ensureInit();
            eigenFmt(std::cout, expr.eval());
            std::cout.flush();
        }
#endif

        /**
         * @brief 通用输出重载（支持任意可被 ostream << 的类型）。
         * @note 通过 SFINAE 避免与 bool/char/string/C 字符串等专用重载冲突。
         */
        template <typename T,
                  std::enable_if_t<
                      !std::is_same_v<std::decay_t<T>, bool> &&
                          !std::is_same_v<std::decay_t<T>, char> &&
                          !std::is_same_v<std::decay_t<T>, char *> &&
                          !std::is_same_v<std::decay_t<T>, const char *> &&
                          !std::is_same_v<std::decay_t<T>, std::string>,
                      int> = 0>
        static void Write(const T &v)
        {
            ensureInit();
            std::cout << v;
            std::cout.flush();
        }

        // ── WriteLine（追加换行） ────────────────────────────────────────────────

        /**
         * @brief 输出换行。
         */
        static void WriteLine()
        {
            ensureInit();
            std::cout.put('\n');
            std::cout.flush();
        }

        /**
         * @brief 输出一行空行，并临时使用指定前景色。
         */
        static void WriteLine(ConsoleColor fg)
        {
            const ColorState prev = saveColorState();
            SetForegroundColor(fg);
            WriteLine();
            restoreColorState(prev);
        }

        /**
         * @brief 输出一行空行，并临时使用指定前景色和背景色。
         */
        static void WriteLine(ConsoleColor fg, ConsoleColor bg)
        {
            const ColorState prev = saveColorState();
            SetForegroundColor(fg);
            SetBackgroundColor(bg);
            WriteLine();
            restoreColorState(prev);
        }

        /**
         * @brief 输出任意内容并追加换行。
         */
        template <typename T>
        static void WriteLine(const T &v)
        {
            Write(v);
            std::cout.put('\n');
            std::cout.flush();
        }

        /**
         * @brief 输出任意内容并换行，且仅本次调用使用指定前景色。
         */
        template <typename T>
        static void WriteLine(const T &v, ConsoleColor fg)
        {
            const ColorState prev = saveColorState();
            SetForegroundColor(fg);
            WriteLine(v);
            restoreColorState(prev);
        }

        /**
         * @brief 输出任意内容并换行，且仅本次调用使用指定前景色和背景色。
         */
        template <typename T>
        static void WriteLine(const T &v, ConsoleColor fg, ConsoleColor bg)
        {
            const ColorState prev = saveColorState();
            SetForegroundColor(fg);
            SetBackgroundColor(bg);
            WriteLine(v);
            restoreColorState(prev);
        }

        // ── 错误输出（stderr） ────────────────────────────────────────────────────

        /**
         * @brief 输出到标准错误流（stderr），不换行。
         */
        template <typename T>
        static void WriteError(const T &v)
        {
            std::cerr << v;
            std::cerr.flush();
        }

        /**
         * @brief 向标准错误流输出换行。
         */
        static void WriteErrorLine()
        {
            std::cerr.put('\n');
            std::cerr.flush();
        }

        /**
         * @brief 输出到标准错误流并换行。
         */
        template <typename T>
        static void WriteErrorLine(const T &v)
        {
            std::cerr << v << '\n';
            std::cerr.flush();
        }

        // ── 清屏 ─────────────────────────────────────────────────────────────────

        /**
         * @brief 清空控制台并将光标移动到左上角。
         */
        static void Clear()
        {
            ensureInit();
#ifdef _WIN32
            HANDLE h = ::GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO csbi{};
            if (::GetConsoleScreenBufferInfo(h, &csbi))
            {
                const DWORD total = static_cast<DWORD>(csbi.dwSize.X * csbi.dwSize.Y);
                const COORD origin = {0, 0};
                DWORD written = 0;
                ::FillConsoleOutputCharacterA(h, ' ', total, origin, &written);
                ::FillConsoleOutputAttribute(h, csbi.wAttributes, total, origin, &written);
                ::SetConsoleCursorPosition(h, origin);
            }
#else
            std::cout << "\033[2J\033[H";
            std::cout.flush();
#endif
        }

        // ── 输入 ─────────────────────────────────────────────────────────────────

        /**
         * @brief 读取一整行输入。
         * @return 读取到的字符串（不含换行符）。
         */
        static std::string ReadLine()
        {
            // 输入字符串应该支持中文
            std::string line;
            std::getline(std::cin, line);
// 将GBK编码的line转为utf-8,然后返回
#ifdef _WIN32
            int wlen = MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, nullptr, 0);
            if (wlen <= 0)
                return "";
            std::wstring wline(wlen - 1, L'\0');
            MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, &wline[0], wlen);

            int u8len = WideCharToMultiByte(CP_UTF8, 0, wline.c_str(), -1, nullptr, 0, nullptr, nullptr);
            if (u8len <= 0)
                return "";
            std::string u8line(u8len - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wline.c_str(), -1, &u8line[0], u8len, nullptr, nullptr);
            return u8line;
#else
            return line;
#endif

            return line;
        }

        /**
         * @brief 读取一个按键。
         * @details
         * - Windows 使用 _getch()（通常不回显）。
         * - Linux/macOS 使用 getchar() 简化实现。
         */
        static int ReadKey()
        {
#ifdef _WIN32
            return ::_getch();
#else
            return std::getchar();
#endif
        }

        // ── 显式刷新 ─────────────────────────────────────────────────────────────

        /**
         * @brief 强制刷新 stdout。
         */
        static void Flush()
        {
            std::cout.flush();
        }

    private:
        struct ColorState
        {
            ConsoleColor fg;
            ConsoleColor bg;
        };

        static ColorState saveColorState()
        {
            return ColorState{s_fg, s_bg};
        }

        static void restoreColorState(const ColorState &state)
        {
            s_fg = state.fg;
            s_bg = state.bg;
            applyColor();
        }

        // ── 静态状态 ─────────────────────────────────────────────────────────────
        static inline ConsoleColor s_fg = ConsoleColor::Default;
        static inline ConsoleColor s_bg = ConsoleColor::Default;
        static inline std::string s_title{};

#ifdef _WIN32
        static inline bool s_useWin32 = false;
        static inline WORD s_defaultAttrib =
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // 默认：灰色前景 + 黑色背景
#endif

        // ── 一次性初始化（C++11 局部静态变量线程安全） ────────────────────────────
        static void ensureInit()
        {
            static const bool kOnce = []
            {
#ifdef _WIN32
                HANDLE h = ::GetStdHandle(STD_OUTPUT_HANDLE);
                // 保存当前控制台属性，便于后续恢复
                CONSOLE_SCREEN_BUFFER_INFO csbi{};
                if (::GetConsoleScreenBufferInfo(h, &csbi))
                    s_defaultAttrib = csbi.wAttributes;
                // 尝试开启 ANSI 虚拟终端模式（Windows 10+）
                DWORD mode = 0;
                if (::GetConsoleMode(h, &mode) &&
                    !::SetConsoleMode(h, mode | ENABLE_PROCESSED_OUTPUT |
                                             ENABLE_VIRTUAL_TERMINAL_PROCESSING))
                    s_useWin32 = true; // ANSI 不可用时回退 Win32 方式
#endif
                return true;
            }();
            (void)kOnce;
        }

        // ── ANSI 转义码构造 ─────────────────────────────────────────────────────
        //
        // ConsoleColor 索引 -> ANSI SGR 参数：
        //   0-7   标准色（前景 30-37 / 背景 40-47）
        //   8-15  高亮色（前景 90-97 / 背景 100-107）
        static std::string ansiEsc(ConsoleColor c, bool bg)
        {
            if (c == ConsoleColor::Default)
                return bg ? "\033[49m" : "\033[39m";
            static constexpr int FG[16] = {30, 34, 32, 36, 31, 35, 33, 37,
                                           90, 94, 92, 96, 91, 95, 93, 97};
            static constexpr int BG[16] = {40, 44, 42, 46, 41, 45, 43, 47,
                                           100, 104, 102, 106, 101, 105, 103, 107};
            const int i = static_cast<int>(c);
            if (i < 0 || i > 15)
                return {};
            char buf[12];
            std::snprintf(buf, sizeof(buf), "\033[%dm", bg ? BG[i] : FG[i]);
            return buf;
        }

        // ── 将当前 s_fg / s_bg 应用到终端 ─────────────────────────────────────────
        static void applyColor()
        {
#ifdef _WIN32
            if (s_useWin32)
            {
                HANDLE h = ::GetStdHandle(STD_OUTPUT_HANDLE);
                WORD attr = s_defaultAttrib;
                attr &= ~(WORD)0x00FF; // 清除前景（0-3 位）和背景（4-7 位）
                if (s_fg != ConsoleColor::Default)
                    attr |= static_cast<WORD>(static_cast<int>(s_fg) & 0x0F);
                if (s_bg != ConsoleColor::Default)
                    attr |= static_cast<WORD>((static_cast<int>(s_bg) & 0x0F) << 4);
                ::SetConsoleTextAttribute(h, attr);
                return;
            }
#endif
            std::string esc;
            esc.reserve(14);
            if (s_fg != ConsoleColor::Default)
                esc += ansiEsc(s_fg, false);
            if (s_bg != ConsoleColor::Default)
                esc += ansiEsc(s_bg, true);
            if (!esc.empty())
            {
                std::cout << esc;
                std::cout.flush();
            }
        }

#if HC3_CONSOLE_EIGEN
        // ── Eigen 格式化输出 ─────────────────────────────────────────────────────
        //
        //   向量（rows==1 或 cols==1） -> [x, y, z]
        //   矩阵（rows>1 且 cols>1）  -> [[a, b, c],
        //                                 [d, e, f]]
        template <typename EvalType>
        static void eigenFmt(std::ostream &os, const EvalType &m)
        {
            const int rows = static_cast<int>(m.rows());
            const int cols = static_cast<int>(m.cols());

            if (rows == 0 || cols == 0)
            {
                os << "[]";
                return;
            }

            if (rows == 1 || cols == 1)
            {
                // ── 向量 ────────────────────────────────────────────────────
                const int n = (rows == 1) ? cols : rows;
                os << "[";
                for (int k = 0; k < n; ++k)
                {
                    if (k)
                        os << ", ";
                    os << ((rows == 1) ? m(0, k) : m(k, 0));
                }
                os << "]";
            }
            else
            {
                // ── 矩阵 ────────────────────────────────────────────────────
                for (int r = 0; r < rows; ++r)
                {
                    os << (r == 0 ? "[[" : " [");
                    for (int c = 0; c < cols; ++c)
                    {
                        if (c)
                            os << ", ";
                        os << m(r, c);
                    }
                    if (r < rows - 1)
                        os << "],\n";
                    else
                        os << "]]";
                }
            }
        }
#endif // HC3_CONSOLE_EIGEN
    };

    // ══════════════════════════════════════════════════════════════════════════════
    //  ConsoleColorScope  — RAII 守卫，离开作用域自动恢复颜色
    // ══════════════════════════════════════════════════════════════════════════════
    /**
     * @brief 作用域颜色守卫。
     * @details 构造时设置颜色，析构时恢复默认颜色，适合局部着色输出。
     *
     * 使用示例：
     *   {
     *       ConsoleColorScope scope(ConsoleColor::Yellow);
     *       Console::WriteLine("警告：时间步长过小");
     *   } // 离开作用域后自动恢复颜色
     */
    class ConsoleColorScope
    {
    public:
        /**
         * @brief 构造时设置颜色。
         * @param fg 前景色。
         * @param bg 背景色（默认不修改）。
         */
        explicit ConsoleColorScope(ConsoleColor fg,
                                   ConsoleColor bg = ConsoleColor::Default)
        {
            if (fg != ConsoleColor::Default)
                Console::SetForegroundColor(fg);
            if (bg != ConsoleColor::Default)
                Console::SetBackgroundColor(bg);
        }

        /**
         * @brief 析构时自动恢复默认颜色。
         */
        ~ConsoleColorScope()
        {
            Console::ResetColor();
        }
        ConsoleColorScope(const ConsoleColorScope &) = delete;
        ConsoleColorScope &operator=(const ConsoleColorScope &) = delete;
    };

} // namespace Qahse::IO::System
