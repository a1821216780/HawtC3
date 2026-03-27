//**********************************************************************************************************************************
// LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of Qahse.IO.Log.
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

#include <sstream>
#include <iomanip>
#include <filesystem>
#include <csignal>
#include <ctime>
#include <algorithm>

#include "LogHelper.h"
#include "../IO/OtherHelper.h"
#include "../IO/TimesHelper.h"
#include "../IO/Extensions.h"
#include "../System/Console.h"

//
// #ifdef _WIN32
// #include <conio.h>
// #endif

using namespace Qahse::IO::IO;
using namespace Qahse::IO::System;

namespace Qahse::IO::Log
{

	// ---- 静态成员定义 ----
	std::shared_ptr<Email> Datastore::EmailSet = nullptr; ///< 全局邮件发送实例，nullptr 表示不发送邮件

	// std::string LogData::LogPath = "";
	// std::vector<std::shared_ptr<IOutFile*>> LogData::OutFilelist;
	// std::vector<std::shared_ptr<BASE_VTK>> LogData::IO_VTK;
	// std::vector<std::string> LogData::log_inf;

	int LogHelper::mode = 0;					 ///< 运行模式：0=Debug 调试模式，1=Release 发布模式
	bool LogHelper::firstshowinformation = true; ///< 首次显示标志，确保启动信息只输出一次

	/**
	 * @brief 向日志消息列表追加一条记录
	 *
	 * @details 将消息追加到 log_inf 字符串数组中。
	 * 若 log_inf 为空（未初始化），则打印错误提示要求先调用 DisplayInformation()。
	 * 所有通过 WriteLog/DebugLog/WriteLogO 输出的内容最终都会调用本函数存档，
	 * 在 EndProgram() 时统一写入 .log 文件。
	 *
	 * @param message 要追加的日志消息字符串
	 *
	 * @code
	 * // 正常用法（必须先初始化）：
	 * LogData::Initialize();
	 * LogData::Add("仿真步骤 1 完成");
	 * // log_inf 中现在包含两条记录：标题行 + 消息行
	 * @endcode
	 */
	void LogData::Add(const std::string &message)
	{
		if (log_inf.empty())
		{
			LogHelper::ErrorLog("Please call DisplayInformation function first!");
		}
		else
		{
			log_inf.push_back(message);
		}
	}

	/**
	 * @brief 初始化日志数据存储结构
	 *
	 * @details 清空并重置所有静态成员：
	 * - 清空 log_inf 消息列表，并写入日志文件头标题行
	 * - 清空 OutFilelist 输出文件列表
	 * - 根据当前工作目录和项目名构造日志文件路径（.log）
	 *
	 * @note 必须在 LogHelper::DisplayInformation() 之前隐式调用，
	 * 不允许在仿真中途重复调用（会清空已有日志）。
	 *
	 * @code
	 * // 通常由 DisplayInformation 自动调用，无需手动使用：
	 * LogData::Initialize();
	 * // LogData::LogPath == "<cwd>/Qahse.ProjectName.log"
	 * @endcode
	 */
	void LogData::Initialize()
	{
		log_inf.clear(); ///< 清空历史日志记录
		log_inf.push_back("----------! Qahse." + OtherHelper::GetCurrentProjectName() + " Log file. !----------");
		OutFilelist.clear(); ///< 清空输出文件列表
		// Initialize LogPath
		std::string currentPath = Extensions::GetABSPath(std::filesystem::current_path().string());					 ///< 当前工作目录的绝对路径
		LogPath = Extensions::GetABSPath(currentPath + "./Qahse." + OtherHelper::GetCurrentProjectName() + ".log"); ///< 日志文件完整路径
	}

	/**
	 * @brief 处理操作系统信号（如 SIGINT、SIGSEGV 等）导致的程序异常终止
	 *
	 * @details 作为全局信号处理器，拦截常见的进程终止信号，
	 * 输出信号名称到错误日志，然后调用 EndProgram(true) 执行优雅退出。
	 * 支持的信号类型：
	 * - SIGINT  (2)：用户按 Ctrl+C 中断
	 * - SIGTERM (15)：系统终止请求
	 * - SIGSEGV (11)：段错误（非法内存访问）
	 * - SIGFPE  (8) ：浮点运算异常（如除零）
	 *
	 * @param signal 操作系统传入的信号编号
	 *
	 * @note 通过 std::signal() 注册，仅在 Release 模式且 DisplayInformation(true) 时生效。
	 *
	 * @code
	 * // 注册方式（由 DisplayInformation 自动完成）：
	 * std::signal(SIGINT,  LogHelper::UnhandledExceptionHandler);
	 * std::signal(SIGTERM, LogHelper::UnhandledExceptionHandler);
	 * std::signal(SIGSEGV, LogHelper::UnhandledExceptionHandler);
	 * std::signal(SIGFPE,  LogHelper::UnhandledExceptionHandler);
	 * @endcode
	 */
	void LogHelper::UnhandledExceptionHandler(int signal)
	{
		std::string signalName; ///< 信号名称字符串，用于日志输出
		switch (signal)
		{
		case SIGINT:
			signalName = "SIGINT";
			break;
		case SIGTERM:
			signalName = "SIGTERM";
			break;
		case SIGSEGV:
			signalName = "SIGSEGV";
			break;
		case SIGFPE:
			signalName = "SIGFPE";
			break;
		default:
			signalName = "Unknown Signal";
			break;
		}

		ErrorLog("未知错误：Signal " + signalName, "", "", 20, "UnhandledExceptionHandler");
		EndProgram(true, "", true);
	}

	/**
	 * @brief 以调试模式（V1 风格）显示程序启动 Banner 和版权信息
	 *
	 * @details 在 Debug 构建下调用，输出包含：
	 * - 彩色 ASCII art Logo（Qahse 图案）
	 * - 版权信息和授权说明
	 * - 当前版本号、年月信息
	 * - 数学库加速信息（MKL/OpenBLAS）
	 * - 设置控制台窗口标题
	 * - 将 mode 设置为 0（Debug 模式，输出详细信息）
	 *
	 * @param lics 许可证显示开关（当前实现中保留参数，暂未使用）
	 *
	 * @code
	 * // Debug 模式下启动：
	 * LogHelper::DisinfV1(true);
	 * // 控制台将显示彩色 Qahse Logo + 版本 + 版权信息
	 * // 并设置窗口标题为 "ProjectName_v1.0.0  Debug"
	 * @endcode
	 */
	void LogHelper::DisinfV1(bool lics)
	{
		// Set console title (Windows only)
		std::filesystem::path exepath = std::filesystem::current_path() / OtherHelper::GetCurrentExeName(); ///< 可执行文件的完整路径（用于提取版本号）

		std::string title = OtherHelper::GetCurrentProjectName() + OtherHelper::GetBuildMode() + "_" +
							OtherHelper::GetCurrentVersion(exepath.string()) + "  " + OtherHelper::GetCurrentBuildMode(); ///< 控制台窗口标题（项目名+构建模式+版本号）

		Console::SetTitle(title.c_str());

		int lp = 100; ///< 每行文本的目标居中宽度（字符数）

		WriteLog("Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯", "",
				 false, "", ConsoleColor::Green);
		WriteLog(OtherHelper::CenterText("------------------------------------------------------------------------------------------", lp),
				 "", false, "", ConsoleColor::Cyan);
		WriteLog(OtherHelper::CenterText("!       000                                00     00    00  0000000   00000  000000      !", lp),
				 "", false, "", ConsoleColor::Cyan);
		WriteLog(OtherHelper::CenterText("!     0     0    000      000      00000   00    00    00  0        0       00     00    !", lp),
				 "", false, "", ConsoleColor::Cyan);
		WriteLog(OtherHelper::CenterText("!     0     0  0     0  0     0   0     0  00   0000   00  000000  0       00      00    !", lp),
				 "", false, "", ConsoleColor::Cyan);
		WriteLog(OtherHelper::CenterText("!      000    0      0 0       0 0     0    0  0   0  0   0        0       00     00     !", lp),
				 "", false, "", ConsoleColor::Cyan);
		WriteLog(OtherHelper::CenterText("!             0 0000   0  000    0     0     00     00    0000000   00000  0000000       !", lp),
				 "", false, "", ConsoleColor::Cyan);
		WriteLog(OtherHelper::CenterText("!             0        0     0                                                           !", lp),
				 "", false, "", ConsoleColor::Cyan);
		WriteLog(OtherHelper::CenterText("!            0          0000                                                             !", lp),
				 "", false, "", ConsoleColor::Cyan);
		WriteLog(OtherHelper::CenterText("!                                                                                        !", lp),
				 "", false, "", ConsoleColor::Cyan);
		WriteLog(OtherHelper::CenterText("!                    Copyright (c) HawtC2.Team.ZZZ,赵子祯 licensed under GPL v3 .                   !", lp),
				 "", false, "", ConsoleColor::Red);
		WriteLog(OtherHelper::CenterText("!                                                                                        !", lp),
				 "", false, "", ConsoleColor::Cyan);
		WriteLog(OtherHelper::CenterText("!   Copyright (c)  Key Laboratory of Jiangsu province High-Tech design of wind turbine   !", lp),
				 "", false, "", ConsoleColor::Green);
		WriteLog(OtherHelper::CenterText("!                                                                                        !", lp),
				 "", false, "", ConsoleColor::Green);

		WriteLog(OtherHelper::CenterText("******                 Running " + OtherHelper::GetCurrentProjectName() +
											 " (v" + OtherHelper::GetCurrentVersion(exepath.string()) + "  " + std::to_string(TimesHelper::GetCurrentYear()) + "-" +
											 std::to_string(TimesHelper::GetCurrentMonth()) + ")                  ******",
										 lp),
				 "", false, "[Message]", ConsoleColor::Green);
		WriteLog(OtherHelper::CenterText("------------------------------------------------------------------------------------------", lp),
				 "", false, "", ConsoleColor::Cyan);
		WriteLog("Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯", "",
				 false, "", ConsoleColor::Green);
		WriteLog("Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯 Debug 调试模式 develop by 赵子祯", "",
				 false, "", ConsoleColor::Green);
		WriteLog(OtherHelper::GetMathAcc());

		mode = 0;
	}

	/**
	 * @brief 以发布模式（V2 风格）显示程序版本和机构信息
	 *
	 * @details 在 Release 构建下调用，输出精简的程序信息：
	 * - 项目名称、联系电话和官网地址
	 * - 版本号、构建模式、构建时间和数学库信息
	 * - 研发机构/团队信息
	 * - 将 mode 设置为 1（Release 模式），将 firstshowinformation 设为 false
	 *
	 * @param lics 许可证显示开关（当前版本保留参数，暂未使用）
	 *
	 * @code
	 * // Release 模式下启动（DisinfV2 自动被 DisplayInformation 调用）：
	 * LogHelper::DisinfV2(true);
	 * // 输出示例：
	 * // Qahse.NREL_5MW - Tel:13935201274  E:xxx@qq.com http://www.HawtC.cn
	 * // ****...
	 * // NREL_5MW Vision:V3.0.0_Release,BuildAt: 2025-08-09 Math:MKL
	 * //  > Publish: Powered by Key Laboratory of Jiangsu province...
	 * @endcode
	 */
	void LogHelper::DisinfV2(bool lics)
	{
		std::string url = "http://www.HawtC.cn"; ///< 官网地址，显示在控制台欢迎信息行

		WriteLog("OpenWECD." + OtherHelper::GetCurrentProjectName() + " - Tel:13935201274  E:1821216780@qq.com " + url,
				 "", false);
		std::cout << "************************************************************************************" << '\n';

		std::filesystem::path exepath = std::filesystem::current_path() / OtherHelper::GetCurrentExeName();

		WriteLog(OtherHelper::GetCurrentProjectName() + " Vision:V" + OtherHelper::GetCurrentVersion(exepath.string()) + "_" +
					 OtherHelper::GetCurrentBuildMode() + OtherHelper::GetBuildMode() + ",BuildAt: " +
					 OtherHelper::GetBuildTime() + " Math:" + OtherHelper::GetMathAcc(),
				 "", false, "", ConsoleColor::Green);
		WriteLog(" > Publish: Powered by Key Laboratory of Jiangsu province high tech design @Tg Team",
				 "", false);

		firstshowinformation = false;
		mode = 1;
	}

	/**
	 * @brief 显示程序启动信息并初始化日志系统（主入口函数）
	 *
	 * @details 程序启动时的首要调用函数，主要功能：
	 * 1. 调用 LogData::Initialize() 初始化日志存储
	 * 2. 根据构建模式（Debug/Release）选择 DisinfV1 或 DisinfV2 显示 Banner
	 * 3. 若 UnhandledException=true 且为 Release 模式，注册全局信号处理器
	 * 4. 通过 firstshowinformation 标志确保只执行一次
	 *
	 * @param UnhandledException 是否注册全局信号处理器（SIGINT/SIGTERM/SIGSEGV/SIGFPE）。
	 *                           仅在 Release 模式下生效；Debug 模式下 IDE 调试器本身就能捕获异常
	 * @param lic 许可证显示开关，传递给 DisinfV1/DisinfV2
	 *
	 * @code
	 * // 推荐在 main() 函数第一行调用：
	 * int main(int argc, char* argv[]) {
	 *     LogHelper::DisplayInformation(true, true);
	 *     // ... 后续仿真初始化 ...
	 * }
	 * @endcode
	 */
	void LogHelper::DisplayInformation(bool UnhandledException, bool lic)
	{
		if (firstshowinformation)
		{
			// Initialize LogData
			LogData::Initialize();

			if (OtherHelper::GetCurrentBuildMode() == "Debug")
			{
				DisinfV1(lic);
			}
			else
			{
				DisinfV2(lic);
			}
			firstshowinformation = false;

			if (OtherHelper::GetCurrentBuildMode() == "Release")
			{
				if (UnhandledException)
				{
					std::signal(SIGINT, UnhandledExceptionHandler);
					std::signal(SIGTERM, UnhandledExceptionHandler);
					std::signal(SIGSEGV, UnhandledExceptionHandler);
					std::signal(SIGFPE, UnhandledExceptionHandler);
				}
			}
		}
	}

	/**
	 * @brief 输出仿真结束信息，包含总仿真时间和计算耗时
	 *
	 * @details 在仿真时间步循环结束后调用，计算并输出：
	 * - 仿真物理终止时间 tfinal
	 * - 时间步长 dt
	 * - 实际计算耗时（秒/分钟，超过 60s 自动转为分钟显示）
	 *
	 * @param tfinal     仿真物理终止时间（单位：秒），例如 600.0
	 * @param dt         仿真时间步长（单位：秒），例如 0.05
	 * @param start_time 仿真循环开始时的时间戳（由 std::chrono::steady_clock::now() 获取）
	 *
	 * @code
	 * auto start = std::chrono::steady_clock::now();
	 * // ... 执行仿真循环 ...
	 * LogHelper::EndInformation(600.0, 0.05, start);
	 * // 输出示例：
	 * // [Message] Simulation Run Finished! The tf=600.000000s Step=0.050000s Cost real time=45.230000 sec
	 * @endcode
	 */
	void LogHelper::EndInformation(double tfinal, double dt, const std::chrono::steady_clock::time_point &start_time)
	{
		auto end_time = std::chrono::steady_clock::now();											  ///< 仿真结束时刻
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time); ///< 总耗时（毫秒）
		double elapsedTime = duration.count() / 1000.0;												  ///< 总耗时转换为秒

		std::string timeString = (elapsedTime > 60.0) ? std::to_string(elapsedTime / 60.0) + " min" : std::to_string(elapsedTime) + " sec"; ///< 格式化耗时字符串，超过60秒则显示分钟

		std::cout << '\n';
		WriteLog("Simulation Run Finished! The tf= " + std::to_string(tfinal) + "s Step=" +
					 std::to_string(dt) + "s Cost real time=" + timeString,
				 "", true, "[Message]",
				 ConsoleColor::White, true, 0);
	}

	/**
	 * @brief 结束程序执行，写入日志文件，关闭所有输出流，支持正常/异常两种退出模式
	 *
	 * @details 程序退出时的统一处理函数，完成以下工作：
	 * 1. 构造结束消息（正常完成 or 错误终止）并追加到日志
	 * 2. 将 LogData::log_inf 中所有日志写入 .log 文件
	 * 3. 关闭 LogData::IO_VTK 中所有 VTK 文件
	 * 4. 调用 LogData::OutFilelist 中所有输出文件的 Outfinish()
	 * 5. 根据 forceTerminate 决定打印绿色（正常）或红色（错误）结束提示
	 * 6. 可选等待用户按键（keepstay=true）或直接抛出异常退出（extcall=false）
	 *
	 * @param forceTerminate 是否为强制异常终止：
	 *                       - true：打印红色错误信息，程序异常退出
	 *                       - false：打印绿色完成信息，程序正常退出
	 * @param outstring      自定义结束消息；若为空则自动生成默认消息
	 * @param keepstay       是否在结束后等待用户按 Enter（便于查看输出）
	 * @param extcall        是否由外部代码调用（true=仅返回，false=抛出异常退出进程）
	 * @param sleeptime      退出前等待毫秒数，默认为 0
	 *
	 * @code
	 * // 正常完成时调用：
	 * LogHelper::EndProgram(false, "", false, false);
	 *
	 * // 异常终止（错误提示 + 等待按键）：
	 * LogHelper::EndProgram(true, "配置文件解析失败", true);
	 * @endcode
	 */
	void LogHelper::EndProgram(bool forceTerminate, const std::string &outstring, bool keepstay,
							   bool extcall, int sleeptime)
	{
		std::string message; ///< 结束消息（正常完成提示 or 错误提示）
		if (outstring.empty())
		{
			message = forceTerminate ? "************* ! " + OtherHelper::FillString(OtherHelper::GetCurrentProjectName() + ".RUN.", " ", 1) + " E R R O R ! **************" : OtherHelper::GetCurrentProjectName() + " Run completed Normaly!";
		}
		else
		{
			message = outstring;
		}

		LogData::Add(message);

		// Write all logs to file
		std::ofstream logFile(LogData::LogPath);
		for (const auto &line : LogData::log_inf)
		{
			logFile << line << '\n';
		}
		logFile.close();

		// Close VTK files
		for (auto &vtkFile : LogData::IO_VTK)
		{
			// vtkFile->OutVTKClose(); // Would need proper implementation
		}

		// Write output files
		for (auto &outFile : LogData::OutFilelist)
		{
			WriteLogO(std::string("Write ") + "output file" + " Out File!");
			outFile->Outfinish(false);
		}

		if (!forceTerminate)
		{
			std::cout << '\n';
			WriteLog(OtherHelper::GetCurrentProjectName() + " terminated normally!",
					 "", false, "", ConsoleColor::Green);
			std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));

			if (keepstay)
			{
				std::cin.get();
			}
			else
			{
				if (!extcall)
				{
					throw new std::runtime_error("Aborting " + OtherHelper::GetCurrentProjectName());
					std::exit(0);
				}
			}
		}
		else
		{
			std::cout << '\n';
			WriteLog(" Aborting " + OtherHelper::GetCurrentProjectName(),
					 "", false, "", ConsoleColor::Red);

			std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));

			if (!keepstay)
			{
				if (extcall)
				{
					return; // throw new Exception("Aborting " + Otherhelper.GetCurrentProjectName());
				}
				else
				{
					if (OtherHelper::GetCurrentProjectName() == "Microsoft.DotNet.Interactive.App")
					{
						return;
					}
					throw new std::runtime_error("Aborting " + OtherHelper::GetCurrentProjectName());
					std::exit(0);
				}
			}
			else
			{
				std::cin.get();
			}
		}
	}

	/**
	 * @brief 输出警告级别日志到控制台，带 [WARNING] 前缀标签
	 *
	 * @details 用于非致命性问题的提示，程序继续运行。
	 * - Debug 模式（mode=0）：输出 debugmessage（详细信息）
	 * - Release 模式（mode=1）：输出 relasemessage（若为空则用 debugmessage）
	 * 消息格式："FunctionName : 消息内容"
	 *
	 * @param debugmessage   调试模式下显示的详细警告消息
	 * @param relasemessage  发布模式下显示的简洁警告消息（空字符串则使用 debugmessage）
	 * @param color1         控制台输出颜色，推荐 ConsoleColor::DarkYellow 或 Yellow
	 * @param leval          缩进级别（空格数），用于层次化日志对齐
	 * @param FunctionName   调用警告的函数名，用于追溯来源
	 *
	 * @code
	 * // 在读取配置文件时发出警告：
	 * LogHelper::WarnLog(
	 *     "叶片翼型数据点数少于推荐值（< 100），可能影响插值精度",
	 *     "翼型数据点数不足",
	 *     ConsoleColor::Yellow, 2, "LoadAirfoilData"
	 * );
	 * // 输出：  [WARNING] LoadAirfoilData : 叶片翼型数据点数少于推荐值（< 100），可能影响插值精度
	 * @endcode
	 */
	void LogHelper::WarnLog(const std::string &debugmessage, const std::string &relasemessage,
							ConsoleColor color1, int leval, const std::string &FunctionName)
	{
		std::string message = relasemessage.empty() ? debugmessage : relasemessage; ///< 最终输出的消息（优先用 relasemessage）

		if (mode == 0)
		{
			std::string formattedMessage = FunctionName + " : " + debugmessage;
			WriteLog(formattedMessage, "", true, "[WARNING]", color1, true, leval);
			Console::ResetColor();
		}
		else
		{
			std::string formattedMessage = FunctionName + " : " + message;
			WriteLog(formattedMessage, "", true, "[WARNING]", color1, true, leval);
			Console::ResetColor();
		}
	}

	/**
	 * @brief 输出错误级别日志，并在打印后调用 EndProgram(true) 强制终止程序
	 *
	 * @details 用于致命错误处理。调用序列：
	 * 1. 格式化消息为 "FunctionName ERROR! message"
	 * 2. 若 title 为空，自动生成包含项目名的错误标题行
	 * 3. 以红色输出到控制台（带标题行）
	 * 4. 调用 EndProgram(true) 写日志、关闭文件、退出程序
	 *
	 * @param message      主错误消息（调试模式显示的详细信息）
	 * @param relmessage   发布模式下的简洁错误消息（空字符串则使用 message）
	 * @param title        错误框标题行（空字符串则自动生成标准格式标题）
	 * @param outtime      EndProgram 等待时间（毫秒），默认 20ms
	 * @param FunctionName 报错函数名（空字符串则显示 "UnknownFunction"）
	 *
	 * @warning 调用此函数后程序必然终止，不会返回
	 *
	 * @code
	 * // 文件不存在时报错并退出：
	 * if (!std::filesystem::exists(inputFile)) {
	 *     LogHelper::ErrorLog(
	 *         "输入文件不存在：" + inputFile,
	 *         "Input file not found",
	 *         "", 20, "LoadInputFile"
	 *     );
	 * }
	 * @endcode
	 */
	void LogHelper::ErrorLog(const std::string &message, const std::string &relmessage,
							 const std::string &title, int outtime, const std::string &FunctionName)
	{
		std::string functionName = FunctionName; ///< 报错函数名，空时赋值为 "UnknownFunction"
		if (functionName.empty())
		{
			functionName = "UnknownFunction"; // In C++ we cannot easily get function name like in C#
		}
		std::string Ntitle = title;			  ///< 最终使用的标题行（可能被自动生成替换）
		std::string Nrelmessage = relmessage; ///< 最终使用的发布模式消息
		std::string Nmessage = message;		  ///< 最终使用的调试模式消息（含函数名前缀）

		Nmessage = functionName + " ERROR! " + message;

		if (title == "")
		{
			Ntitle = "*********************! " + OtherHelper::FillString(OtherHelper::GetCurrentProjectName() + ".RUN.ERROR", " ", 1) + " !*********************\n";
		}

		std::cout << '\n';
		if (relmessage == "")
		{
			Nrelmessage = message;
		}

		if (mode == 0)
		{
			WriteLog(Nmessage, "", true, Ntitle, ConsoleColor::Red);
			Console::ResetColor();
		}
		else
		{
			WriteLog("", Nrelmessage, true, Ntitle, ConsoleColor::Red);
			Console::ResetColor();
		}
#ifdef _DEBUG
		// LogHelper::ErrorLog(Nmessage);
		// return;
		EndProgram(true);
#else
		/// LogHelper::ErrorLog(formattedMessage);
		EndProgram(true);
#endif
	}

	/**
	 * @brief 向控制台输出格式化日志消息，同时追加到内存日志列表
	 *
	 * @details 核心日志输出函数，所有其他 WriteLog/DebugLog 最终委托此函数执行。
	 * 行为模式：
	 * - mode=0（Debug）+ show_title=true：仅输出 message（不输出标题行），兼容调试器
	 * - mode=0（Debug）+ show_title=false：直接输出 message，带可选颜色
	 * - mode=1（Release）+ show_title=true：换行后输出标题行 + displayMessage
	 * - mode=1（Release）+ show_title=false：直接输出 displayMessage，带可选颜色
	 * displayMessage 优先取 relaesemes（若非空），否则取 message。
	 *
	 * @param message     调试模式显示的完整消息
	 * @param relaesemes  发布模式显示的简洁消息（空字符串则使用 message）
	 * @param show_title  是否在消息前显示标题标签（如 "[Message]"）
	 * @param title       标题标签字符串，仅当 show_title=true 时使用
	 * @param color       控制台前景颜色（ConsoleColor::White 时不设置颜色）
	 * @param newline     消息末尾是否换行（true=换行，false=不换行）
	 * @param leval       标题前的缩进空格数（0=不缩进）
	 *
	 * @code
	 * // 普通消息，绿色，换行：
	 * LogHelper::WriteLog("网格划分完成，共 1024 个单元", "", false, "", ConsoleColor::Green);
	 *
	 * // 带标题标签，蓝色：
	 * LogHelper::WriteLog("读取叶片参数...", "", true, "[IO]", ConsoleColor::Cyan);
	 * // 输出：
	 * // [IO] 读取叶片参数...
	 * @endcode
	 */
	void LogHelper::WriteLog(const std::string &message, const std::string &relaesemes,
							 bool show_title, const std::string &title, ConsoleColor color,
							 bool newline, int leval)
	{
		std::string displayMessage = relaesemes.empty() ? message : relaesemes; ///< 实际显示的消息（发布模式优先用 relaesemes）

		std::string levelPadding(leval, ' '); ///< 缩进填充字符串（leval 个空格）

		if (mode == 0)
		{
			if (show_title)
			{
#ifdef _DEBUG
				if (newline)
				{
					std::cout << " " << message << '\n';
				}
				else
				{
					std::cout << " " << message;
				}
#else
				std::cout << '\n';
				if (color != ConsoleColor::White)
				{
					Console::SetForegroundColor(color);
				}
				std::cout << levelPadding << title;
				if (newline)
				{
					std::cout << " " << message << '\n';
				}
				else
				{
					std::cout << " " << message;
				}
				Console::ResetColor();
#endif
			}
			else
			{
				if (color != ConsoleColor::White)
				{
					Console::SetForegroundColor(color);
				}
				if (newline)
				{
					std::cout << message << '\n';
				}
				else
				{
					std::cout << message;
				}
				Console::ResetColor();
			}
			LogData::Add(message);
		}
		else
		{
			if (show_title)
			{
				std::cout << '\n';
				if (color != ConsoleColor::White)
				{
					Console::SetForegroundColor(color);
				}
				std::cout << levelPadding << title;
				if (newline)
				{
					std::cout << " " << displayMessage << '\n';
				}
				else
				{
					std::cout << " " << displayMessage;
				}
				Console::ResetColor();
			}
			else
			{
				if (color != ConsoleColor::White)
				{
					Console::SetForegroundColor(color);
				}
				if (newline)
				{
					std::cout << displayMessage << '\n';
				}
				else
				{
					std::cout << displayMessage;
				}
				Console::ResetColor();
			}
			LogData::Add(displayMessage);
		}
	}

	/**
	 * @brief 输出单精度 Eigen 矩阵到控制台和日志（重载版本）
	 *
	 * @param message 要输出的单精度浮点矩阵
	 *
	 * @code
	 * Eigen::MatrixXf m = Eigen::MatrixXf::Random(3, 3);
	 * LogHelper::WriteLog(m);
	 * // 输出：[Message]  <3x3 矩阵数据>
	 * @endcode
	 */
	void LogHelper::WriteLog(const Eigen::MatrixXf &message)
	{
		std::cout << "[Message]  " << message << '\n';
		std::stringstream ss; ///< 用于将矩阵序列化为字符串以存入日志
		ss << message;
		LogData::Add(ss.str());
	}

	/**
	 * @brief 输出双精度 Eigen 矩阵到控制台和日志（重载版本）
	 *
	 * @param message 要输出的双精度浮点矩阵
	 *
	 * @code
	 * Eigen::MatrixXd m = Eigen::MatrixXd::Identity(4, 4);
	 * LogHelper::WriteLog(m);
	 * @endcode
	 */
	void LogHelper::WriteLog(const Eigen::MatrixXd &message)
	{
		std::cout << "[Message]  " << message << '\n';
		std::stringstream ss; ///< 用于将矩阵序列化为字符串以存入日志
		ss << message;
		LogData::Add(ss.str());
	}

	/**
	 * @brief 输出 double 型 std::vector 到控制台和日志（重载版本）
	 *
	 * @details 将向量所有元素以空格分隔拼接后输出，同时存入日志。
	 *
	 * @param message 要输出的双精度浮点向量
	 *
	 * @code
	 * std::vector<double> v = {1.1, 2.2, 3.3};
	 * LogHelper::WriteLog(v);
	 * // 输出：" 1.1 2.2 3.3"
	 * @endcode
	 */
	void LogHelper::WriteLog(const std::vector<double> &message)
	{
		WriteLog("");
		std::stringstream ss;
		for (size_t i = 0; i < message.size(); ++i)
		{
			ss << " " << message[i];
		}
		std::string messageStr = ss.str(); ///< 拼接后的字符串（各元素以空格分隔）
		std::cout << messageStr << '\n';
		LogData::Add(messageStr);
	}

	/**
	 * @brief 输出 float 型 std::vector 到控制台和日志（重载版本）
	 *
	 * @param message 要输出的单精度浮点向量
	 *
	 * @code
	 * std::vector<float> v = {1.0f, 2.0f, 3.0f};
	 * LogHelper::WriteLog(v);
	 * @endcode
	 */
	void LogHelper::WriteLog(const std::vector<float> &message)
	{
		WriteLog("");
		std::stringstream ss;
		for (size_t i = 0; i < message.size(); ++i)
		{
			ss << " " << message[i];
		}
		std::string messageStr = ss.str();
		std::cout << messageStr << '\n';
		LogData::Add(messageStr);
	}

	/**
	 * @brief 输出单精度 Eigen 向量到控制台和日志（重载版本）
	 *
	 * @param message 要输出的单精度 Eigen 列向量
	 *
	 * @code
	 * Eigen::VectorXf v = Eigen::VectorXf::LinSpaced(5, 0.0f, 1.0f);
	 * LogHelper::WriteLog(v);
	 * // 输出：" 0 0.25 0.5 0.75 1"
	 * @endcode
	 */
	void LogHelper::WriteLog(const Eigen::VectorXf &message)
	{
		WriteLog("");
		std::stringstream ss; ///< 用于将向量元素拼接为字符串
		for (int i = 0; i < message.size(); ++i)
		{
			ss << " " << message[i];
		}
		std::string messageStr = ss.str(); ///< 空格分隔的向量元素字符串
		std::cout << messageStr << '\n';
		LogData::Add(messageStr);
	}

	/**
	 * @brief 输出双精度 Eigen 向量到控制台和日志（重载版本）
	 *
	 * @param message 要输出的双精度 Eigen 列向量
	 *
	 * @code
	 * Eigen::VectorXd result = model.GetResponse();
	 * LogHelper::WriteLog(result);
	 * @endcode
	 */
	void LogHelper::WriteLog(const Eigen::VectorXd &message)
	{
		WriteLog("");
		std::stringstream ss;
		for (int i = 0; i < message.size(); ++i)
		{
			ss << " " << message[i];
		}
		std::string messageStr = ss.str();
		std::cout << messageStr << '\n';
		LogData::Add(messageStr);
	}

	/**
	 * @brief 输出调试级别日志（与 WriteLog 参数接口相同，但语义上专用于调试信息）
	 *
	 * @details 行为与 WriteLog 相同，但命名语义上区分两者用途：
	 * - WriteLog：业务流程的常规信息输出（始终可见）
	 * - DebugLog：调试诊断信息输出（考虑未来可按级别过滤）
	 *
	 * @param message     调试模式下的详细消息
	 * @param relaesemes  发布模式下的简洁消息（空则使用 message）
	 * @param show_title  是否在消息前加标题标签
	 * @param title       标题标签字符串
	 * @param color       控制台输出颜色
	 * @param newline     是否换行
	 * @param leval       缩进级别（空格数）
	 *
	 * @code
	 * // 调试输出迭代器状态：
	 * LogHelper::DebugLog(
	 *     "迭代第 " + std::to_string(iter) + " 步，残差 = " + std::to_string(residual),
	 *     "", false, "", ConsoleColor::Cyan
	 * );
	 * @endcode
	 */
	void LogHelper::DebugLog(const std::string &message, const std::string &relaesemes,
							 bool show_title, const std::string &title, ConsoleColor color,
							 bool newline, int leval)
	{
		std::string displayMessage = relaesemes.empty() ? message : relaesemes; ///< 实际显示的消息
		std::string levelPadding(leval, ' ');									///< 缩进填充字符串

		if (mode == 0)
		{
			if (show_title)
			{
#ifdef _DEBUG
				if (newline)
				{
					std::cout << " " << message << '\n';
				}
				else
				{
					std::cout << " " << message;
				}
#else
				if (color != ConsoleColor::White)
				{
					Console::SetForegroundColor(color);
				}
				std::cout << levelPadding << title;
				if (newline)
				{
					std::cout << " " << message << '\n';
				}
				else
				{
					std::cout << " " << message;
				}
				Console::ResetColor();
#endif
			}
			else
			{
				if (color != ConsoleColor::White)
				{
					Console::SetForegroundColor(color);
				}
				if (newline)
				{
					std::cout << message << '\n';
				}
				else
				{
					std::cout << message;
				}
				Console::ResetColor();
			}
			LogData::Add(message);
		}
		else
		{
			if (show_title)
			{
				std::cout << '\n';
				if (color != ConsoleColor::White)
				{
					Console::SetForegroundColor(color);
				}
				std::cout << levelPadding << title;
				if (newline)
				{
					std::cout << " " << displayMessage << '\n';
				}
				else
				{
					std::cout << " " << displayMessage;
				}
				Console::ResetColor();
			}
			else
			{
				if (color != ConsoleColor::White)
				{
					Console::SetForegroundColor(color);
				}
				if (newline)
				{
					std::cout << displayMessage << '\n';
				}
				else
				{
					std::cout << displayMessage;
				}
				Console::ResetColor();
			}
			LogData::Add(displayMessage);
		}
	}

	/**
	 * @brief 以调试方式输出单精度 Eigen 矩阵（重载版本）
	 *
	 * @param message 要调试输出的单精度浮点矩阵
	 *
	 * @code
	 * Eigen::MatrixXf K = stiffness_matrix.cast<float>();
	 * LogHelper::DebugLog(K); // 输出刚度矩阵供调试
	 * @endcode
	 */
	void LogHelper::DebugLog(const Eigen::MatrixXf &message)
	{
		std::cout << "[Message]  " << message << '\n';
		std::stringstream ss; ///< 矩阵序列化缓冲区
		ss << message;
		LogData::Add(ss.str());
	}

	/**
	 * @brief 以调试方式输出双精度 Eigen 矩阵（重载版本）
	 *
	 * @param message 要调试输出的双精度浮点矩阵
	 *
	 * @code
	 * Eigen::MatrixXd M = mass_matrix;
	 * LogHelper::DebugLog(M); // 输出质量矩阵供调试
	 * @endcode
	 */
	void LogHelper::DebugLog(const Eigen::MatrixXd &message)
	{
		std::cout << "[Message]  " << message << '\n';
		std::stringstream ss; ///< 矩阵序列化缓冲区
		ss << message;
		LogData::Add(ss.str());
	}

	/**
	 * @brief 以调试方式输出 double 型 std::vector（重载版本）
	 *
	 * @param message 要调试输出的双精度浮点向量
	 *
	 * @code
	 * std::vector<double> angles = {0.0, 10.5, 21.3, 35.7};
	 * LogHelper::DebugLog(angles); // 输出翼型攻角列表
	 * @endcode
	 */
	void LogHelper::DebugLog(const std::vector<double> &message)
	{
		DebugLog("");
		std::stringstream ss;
		for (size_t i = 0; i < message.size(); ++i)
		{
			ss << " " << message[i];
		}
		std::string messageStr = ss.str();
		std::cout << messageStr << '\n';
		LogData::Add(messageStr);
	}

	/**
	 * @brief 以调试方式输出 float 型 std::vector（重载版本）
	 *
	 * @param message 要调试输出的单精度浮点向量
	 *
	 * @code
	 * std::vector<float> loads = {12.5f, 23.4f, 8.9f};
	 * LogHelper::DebugLog(loads);
	 * @endcode
	 */
	void LogHelper::DebugLog(const std::vector<float> &message)
	{
		DebugLog("");
		std::stringstream ss; ///< 用于拼接向量元素
		for (size_t i = 0; i < message.size(); ++i)
		{
			ss << " " << message[i];
		}
		std::string messageStr = ss.str();
		std::cout << messageStr << '\n';
		LogData::Add(messageStr);
	}

	/**
	 * @brief 以调试方式输出单精度 Eigen 向量（重载版本）
	 *
	 * @param message 要调试输出的单精度 Eigen 列向量
	 *
	 * @code
	 * Eigen::VectorXf forces = blade_forces.cast<float>();
	 * LogHelper::DebugLog(forces);
	 * @endcode
	 */
	void LogHelper::DebugLog(const Eigen::VectorXf &message)
	{
		DebugLog("");
		std::stringstream ss; ///< 向量元素序列化缓冲区
		for (int i = 0; i < message.size(); ++i)
		{
			ss << " " << message[i];
		}
		std::string messageStr = ss.str();
		std::cout << messageStr << '\n';
		LogData::Add(messageStr);
	}

	/**
	 * @brief 以调试方式输出双精度 Eigen 向量（重载版本）
	 *
	 * @param message 要调试输出的双精度 Eigen 列向量
	 *
	 * @code
	 * Eigen::VectorXd displacements = fem_solver.Solve();
	 * LogHelper::DebugLog(displacements); // 输出位移向量以验证求解结果
	 * @endcode
	 */
	void LogHelper::DebugLog(const Eigen::VectorXd &message)
	{
		DebugLog("");
		std::stringstream ss; ///< 向量元素序列化缓冲区
		for (int i = 0; i < message.size(); ++i)
		{
			ss << " " << message[i];
		}
		std::string messageStr = ss.str();
		std::cout << messageStr << '\n';
		LogData::Add(messageStr);
	}

	/**
	 * @brief 将单精度 Eigen 向量写入日志文件（不输出到控制台）
	 *
	 * @details "O" 代表 Output-only（仅写入日志文件，不显示到控制台）。
	 * 元素之间以制表符 '\t' 分隔，适配表格格式的日志文件。
	 *
	 * @param message 要写入的单精度 Eigen 向量
	 *
	 * @code
	 * Eigen::VectorXf rotor_speed = sim.GetRotorSpeed();
	 * LogHelper::WriteLogO(rotor_speed); // 静默写入日志，不干扰控制台输出
	 * @endcode
	 */
	void LogHelper::WriteLogO(const Eigen::VectorXf &message)
	{
		std::stringstream ss; ///< 向量序列化缓冲区（制表符分隔）
		for (int i = 0; i < message.size(); ++i)
		{
			ss << '\t' << message[i];
		}
		LogData::Add(ss.str());
	}

	/**
	 * @brief 将任意类型的 std::vector 写入日志文件（模板版本，不输出到控制台）
	 *
	 * @details 模板函数，支持 string/int/float/double 等类型。
	 * 通过显式模板实例化支持四种常用类型，元素以制表符 '\t' 分隔。
	 *
	 * @tparam T 向量元素类型（已实例化：std::string, int, float, double）
	 * @param message 要写入的类型为 T 的 std::vector
	 *
	 * @code
	 * std::vector<std::string> headers = {"时间", "位移", "速度", "加速度"};
	 * LogHelper::WriteLogO(headers); // 写入表头行
	 *
	 * std::vector<double> row = {0.0, 1.23, 4.56, 7.89};
	 * LogHelper::WriteLogO(row); // 写入数据行
	 * @endcode
	 */
	template <typename T>
	void LogHelper::WriteLogO(const std::vector<T> &message)
	{
		std::stringstream ss;
		for (const auto &item : message)
		{
			ss << '\t' << item;
		}
		LogData::Add(ss.str());
	}

	// Explicit template instantiations
	template void LogHelper::WriteLogO<std::string>(const std::vector<std::string> &);
	template void LogHelper::WriteLogO<int>(const std::vector<int> &);
	template void LogHelper::WriteLogO<float>(const std::vector<float> &);
	template void LogHelper::WriteLogO<double>(const std::vector<double> &);

	/**
	 * @brief 将双精度 Eigen 向量写入日志文件（不输出到控制台）
	 *
	 * @param message 要写入的双精度 Eigen 列向量
	 *
	 * @code
	 * Eigen::VectorXd blade_deflection = aeroelastic.GetDeflection();
	 * LogHelper::WriteLogO(blade_deflection);
	 * @endcode
	 */
	void LogHelper::WriteLogO(const Eigen::VectorXd &message)
	{
		std::stringstream ss; ///< 向量序列化缓冲区（空格分隔）
		for (int i = 0; i < message.size(); ++i)
		{
			ss << " " << message[i];
		}
		LogData::Add(ss.str());
	}

	/**
	 * @brief 将字符串消息写入日志文件（可选带标题前缀，不输出到控制台）
	 *
	 * @details 与 WriteLog 不同，WriteLogO 系列不在控制台输出，
	 * 适用于高频数据记录（如每个时间步的仿真结果）。
	 *
	 * @param message  要写入日志的字符串消息
	 * @param show_log 是否添加标题前缀（true 时格式为 "title: message"）
	 * @param title    标题前缀字符串（仅当 show_log=true 时使用）
	 *
	 * @code
	 * // 不带标题：
	 * LogHelper::WriteLogO("step 100 completed", false);
	 *
	 * // 带标题：
	 * LogHelper::WriteLogO("3200 rpm", true, "[RotorSpeed]");
	 * // 写入日志："[RotorSpeed]: 3200 rpm"
	 * @endcode
	 */
	void LogHelper::WriteLogO(const std::string &message, bool show_log, const std::string &title)
	{
		std::string formattedMessage = message;
		if (show_log)
		{
			formattedMessage = title + ": " + message;
		}
		LogData::Add(formattedMessage);
	}

	/**
	 * @brief 将单精度 Eigen 矩阵写入日志文件（不输出到控制台）
	 *
	 * @param message 要写入的单精度浮点矩阵
	 *
	 * @code
	 * Eigen::MatrixXf Kf = stiffness.cast<float>();
	 * LogHelper::WriteLogO(Kf);
	 * @endcode
	 */
	void LogHelper::WriteLogO(const Eigen::MatrixXf &message)
	{
		std::stringstream ss; ///< 矩阵序列化缓冲区
		ss << "[Message]  " << message;
		LogData::Add(ss.str());
	}

	/**
	 * @brief 将双精度 Eigen 矩阵写入日志文件（不输出到控制台）
	 *
	 * @param message 要写入的双精度浮点矩阵
	 *
	 * @code
	 * Eigen::MatrixXd M = mass_matrix;
	 * LogHelper::WriteLogO(M); // 将质量矩阵记录到日志文件
	 * @endcode
	 */
	void LogHelper::WriteLogO(const Eigen::MatrixXd &message)
	{
		std::stringstream ss; ///< 矩阵序列化缓冲区
		ss << "[Message]  " << message;
		LogData::Add(ss.str());
	}

	/**
	 * @brief 将 double 标量写入日志文件（不输出到控制台）
	 *
	 * @param message 要记录的双精度浮点数值
	 *
	 * @code
	 * double power = turbine.GetPower();
	 * LogHelper::WriteLogO(power); // 记录当前发电功率
	 * @endcode
	 */
	void LogHelper::WriteLogO(double message)
	{
		LogData::Add("[Message]  " + std::to_string(message));
	}

	/**
	 * @brief 将 float 标量写入日志文件（不输出到控制台）
	 *
	 * @param message 要记录的单精度浮点数值
	 *
	 * @code
	 * float thrust = (float)turbine.GetThrust();
	 * LogHelper::WriteLogO(thrust);
	 * @endcode
	 */
	void LogHelper::WriteLogO(float message)
	{
		LogData::Add("[Message]  " + std::to_string(message));
	}

	/**
	 * @brief 将 double 型 std::vector 写入日志文件（不输出到控制台）
	 *
	 * @details 元素以空格分隔，整个向量写为日志文件的一行。
	 * 适用于高频仿真数据的批量写入（如每时间步的力、位移数组）。
	 *
	 * @param message 要写入的双精度浮点向量
	 *
	 * @code
	 * std::vector<double> time_step_data = sim.GetCurrentStepData();
	 * LogHelper::WriteLogO(time_step_data);
	 * @endcode
	 */
	void LogHelper::WriteLogO(const std::vector<double> &message)
	{
		std::stringstream ss;
		for (size_t i = 0; i < message.size(); ++i)
		{
			ss << " " << message[i];
		}
		LogData::Add(ss.str());
	}

	/**
	 * @brief 将 float 型 std::vector 写入日志文件（不输出到控制台）
	 *
	 * @param message 要写入的单精度浮点向量
	 *
	 * @code
	 * std::vector<float> floatData = getSimResults();
	 * LogHelper::WriteLogO(floatData);
	 * @endcode
	 */
	void LogHelper::WriteLogO(const std::vector<float> &message)
	{
		std::stringstream ss; ///< 向量序列化缓冲区（空格分隔）
		for (size_t i = 0; i < message.size(); ++i)
		{
			ss << " " << message[i];
		}
		LogData::Add(ss.str());
	}

} // namespace Log
