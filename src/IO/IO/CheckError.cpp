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

#include "CheckError.h"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include "../Log/LogHelper.h"
#include "../IO/Extensions.h"

namespace fs = std::filesystem;
using namespace Qahse::IO::Log;

namespace Qahse::IO::IO
{

	/**
	 * @brief 用于记录波浪文件超时次数的静态变量。
	 * @details 每当仿真时间超过波浪文件最大时间时自增。
	 */
	int CheckError::Checkwavetime_son = 1;

	/**
	 * @brief 控制是否在仿真时间超出波浪文件时输出提示的静态变量。
	 * @details true 表示输出提示，false 表示不输出。
	 */
	bool CheckError::Checkwavetime_show = true;

	/**
	 * @brief 检查仿真时间t是否超出波浪文件时间范围，并根据参数处理。
	 * @param timespan 波浪文件的时间序列（单位：秒）。
	 * @param t 当前仿真时间，若超出范围会被重置。
	 * @param ContinueCal 是否允许循环读取波浪文件（true允许，false报错）。
	 * @details 若仿真时间t超过波浪文件最大时间，根据ContinueCal决定是否循环。
	 * @code
	 * std::vector<double> timespan = {0, 1, 2, 3};
	 * double t = 5.5;
	 * CheckError::Checkwavetime(timespan, t, true); // t会被重置到范围内
	 * @endcode
	 */
	void CheckError::Checkwavetime(const std::vector<double> &timespan, double &t, bool ContinueCal)
	{
		if (timespan.empty())
			return; ///< 时间序列为空直接返回

		t = t > 0 ? t - 0.0000001 : t; ///< 避免精度误差导致边界问题
		if (t < 0)
		{
			LogHelper::WarnLog("仿真时间小于0,请检查", "", ConsoleColor::DarkYellow, 0, "CheckError::Checkwavetime");
			return;
		}
		double ZHENSHU = std::floor(t / timespan.back()); ///< 计算超出周期数

		if (ZHENSHU != 0)
		{
			if (ContinueCal)
			{
				if (ZHENSHU == Checkwavetime_son)
				{
					if (Checkwavetime_show)
					{
						std::cout << '\n';
						Checkwavetime_son++;
						LogHelper::WarnLog("Current Time is the " + std::to_string(ZHENSHU) +
											   " times exceed wave file, we will read file at start",
										   "Checkwavetime");
					}
				}
				t = t - ZHENSHU * timespan.back(); ///< 循环重置仿真时间
			}
			else
			{
				LogHelper::ErrorLog("仿真时间超过波浪文件最大时间" +
										std::to_string(*std::max_element(timespan.begin(), timespan.end())) +
										"s 请将windL的CreadW设置为true再试",
									"", "", 20, "CheckError::Checkwavetime");
			}
		}
	}

	/**
	 * @brief 检查指定文件路径的父目录是否存在，不存在时可选择自动创建。
	 * @param name 文件路径。
	 * @param createdir 是否自动创建父目录。
	 * @details 若父目录不存在且createdir为true，则尝试自动创建。
	 * @code
	 * CheckError::CheckPath("./data/test.txt", true);
	 * @endcode
	 */
	void CheckError::CheckPath(const std::string &name, bool createdir)
	{
		std::string name1 = Extensions::GetABSPath(name); ///< 获取绝对路径
		fs::path path(name1);
		std::string temp = path.parent_path().string(); ///< 父目录路径

		if (temp.empty())
		{
			LogHelper::ErrorLog("文件路径：" + name1 + "文件的父文件夹为空！或路径定义错误！", "", "", 20, "CheckError::CheckPath");
			return;
		}

		if (!fs::exists(temp))
		{
			LogHelper::WriteLogO("文件路径：" + temp + "没有不存在！");
			if (createdir)
			{
				try
				{
					fs::create_directories(temp); ///< 自动创建父目录
				}
				catch (const std::exception &)
				{
					LogHelper::WarnLog("您的权限不够！", "CheckPath");
					LogHelper::ErrorLog("文件路径：" + temp + "没有不存在,且系统权限不够，无法创建", "", "", 20, "CheckError::CheckPath");
				}
			}
		}
	}

	/**
	 * @brief 检查指定目录是否存在，不存在时可选择自动创建。
	 * @param name 目录路径。
	 * @param createDir 是否自动创建目录。
	 * @details 若目录不存在且createDir为true，则尝试自动创建。
	 * @code
	 * CheckError::CheckDir("./output", true);
	 * @endcode
	 */
	void CheckError::CheckDir(const std::string &name, bool createDir)
	{
		if (!fs::exists(name))
		{
			LogHelper::WriteLogO("文件夹路径：" + name + "不存在！");
			if (createDir)
			{
				try
				{
					fs::create_directories(name); ///< 自动创建目录
				}
				catch (const std::exception &)
				{
					LogHelper::WarnLog("您的权限不够！", "CheckDir");
					LogHelper::ErrorLog("文件路径：" + name + "没有不存在,且系统权限不够，无法创建", "", "", 20, "CheckError::CheckDir");
				}
			}
		}
	}

	/**
	 * @brief 检查函数指针是否为空。
	 * @param ptr 待检查的指针。
	 * @param name 函数名。
	 * @param dllpath DLL路径。
	 * @details 若ptr为nullptr则输出错误日志。
	 * @code
	 * void* funcPtr = GetProcAddress(...);
	 * CheckError::CheckInptr(funcPtr, "MyFunc", "./lib.dll");
	 * @endcode
	 */
	void CheckError::CheckInptr(void *ptr, const std::string &name, const std::string &dllpath)
	{
		if (ptr == nullptr)
		{
			LogHelper::ErrorLog("Cant find function name: " + name + " in dll path:\n  " + dllpath, "", "", 20, "CheckError::CheckInptr");
		}
	}

	/**
	 * @brief 检查并修正文件路径的扩展名。
	 * @param path 文件路径（引用，可能被修改）。
	 * @param extension1 期望的扩展名（如".txt"）。
	 * @param show 是否输出警告。
	 * @param information 警告信息前缀。
	 * @details 若扩展名为空则自动补全，若不一致则报错。
	 * @code
	 * std::string path = "result";
	 * CheckError::CheckPath(path, ".dat"); // path会变为"result.dat"
	 * @endcode
	 */
	void CheckError::CheckPath(std::string &path, const std::string &extension1,
							   bool show, const std::string &information)
	{
		fs::path filePath(path);
		std::string extension = filePath.extension().string();

		if (extension.empty())
		{
			if (show)
			{
				LogHelper::WarnLog(information + extension1, "CheckPath");
			}
			else
			{
				LogHelper::WriteLogO(information + extension1);
			}
			path += extension1; ///< 自动补全扩展名
		}
		else if (extension != extension1)
		{
			LogHelper::ErrorLog("当前文件的拓展名称" + extension + " 与指定名称" +
									extension1 + " 不同！请检查后再尝试",
								"", "", 20, "CheckError::CheckPath");
		}
	}

	/**
	 * @brief 检查指定文件是否存在。
	 * @param filePath 文件路径。
	 * @param error 不存在时是否输出错误。
	 * @param showwaring 不存在时是否输出警告。
	 * @param inf 自定义提示信息。
	 * @param FunctionName 调用函数名。
	 * @return 存在返回true，否则false。
	 * @code
	 * bool ok = CheckError::Filexists("./data/input.txt");
	 * @endcode
	 */
	bool CheckError::Filexists(const std::string &filePath, bool error, bool showwaring,
							   const std::string &inf, const std::string &FunctionName)
	{
		if (fs::exists(filePath))
		{
			return true;
		}

		std::string message = inf.empty() ? "当前文件：" + filePath + "不存在！请检查路径" : inf;

		if (error)
		{
			LogHelper::ErrorLog(FunctionName + ":" + message, "", "", 20, "CheckError::Filexists");
		}
		else if (showwaring)
		{
			LogHelper::WarnLog(FunctionName + ":" + message, "", ConsoleColor::DarkYellow, 0, "Filexists");
		}
		else
		{
			LogHelper::WriteLogO(FunctionName + ":" + message);
		}
		return false;
	}

	/**
	 * @brief 检查并修正文件路径，必要时自动创建目录。
	 * @param oripath 原始文件路径。
	 * @param Path 目标路径（引用，可能被修改）。
	 * @param createdir 是否自动创建目录。
	 * @param extren 期望的扩展名。
	 * @param outfile 是否为输出文件。
	 * @param error 不存在时是否输出错误。
	 * @details 若目标路径不存在则尝试修正并创建。
	 * @code
	 * std::string path = "result";
	 * CheckError::Filexists("./data/input.txt", path, true, ".dat");
	 * @endcode
	 */
	void CheckError::Filexists(const std::string &oripath, std::string &Path, bool createdir,
							   const std::string &extren, bool outfile, bool error)
	{
		CheckPath(Path, extren, false); ///< 检查扩展名
		CheckPath(Path, createdir);		///< 检查并创建目录

		fs::path originalPath(oripath);
		std::string oriDir = originalPath.parent_path().string();

		if (!fs::exists(Path))
		{
			fs::path newPath = fs::path(oriDir) / Path;
			Path = fs::absolute(newPath).string();
			CheckPath(Path, createdir);

			if (!outfile && error && !fs::exists(Path))
			{
				LogHelper::ErrorLog("当前文件： " + oripath +
										" \n   当中的路径参数： " + Path + " 无法读取",
									"", "", 20, "CheckError::Filexists");
			}
		}
		else
		{
			Path = fs::absolute(Path).string(); ///< 绝对化路径
		}
	}
}