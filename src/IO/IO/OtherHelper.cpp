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

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <thread>
#include <filesystem>
#include <chrono>
#include <regex>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

// #include <windows.h>

#include <process.h>
// #include <Psapi.h>
#include <io.h>
#include <fcntl.h>

#pragma comment(lib, "version.lib")
#pragma comment(lib, "psapi.lib")
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <dlfcn.h>
#endif

#include <Eigen/Dense>
#include "OtherHelper.h"
#include "../Log/LogHelper.h"
#include "../io/Extensions.h"

using namespace Qahse::IO::Log;
using namespace Qahse::IO::IO;

namespace Qahse::IO::IO
{

	// ProgressBar 类实现
	/**
	 * @brief 构造并初始化进度条对象。
	 * @param totalIterations 总迭代步数（用于计算百分比）。
	 * @param level           缩进级别，每级缩进 2 个空格。
	 * @code
	 * OtherHelper::ProgressBar bar(100, 0);
	 * for(int i=0; i<100; i++) bar.UpdateProgress();
	 * @endcode
	 */
	OtherHelper::ProgressBar::ProgressBar(int totalIterations, int level)
		: _totalIterations(totalIterations), _currentIteration(0)
	{
		levels = std::string(level * 2, ' '); ///< 缩进字符串，每个级别缩进 2 个空格
	}

	/**
	 * @brief 创建并更新进度条显示，将当前进度刷新输出到控制台。
	 * @details 每次调用将 `_currentIteration` 加 1，然后使用 `\r` 覆盖当前行输出进度条。
	 *          当迭代完成时（`_currentIteration == _totalIterations`）自动换行。
	 */
	void OtherHelper::ProgressBar::UpdateProgress()
	{
		_currentIteration++; ///< 当前迭代计数器，每调用一次加 1

		float progress = static_cast<float>(_currentIteration) / _totalIterations; ///< 当前进度百分比 [0.0, 1.0]
		int barLength = static_cast<int>(progress * TotalBars);					   ///< 已完成的进度条段长度

		std::cout << "\r" << levels << "[";

		for (int i = 0; i < TotalBars; ++i)
		{
			if (i < barLength)
			{
				std::cout << ProgressBarChar;
			}
			else
			{
				std::cout << BackgroundChar;
			}
		}

		std::cout << "] " << static_cast<int>(progress * 100) << "% "
				  << "(" << _currentIteration << "/" << _totalIterations << ")";

		if (_currentIteration == _totalIterations)
		{
			std::cout << '\n';
		}

		std::cout.flush();
	}

	// 主要功能方法实现
	/**
	 * @brief 将相对路径转换为绝对路径并规范化格式。
	 * @param path 要处理的文件路径（相对或绝对）。
	 * @return 规范化后的绝对路径字符串。
	 * @code
	 * std::string abs = OtherHelper::FormortPath("..\\data\\file.txt");
	 * // abs == "C:\\project\\data\\file.txt"  (示例)
	 * @endcode
	 */
	std::string OtherHelper::FormortPath(const std::string &path)
	{
		try
		{
			std::filesystem::path fsPath(path);
			return std::filesystem::absolute(fsPath).string();
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error formatting path: " << e.what() << '\n';
			return path;
		}
	}

	/**
	 * @brief 在字符串列表中找到与目标字符串编辑距离最小的元素。
	 * @details 使用 Levenshtein 编辑距离算法计算相似度。
	 * @param strings 候选字符串列表。
	 * @param target  目标匹配字符串。
	 * @return `std::tuple<int, std::string>`，第一项为最佳匹配的索引，第二项为匹配的字符串内容。
	 *          若列表为空，返回 `{-1, ""}`。
	 * @code
	 * auto [idx, val] = OtherHelper::FindBestMatch({"alpha", "beta", "gamma"}, "alpa");
	 * // idx == 0, val == "alpha"
	 * @endcode
	 */
	std::tuple<int, std::string> OtherHelper::FindBestMatch(const std::vector<std::string> &strings, const std::string &target)
	{
		if (strings.empty())
		{
			return std::make_tuple(-1, "");
		}

		int bestIndex = 0;
		int minDistance = LevenshteinDistance(strings[0], target);

		for (size_t i = 1; i < strings.size(); ++i)
		{
			int distance = LevenshteinDistance(strings[i], target);
			if (distance < minDistance)
			{
				minDistance = distance;
				bestIndex = static_cast<int>(i);
			}
		}

		return std::make_tuple(bestIndex, strings[bestIndex]);
	}

	//	void OtherHelper::SetMathAcc(const std::string& acc)
	//	{
	//		//将字符串转换为小写
	//        auto acc1 = ToLowerString(acc);
	//		if (acc1 == "mkl")
	//		{
	// #define EIGEN_USE_MKL_ALL
	//		}
	//		else if (acc1 == "openblas"|| acc1 == "blas")
	//		{
	// #define EIGEN_USE_BLAS
	//		}
	//		else if (acc1 == "cuda")
	//		{
	// #define EIGEN_USE_CUDA
	//		}
	//		else if (acc1 == "none")
	//		{
	// #undef EIGEN_USE_MKL_ALL
	// #undef EIGEN_USE_BLAS
	// #undef EIGEN_USE_CUDA
	//		}
	//		else
	//		{
	//			LogHelper::ErrorLog("未知加速器！" + acc1, "", "", 20, "OtherHelper::Usemkl");
	//		}
	//	}

	/**
	 * @brief 获取当前编译时开启的 Eigen 数学加速库名称。
	 * @return 加速库名称字符串，可能的返回値：
	 *  - \"MKL\"：Intel® Math Kernel Library
	 *  - \"Blas\"：OpenBLAS
	 *  - \"CUDA\"：NVIDIA CUDA
	 *  - \"none\"：GPU 加速
	 *  - \"pps\"：纯硬件实现（无加速）
	 * @code
	 * std::string acc = OtherHelper::GetMathAcc();
	 * // 如 acc == "MKL"
	 * @endcode
	 */
	std::string OtherHelper::GetMathAcc()
	{
#if defined(EIGEN_USE_MKL_ALL)
		return "MKL";
#elif defined(EIGEN_USE_BLAS)
		return "Blas";
#elif defined(EIGEN_USE_CUDA)
		return "CUDA";
#elif defined(EIGEN_USE_GPU)
		return "none";
#else
		return "pps";
#endif
	}

	/**
	 * @brief 使用系统命令调用外部可执行文件，并传入一个文件路径参数。
	 * @param exePath  可执行文件的路径。
	 * @param filePath 作为参数传入的文件路径。
	 * @code
	 * OtherHelper::SysRun("C:\\tools\\calc.exe", "data.txt");
	 * @endcode
	 */
	void OtherHelper::SysRun(const std::string &exePath, const std::string &filePath)
	{
		std::string command = "\"" + exePath + "\" \"" + filePath + "\"";

		int result = std::system(command.c_str());
		if (result != 0)
		{
			std::cerr << "Failed to execute: " << command << '\n';
		}
	}

	/**
	 * @brief 在 Windows 命令提示符中运行一个命令。
	 * @param command 要执行的 cmd 命令字符串。
	 * @code
	 * OtherHelper::RunCmd("echo Hello World");
	 * @endcode
	 */
	void OtherHelper::RunCmd(const std::string &command)
	{
		std::string fullCommand = "cmd /c " + command;

		std::system(command.c_str());
	}

	/**
	 * @brief 在 Windows 环境下执行 PowerShell 命令。
	 * @details 非 Windows 平台将输出提示信息而不执行。
	 * @param cmd 要执行的 PowerShell 命令字符串。
	 * @code
	 * OtherHelper::RunPowershell("Write-Output 'Hello'");
	 * @endcode
	 */
	void OtherHelper::RunPowershell(const std::string &cmd)
	{
		std::string command = "powershell -Command \"" + cmd + "\"";

#ifdef _WIN32
		std::system(command.c_str());
#else
		std::cout << "PowerShell not available on this platform" << '\n';
#endif
	}

	/**
	 * @brief 获取当前项目名称。
	 * @return 固定返回 `\"Qahse\"`。
	 */
	std::string OtherHelper::GetCurrentProjectName()
	{

		return "Qahse";
	}

	/**
	 * @brief 获取当前可执行文件名称。
	 * @return 固定返回 `\"Qahse.exe\"`。
	 */
	std::string OtherHelper::GetCurrentExeName()
	{

		return "Qahse.exe";
	}

	/**
	 * @brief 获取当前软件版本号。
	 * @param path 保留参数，当前未使用。
	 * @return 固定返回 `\"3.0.0\"`。
	 */
	std::string OtherHelper::GetCurrentVersion(const std::string &path)
	{

		return "3.0.0";
	}

	/**
	 * @brief 获取当前程序集版本号（转回 GetCurrentVersion(\"\")）。
	 * @return 程序集版本字符串。
	 */
	std::string OtherHelper::GetCurrentAssemblyVersion()
	{
		return GetCurrentVersion("");
	}

	/**
	 * @brief 获取当前编译模式（Debug 或 Release）。
	 * @return 在 Debug 模式下返回 `\"Debug\"`，否则返回 `\"Release\"`。
	 */
	std::string OtherHelper::GetCurrentBuildMode()
	{
#ifdef _DEBUG
		return "Debug";
#else
		return "Release";
#endif
	}

	/**
	 * @brief 获取目标平台位数后缀字符串。
	 * @return 64 位平台返回 `\"_x64\"`，32 位平台返回 `\"_x32\"`。
	 */
	std::string OtherHelper::GetBuildMode()
	{
#ifdef _WIN64
		return "_x64";
#elif defined(_WIN32)
		return "_x32";
#else
		return sizeof(void *) == 8 ? "_x64" : "_x32";
#endif
	}

	/**
	 * @brief 在指定目录下递归查找所有具有特定扩展名的文件路径。
	 * @param directoryPath  要搜索的目录路径。
	 * @param fileExtension  要匹配的文件扩展名（例如 `".pcs"`，大小写不敏感）。
	 * @return 包含所有匹配文件绝对路径的字符串向量。
	 * @code
	 * auto files = OtherHelper::FindFilesWithExtension("C:\\demo", ".pcs");
	 * for(auto& f : files) std::cout << f << '\n';
	 * @endcode
	 */
	std::vector<std::string> OtherHelper::FindFilesWithExtension(const std::string &directoryPath, const std::string &fileExtension)
	{
		std::vector<std::string> result;

		try
		{
			if (DirectoryExists(directoryPath))
			{
				for (const auto &entry : std::filesystem::recursive_directory_iterator(directoryPath))
				{
					if (entry.is_regular_file())
					{
						std::string ext = entry.path().extension().string();
						std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

						std::string targetExt = fileExtension;
						std::transform(targetExt.begin(), targetExt.end(), targetExt.begin(), ::tolower);

						if (ext == targetExt)
						{
							result.push_back(Extensions::GetABSPath(entry.path().string()));
						}
					}
				}
			}
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error finding files: " << e.what() << '\n';
		}

		return result;
	}

	/**
	 * @brief 将当前工作目录切换到指定文件的所在目录。
	 * @param mainFilePath 参考文件路径，将工作目录切换到该文件所在的目录。
	 * @code
	 * OtherHelper::SetCurrentDirectoryW("C:\\project\\main.pcs");
	 * // 工作目录更改为 "C:\\project\\"
	 * @endcode
	 */
	void OtherHelper::SetCurrentDirectoryW(const std::string &mainFilePath)
	{
		try
		{
			std::filesystem::path filePath(mainFilePath);
			std::filesystem::path dirPath = filePath.parent_path();

			if (DirectoryExists(dirPath.string()))
			{
				std::filesystem::current_path(dirPath);
			}
			else
			{
				std::cerr << "Directory does not exist: " << dirPath << '\n';
			}
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error setting current directory: " << e.what() << '\n';
		}
	}

	/**
	 * @brief 从字符串数组中提取起始索引后的非空行（至 END/end 为止）。
	 * @param data       输入数据行数组。
	 * @param index      起始读取的行索引。
	 * @param deleteSame 是否对结果去重，true 表示去除重复行。
	 * @return 提取到的非空数据行字符串列表。
	 * @code
	 * auto lines = OtherHelper::ReadOutputWord(data, 5, false);
	 * @endcode
	 */
	std::vector<std::string> OtherHelper::ReadOutputWord(const std::vector<std::string> &data, int index, bool deleteSame)
	{
		std::vector<std::string> result;

		for (size_t i = static_cast<size_t>(index); i < data.size(); ++i)
		{
			std::string line = TrimString(data[i]);

			if (line == "END" || line == "end")
			{
				break;
			}

			if (!line.empty())
			{
				result.push_back(line);
			}
		}

		if (deleteSame)
		{
			std::sort(result.begin(), result.end());
			result.erase(std::unique(result.begin(), result.end()), result.end());
		}

		return result;
	}

	/**
	 * @brief 在字符串数组中查找包含搜索关键词的行索引列表。
	 * @param input      要搜索的字符串数组。
	 * @param searchTerm 要搜索的子字符串。
	 * @param path       参考文件路径，用于错误日志提示。
	 * @param error      未找到时是否输出错误信息。
	 * @param show       未找到时是否输出警告信息。
	 * @return 包含所有匹配行索引的向量；未找到时返回 `{-1}`。
	 * @code
	 * auto indices = OtherHelper::GetMatchingLineIndexes(lines, "WindSpeed", "file.yaml", false, true);
	 * @endcode
	 */
	std::vector<int> OtherHelper::GetMatchingLineIndexes(const std::vector<std::string> &input, const std::string &searchTerm,
														 const std::string &path, bool error, bool show)
	{
		std::vector<int> result;

		for (size_t i = 0; i < input.size(); ++i)
		{
			if (input[i].find(searchTerm) != std::string::npos)
			{
				result.push_back(static_cast<int>(i));
			}
		}

		if (result.empty())
		{
			if (error)
			{
				std::cerr << "Search term '" << searchTerm << "' not found in file: " << path << '\n';
			}
			else if (show)
			{
				std::cout << "Warning: Search term '" << searchTerm << "' not found in file: " << path << '\n';
			}
			result.push_back(-1);
		}

		return result;
	}

	/**
	 * @brief 检查单行字符串是否包含指定子字符串。
	 * @param input      要检查的字符串。
	 * @param searchTerm 要搜索的子字符串。
	 * @return 包含则返回 true，否则返回 false。
	 */
	bool OtherHelper::GetMatchingLineIndexes(const std::string &input, const std::string &searchTerm)
	{
		return input.find(searchTerm) != std::string::npos;
	}

	/**
	 * @brief 获取当前系统硬件支持的逻辑线程数。
	 * @return 逻辑线程数（CPU 核心数 x 超线程数）。
	 * @code
	 * int n = OtherHelper::GetThreadCount();
	 * // n == 16 (运行在 8 核 16 线 CPU 上）
	 * @endcode
	 */
	int OtherHelper::GetThreadCount()
	{
		return static_cast<int>(std::thread::hardware_concurrency());
	}

	/**
	 * @brief 获取文件路径的扩展名（小写，包含点号）。
	 * @param path 要解析的文件路径。
	 * @return 扩展名字符串，如 `".yaml"`，无扩展名时返回空字符串。
	 * @code
	 * std::string ext = OtherHelper::GetFileExtension("config.YAML");
	 * // ext == ".yaml"
	 * @endcode
	 */
	std::string OtherHelper::GetFileExtension(const std::string &path)
	{
		std::filesystem::path filePath(path);
		std::string extension = filePath.extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		return extension;
	}

	/**
	 * @brief 将文件的扩展名更改为新扩展名，并将原文件重命名。
	 * @param path         原文件路径，成功时将被更新为新路径。
	 * @param newExtension 新的扩展名（如 `".txt"`）。
	 */
	void OtherHelper::SetFileExtension(std::string &path, const std::string &newExtension)
	{
		try
		{
			std::filesystem::path oldPath(path);
			std::filesystem::path newPath = oldPath;
			newPath.replace_extension(newExtension);

			if (FileExists(oldPath.string()))
			{
				std::filesystem::rename(oldPath, newPath);
				path = newPath.string();
			}
			else
			{
				std::cerr << "File does not exist: " << path << '\n';
			}
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error changing file extension: " << e.what() << '\n';
		}
	}
	/**
	 * @brief 将源字符串的每个字符之间插入指定数量的分隔字符串。
	 * @param source 源字符串。
	 * @param spilt  要插入的分隔字符串。
	 * @param num    插入分隔符的次数。
	 * @return 插入和后的新字符串。
	 * @code
	 * std::string r = OtherHelper::FillString("abc", ".", 2);
	 * // r == "a..b..c"
	 * @endcode
	 */
	std::string OtherHelper::FillString(const std::string &source, const std::string &spilt, int num)
	{
		if (source.empty() || spilt.empty() || num <= 0)
		{
			return source;
		}
		std::string result = "";
		for (size_t i = 0; i < source.length(); ++i)
		{
			result += source[i];
			if (i < source.length() - 1)
			{
				for (int j = 0; j < num; ++j)
				{
					result += spilt;
				}
			}
		}
		return result;
	}

	/**
	 * @brief 生成指定长度的随机大小写字母字符串。
	 * @param len 要生成的字符串长度。
	 * @return 由随机大小写字母构成的字符串。
	 * @code
	 * std::string s = OtherHelper::RandomString(8);
	 * // s 可能为 "aBcDeFgH"
	 * @endcode
	 */
	std::string OtherHelper::RandomString(int len)
	{
		const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		std::string result;
		result.reserve(len);

		// 使用C标准库的rand函数，避免C++11 random引起的编译问题
		static bool seeded = false;
		if (!seeded)
		{
			std::srand(static_cast<unsigned int>(std::time(nullptr)));
			seeded = true;
		}

		for (int i = 0; i < len; ++i)
		{
			result += chars[std::rand() % chars.size()];
		}

		return result;
	}

	/**
	 * @brief 将字符串居中显示，用指定符号填充两侧。
	 * @param input  要居中的字符串。
	 * @param width  居中后的总宽度。
	 * @param symbol 用于填充的字符，默认为空格。
	 * @return 居中后的字符串；若字符串长度 >= width，则直接返回原字符串。
	 * @code
	 * std::string s = OtherHelper::CenterText("hello", 11, '-');
	 * // s == "---hello---"
	 * @endcode
	 */
	std::string OtherHelper::CenterText(const std::string &input, int width, char symbol)
	{
		if (static_cast<int>(input.length()) >= width)
		{
			return input;
		}

		int totalPadding = width - static_cast<int>(input.length());
		int leftPadding = totalPadding / 2;
		int rightPadding = totalPadding - leftPadding;

		return std::string(leftPadding, symbol) + input + std::string(rightPadding, symbol);
	}

	/**
	 * @brief 将源目录中的文件复制到目标目录。
	 * @param sourceDirectory 源目录路径。
	 * @param targetDirectory 目标目录路径（不存在时自动创建）。
	 * @param fileType        要复制的文件类型，`\"*\"` 表示复制全部文件，否则指定扩展名（如 `\"*.txt\"`）。
	 * @param overwrite       是否覆盖已存在的目标文件。
	 * @code
	 * OtherHelper::CopyFileW("C:\\src\\", "C:\\dst\\", "*.yaml", true);
	 * @endcode
	 */
	void OtherHelper::CopyFileW(const std::string &sourceDirectory, const std::string &targetDirectory,
								const std::string &fileType, bool overwrite)
	{
		try
		{
			if (!DirectoryExists(targetDirectory))
			{
				CreateDirectories(targetDirectory);
			}

			std::filesystem::copy_options options = std::filesystem::copy_options::recursive;
			if (overwrite)
			{
				options |= std::filesystem::copy_options::overwrite_existing;
			}

			if (fileType == "*")
			{
				std::filesystem::copy(sourceDirectory, targetDirectory, options);
			}
			else
			{
				// 复制特定类型的文件
				for (const auto &entry : std::filesystem::recursive_directory_iterator(sourceDirectory))
				{
					if (entry.is_regular_file())
					{
						std::string ext = entry.path().extension().string();
						if (ext == fileType || ("*" + ext) == fileType)
						{
							std::filesystem::path relativePath = std::filesystem::relative(entry.path(), sourceDirectory);
							std::filesystem::path targetPath = std::filesystem::path(targetDirectory) / relativePath;

							CreateDirectories(targetPath.parent_path().string());
							std::filesystem::copy_file(entry.path(), targetPath,
													   overwrite ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::none);
						}
					}
				}
			}
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error copying files: " << e.what() << '\n';
		}
	}

	/**
	 * @brief 使用 ANSI 转义序列移动控制台光标位置。
	 * @param left 水平移动量（正向右，负向左）；为 0 时不移动。
	 * @param top  垂直移动量（正向下，负向上）；为 0 时不移动。
	 */
	void OtherHelper::SetCursorPosition(int left, int top)
	{

		// ANSI escape sequences for UNIX-like systems
		if (left != 0)
		{
			if (left > 0)
			{
				std::cout << "\033[" << left << "C"; // Move right
			}
			else
			{
				std::cout << "\033[" << (-left) << "D"; // Move left
			}
		}
		if (top != 0)
		{
			if (top > 0)
			{
				std::cout << "\033[" << top << "B"; // Move down
			}
			else
			{
				std::cout << "\033[" << (-top) << "A"; // Move up
			}
		}
	}
	/**
	 * @brief 安全获取本地时间并以 `std::tm` 结构体返回（辅助函数）。
	 * @details 在 Windows 上使用 `localtime_s`，在 POSIX 上使用 `localtime_r`，保证线程安全。
	 * @param time 要转换的 `std::time_t` 时间戳。
	 * @return 本地时间的 `std::tm` 结构体副本。
	 */
	// 辅助函数：安全获取本地时间
	std::tm OtherHelper::GetSafeLocalTime(const std::time_t &time)
	{
		std::tm tm_result = {};
#ifdef _WIN32
		localtime_s(&tm_result, &time);
#else
		localtime_r(&time, &tm_result);
#endif
		return tm_result;
	}

	/**
	 * @brief 安全获取本地时间并以指针形式返回（重载版本，指针参数区分）。
	 * @param time  要转换的 `std::time_t` 时间戳。
	 * @param ptr   区分安全重载的占位参数，传入任意 bool 即可。
	 * @return 指向 `std::tm` 结构体的指针。
	 * @warning 返回的指针指向局部变量，调用后应立即使用，不得长期保存。
	 */
	// 辅助函数：安全获取本地时间
	std::tm *OtherHelper::GetSafeLocalTime(const std::time_t &time, const bool ptr)
	{
		std::tm tm_result = {};
#ifdef _WIN32
		localtime_s(&tm_result, &time);
#else
		localtime_r(&time, &tm_result);
#endif
		return &tm_result;
	}

	// 时间相关函数实现
	/**
	 * @brief 获取当前年份字符串（本地时区）。
	 * @return 四位年份字符串，例如 \"2025\"。
	 */
	std::string OtherHelper::GetCurrentYear()
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_year + 1900);
	}

	/**
	 * @brief 获取当前年份整数形式（本地时区）。
	 * @param temp111 占位参数，用于重载区分。
	 * @return 当前年份的整数值，例如 2025。
	 */
	int OtherHelper::GetCurrentYear(bool temp111)
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_year + 1900;
	}

	/**
	 * @brief 获取当前月份字符串（本地时区）。
	 * @return 月份字符串，例如 \"6\"。
	 */
	std::string OtherHelper::GetCurrentMonth()
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_mon + 1);
	}

	/**
	 * @brief 获取当前月份整数形式（本地时区）。
	 * @param temp111 占位参数。
	 * @return 当前月份整数，范围 [1, 12]。
	 */
	int OtherHelper::GetCurrentMonth(bool temp111)
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_mon + 1;
	}

	/**
	 * @brief 获取当前日期字符串（本地时区）。
	 * @return 日期字符串，例如 \"15\"。
	 */
	std::string OtherHelper::GetCurrentDay()
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_mday);
	}

	/**
	 * @brief 获取当前日期整数形式（本地时区）。
	 * @param temp111 占位参数。
	 * @return 当前日期整数，范围 [1, 31]。
	 */
	int OtherHelper::GetCurrentDay(bool temp111)
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_mday;
	}

	/**
	 * @brief 获取当前小时字符串（24 小时制，本地时区）。
	 * @return 小时字符串，例如 \"14\"。
	 */
	std::string OtherHelper::GetCurrentHour()
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_hour);
	}

	/**
	 * @brief 获取当前小时整数形式（24 小时制，本地时区）。
	 * @param temp111 占位参数。
	 * @return 当前小时整数，范围 [0, 23]。
	 */
	int OtherHelper::GetCurrentHour(bool temp111)
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_hour;
	}

	/**
	 * @brief 获取当前分钟字符串（本地时区）。
	 * @return 分钟字符串，例如 \"30\"。
	 */
	std::string OtherHelper::GetCurrentMinute()
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_min);
	}

	/**
	 * @brief 获取当前分钟整数形式（本地时区）。
	 * @param temp111 占位参数。
	 * @return 当前分钟整数，范围 [0, 59]。
	 */
	int OtherHelper::GetCurrentMinute(bool temp111)
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_min;
	}

	/**
	 * @brief 获取当前秒数字符串（本地时区）。
	 * @return 秒数字符串，范围 \"0\"~\"59\"。
	 */
	std::string OtherHelper::GetCurrentSecond()
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_sec);
	}

	/**
	 * @brief 获取当前秒数整数形式（本地时区）。
	 * @param temp111 占位参数。
	 * @return 当前秒数整数，范围 [0, 59]。
	 */
	int OtherHelper::GetCurrentSecond(bool temp111)
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_sec;
	}

	/**
	 * @brief 获取当前时间格式化字符串，格式为 \"yyyy-MM-dd HH:MM:SS\"。
	 * @return 格式化时间字符串，例如 \"2025-06-01 14:30:00\"。
	 * @code
	 * std::string t = OtherHelper::GetCurrentTimeW();
	 * // t == "2025-06-01 14:30:00"
	 * @endcode
	 */
	std::string OtherHelper::GetCurrentTimeW()
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);

		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
		return oss.str();
	}

	/**
	 * @brief 获取编译时间戳字符串（由编译器预定义宏 `__DATE__` `__TIME__` 提供）。
	 * @return 编译时日期和时间字符串，例如 \"Jun  1 2025 14:30:00\"。
	 */
	std::string OtherHelper::GetBuildTime()
	{
		return __DATE__ " " __TIME__;
	}

	/**
	 * @brief 计算两个字符串之间的 Levenshtein 编辑距离（私有辅助函数）。
	 * @details 编辑距离是将字符串 `a` 转化为 `b` 所需的最少单字符操作次数
	 *          （插入、删除、替换）。
	 * @param a 第一个字符串。
	 * @param b 第二个字符串。
	 * @return 两字符串间的编辑距离整数。
	 * @code
	 * int d = OtherHelper::LevenshteinDistance("kitten", "sitting");
	 * // d == 3
	 * @endcode
	 */
	// 私有辅助函数实现
	int OtherHelper::LevenshteinDistance(const std::string &a, const std::string &b)
	{
		const size_t m = a.length();
		const size_t n = b.length();

		if (m == 0)
			return static_cast<int>(n);
		if (n == 0)
			return static_cast<int>(m);

		std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

		for (size_t i = 0; i <= m; ++i)
		{
			dp[i][0] = static_cast<int>(i);
		}
		for (size_t j = 0; j <= n; ++j)
		{
			dp[0][j] = static_cast<int>(j);
		}

		for (size_t i = 1; i <= m; ++i)
		{
			for (size_t j = 1; j <= n; ++j)
			{
				int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
				// dp[i][j] = std::min
				//(
				//	{
				//	dp[i - 1][j] + 1,      // deletion
				//	dp[i][j - 1] + 1,      // insertion
				//	dp[i - 1][j - 1] + cost // substitution
				//	}
				//);
				/*dp[i][j] = std::min(std::min(dp[i - 1][j] + 1, dp[i][j - 1] + 1), dp[i - 1][j - 1] + cost);*/
				auto min = {
					dp[i - 1][j] + 1,		// deletion
					dp[i][j - 1] + 1,		// insertion
					dp[i - 1][j - 1] + cost // substitution
				};
				dp[i][j] = std::min_element(min.begin(), min.end(), [](int a, int b)
											{ return a < b; })[0];
				// dp[i][j] = std::min(dp[i - 1][j] + 1,
				//	std::min(dp[i][j - 1] + 1,
				//		dp[i - 1][j - 1] + cost));
			}
		}

		return dp[m][n];
	}

	/**
	 * @brief 按指定分隔符将字符串分割为子字符串向量。
	 * @param str       要分割的字符串。
	 * @param delimiter 分隔符字符串。
	 * @return 分割后的子字符串向量。
	 * @code
	 * auto v = OtherHelper::SplitString("a,b,c", ",");
	 * // v == {"a", "b", "c"}
	 * @endcode
	 */
	std::vector<std::string> OtherHelper::SplitString(const std::string &str, const std::string &delimiter)
	{
		std::vector<std::string> tokens;
		size_t start = 0;
		size_t end = str.find(delimiter);

		while (end != std::string::npos)
		{
			tokens.push_back(str.substr(start, end - start));
			start = end + delimiter.length();
			end = str.find(delimiter, start);
		}

		tokens.push_back(str.substr(start));
		return tokens;
	}

	/**
	 * @brief 去除字符串两端的空白字符（空格、制表符、换行符等）。
	 * @param str 要处理的字符串。
	 * @return 去除两端空白后的字符串副本。
	 * @code
	 * std::string s = OtherHelper::TrimString("  hello  ");
	 * // s == "hello"
	 * @endcode
	 */
	std::string OtherHelper::TrimString(const std::string &str)
	{
		const std::string whitespace = " \t\n\r\f\v";

		size_t start = str.find_first_not_of(whitespace);
		if (start == std::string::npos)
		{
			return "";
		}

		size_t end = str.find_last_not_of(whitespace);
		return str.substr(start, end - start + 1);
	}

	/**
	 * @brief 将字符串转换为全小写。
	 * @param str 要转换的字符串。
	 * @return 全小写版本的字符串副本。
	 * @code
	 * std::string s = OtherHelper::ToLowerString("Hello World");
	 * // s == "hello world"
	 * @endcode
	 */
	std::string OtherHelper::ToLowerString(const std::string &str)
	{
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		return result;
	}

	/**
	 * @brief 检尺指定路径是否对应一个存在的常规文件。
	 * @param path 要检查的文件路径。
	 * @return 文件存在且为常规文件时返回 true，否则返回 false。
	 */
	bool OtherHelper::FileExists(const std::string &path)
	{
		return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
	}

	/**
	 * @brief 检尺指定路径是否对应一个存在的目录。
	 * @param path 要检查的目录路径。
	 * @return 目录存在时返回 true，否则返回 false。
	 */
	bool OtherHelper::DirectoryExists(const std::string &path)
	{
		return std::filesystem::exists(path) && std::filesystem::is_directory(path);
	}

	/**
	 * @brief 递归创建指定路径下的所有目录（如果不存在）。
	 * @param path 要创建的目录路径。
	 */
	void OtherHelper::CreateDirectories(const std::string &path)
	{
		try
		{
			std::filesystem::create_directories(path);
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error creating directories: " << e.what() << '\n';
		}
	}

	/**
	 * @brief 比较两个对象是否相等，可选择忽略指定字段名称的字段。
	 * @tparam T 要比较的类型，操算符 `==` 可用的类型均支持。
	 * @param left             第一个对象。
	 * @param right            第二个对象。
	 * @param ignoreFieldsName 忽略的字段名称列表（复杂类型保留实现）。
	 * @return 两对象相等返回 true。
	 */
	// 模板方法实现
	template <typename T>
	bool OtherHelper::AreEqual(const T &left, const T &right, const std::vector<std::string> &ignoreFieldsName)
	{
		// 简化实现，对于基本类型直接比较
		if constexpr (std::is_arithmetic_v<T>)
		{
			return left == right;
		}
		else
		{
			// 对于复杂类型，需要自定义比较逻辑
			return left == right;
		}
	}

	/**
	 * @brief 将指定类型的对象转换为字符串表示（int 特化）。
	 * @param message 要转换的 int 整数。
	 * @param fg      分隔符字符（对标量无意义）。
	 * @param coloum  输出方向（对标量无意义）。
	 * @return 转换后的字符串。
	 */
	template <>
	std::string OtherHelper::Tostring<int>(const int &message, char fg, bool coloum)
	{
		return std::to_string(message);
	}

	/**
	 * @brief 将 bool 尺转换为字符串（\"0\" 或 \"1\"）。
	 */
	template <>
	std::string OtherHelper::Tostring<bool>(const bool &message, char fg, bool coloum)
	{
		return std::to_string(message);
	}

	/**
	 * @brief 将 double 数字转换为字符串。
	 */
	template <>
	std::string OtherHelper::Tostring<double>(const double &message, char fg, bool coloum)
	{

		return std::to_string(message);
	}
	/**
	 * @brief 将 float 数字转换为字符串。
	 */
	template <>
	std::string OtherHelper::Tostring<float>(const float &message, char fg, bool coloum)
	{

		return std::to_string(message);
	}
	/**
	 * @brief 将 std::string 转换为字符串（直接返回原字符串）。
	 */
	template <>
	std::string OtherHelper::Tostring<std::string>(const std::string &message, char fg, bool coloum)
	{

		return message;
	}
	/**
	 * @brief 将 Eigen::VectorXd 向量转换为字符串，元素间用 `fg` 分隔。
	 * @param message 要转换的 Eigen::VectorXd 对象。
	 * @param fg      元素间的分隔符字符。
	 * @param coloum  true 表示按列输出ff08保留参数），false 按行输出。
	 * @return 转换后的字符串。
	 */
	template <>
	std::string OtherHelper::Tostring<Eigen::VectorXd>(const Eigen::VectorXd &message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum)
		{
			for (int i = 0; i < message.size(); ++i)
			{
				oss << message(i);
				if (i < message.size() - 1)
				{
					oss << fg;
				}
			}
		}
		else
		{
			for (int i = 0; i < message.size(); ++i)
			{
				oss << message(i);
				if (i < message.size() - 1)
				{
					oss << fg;
				}
			}
		}
		return oss.str();
	}
	/**
	 * @brief 将 Eigen::VectorXf 向量转换为字符串，元素间用 `fg` 分隔。
	 * @param message 要转换的 Eigen::VectorXf 对象。
	 * @param fg      元素间的分隔符字符。
	 * @param coloum  true 表示按列输出，false 按行输出。
	 * @return 转换后的字符串。
	 */
	template <>
	std::string OtherHelper::Tostring<Eigen::VectorXf>(const Eigen::VectorXf &message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum)
		{
			for (int i = 0; i < message.size(); ++i)
			{
				oss << message(i);
				if (i < message.size() - 1)
				{
					oss << fg;
				}
			}
		}
		else
		{
			for (int i = 0; i < message.size(); ++i)
			{
				oss << message(i);
			}
		}
		return oss.str();
	}

	/**
	 * @brief 将 Eigen::MatrixXd 矩阵转换为字符串，行内元素用 `fg` 分隔，行间用换行符分隔。
	 */
	template <>
	std::string OtherHelper::Tostring<Eigen::MatrixXd>(const Eigen::MatrixXd &message, char fg, bool coloum)
	{
		std::ostringstream oss;

		// Output matrix in column-major format
		for (int i = 0; i < message.rows(); ++i)
		{
			for (int j = 0; j < message.cols(); ++j)
			{
				oss << message(j, j);
				if (j < message.rows() - 1)
				{
					oss << fg;
				}
			}
			if (i < message.cols() - 1)
			{
				oss << '\n';
			}
		}

		return oss.str();
	}

	/**
	 * @brief 将 Eigen::MatrixXf 矩阵转换为字符串（行优先布局），行内元素用 `fg` 分隔。
	 */
	template <>
	std::string OtherHelper::Tostring<Eigen::MatrixXf>(const Eigen::MatrixXf &message, char fg, bool coloum)
	{
		std::ostringstream oss;

		// Output matrix in row-major format
		for (int i = 0; i < message.rows(); ++i)
		{
			for (int j = 0; j < message.cols(); ++j)
			{
				oss << message(i, j);
				if (j < message.cols() - 1)
				{
					oss << fg;
				}
			}
			if (i < message.rows() - 1)
			{
				oss << '\n';
			}
		}

		return oss.str();
	}

	/**
	 * @brief 将对象转换为字符串表示
	 */
	template <>
	static std::string OtherHelper::Tostring<std::vector<double>>(const std::vector<double> &message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum)
		{
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1)
				{
					oss << '\n';
				}
			}
		}
		else
		{
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1)
				{
					oss << fg;
				}
			}
		}
		return oss.str();
	}
	/**
	 * @brief 将 `std::vector<float>` 转换为字符串。
	 */
	template <>
	static std::string OtherHelper::Tostring<std::vector<float>>(const std::vector<float> &message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum)
		{
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1)
				{
					oss << '\n';
				}
			}
		}
		else
		{
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1)
				{
					oss << fg;
				}
			}
		}
		return oss.str();
	}

	/**
	 * @brief 将 `std::vector<int>` 转换为字符串。
	 */
	template <>
	static std::string OtherHelper::Tostring<std::vector<int>>(const std::vector<int> &message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum)
		{
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1)
				{
					oss << '\n';
				}
			}
		}
		else
		{
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1)
				{
					oss << fg;
				}
			}
		}
		return oss.str();
	}

	/**
	 * @brief 将 `std::vector<bool>` 转换为字符串。
	 */
	template <>
	static std::string OtherHelper::Tostring<std::vector<bool>>(const std::vector<bool> &message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum)
		{
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1)
				{
					oss << '\n';
				}
			}
		}
		else
		{
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1)
				{
					oss << fg;
				}
			}
		}
		return oss.str();
	}

	// 显式模板实例化
	template bool OtherHelper::AreEqual<int>(const int &, const int &, const std::vector<std::string> &);
	template bool OtherHelper::AreEqual<double>(const double &, const double &, const std::vector<std::string> &);
	template bool OtherHelper::AreEqual<float>(const float &, const float &, const std::vector<std::string> &);
	template bool OtherHelper::AreEqual<std::string>(const std::string &, const std::string &, const std::vector<std::string> &);



	/**
	 * @brief 从文件行数组中解析指定行的关键字后的数据，并返回指定类型。
	 * @tparam T             感兴数据的目标类型（int/double/float/string 等）。
	 * @param lines          文件全部行的字符串数组。
	 * @param filename       文件名，用于错误日志。
	 * @param pp             `{index, keyword}` 元组，指定要搜索的行号和关键字。
	 * @param moren          可选默认値；解析失败时返回。
	 * @param num            尺数（保留参数）。
	 * @param fg             行内分隔符字符，默认为空格。
	 * @param fg1            化小分隔符字符（保留参数）。
	 * @param station        要提取分割后的第几个元素（0 基）。
	 * @param namelist       头标列表指针（保留参数）。
	 * @param row            读取方向（保留参数）。
	 * @param titleLine      标题行指针（保留参数）。
	 * @param warning        是否在解析失败时输出警告。
	 * @param errorInf       自定义错误信息（保留参数）。
	 * @return 解析得到的目标类型对象；失败时返回默认値或 `T{}`。
	 */
	template <typename T>
	T OtherHelper::ParseLine(ParseLineArgs<T> args)
	{
		try
		{
			int index = std::get<0>(args.pp);
			std::string key = std::get<1>(args.pp);

			if (index >= 0 && index < static_cast<int>(args.lines.size()))
			{
				std::string line = args.lines[index];

				// 查找关键字
				size_t keyPos = line.find(key);
				if (keyPos != std::string::npos)
				{
					// 提取值部分
					std::string valuePart = line.substr(keyPos + key.length());

					// 去除前导空白
					valuePart = TrimString(valuePart);

					// 按分隔符分割
					std::vector<std::string> tokens = SplitString(valuePart, std::string(1, args.fg));

					if (args.station < static_cast<int>(tokens.size()))
					{
						std::string valueStr = TrimString(tokens[args.station]);

						// 类型转换
						if constexpr (std::is_same_v<T, int>)
						{
							return std::stoi(valueStr);
						}
						else if constexpr (std::is_same_v<T, double>)
						{
							return std::stod(valueStr);
						}
						else if constexpr (std::is_same_v<T, float>)
						{
							return std::stof(valueStr);
						}
						else if constexpr (std::is_same_v<T, std::string>)
						{
							return valueStr;
						}
						
						else
						{
							// 对于其他类型，尝试从字符串构造
							std::istringstream iss(valueStr);
							T result;
							iss >> result;
							return result;
						}
					}
				}
			}

			// 解析失败，返回默认值或抛出异常
			if (args.moren.has_value())
			{
				return args.moren.value();
			}
			else
			{
				if (args.warning)
				{
					std::cerr << "Warning: Failed to parse line in " << args.filename
							  << " for key: " << std::get<1>(args.pp) << '\n';
				}
				return T{};
			}
		}
		catch (const std::exception &e)
		{
			if (args.warning)
			{
				std::cerr << "Error parsing line in " << args.filename << ": " << e.what() << '\n';
			}

			if (args.moren.has_value())
			{
				return args.moren.value();
			}
			else
			{
				return T{};
			}
		}
	}

	/**
	 * @brief 获取结构体的字段名列表和类型信息（简化实现，返回空向量）。
	 * @tparam T       结构体类型。
	 * @param structure 结构体对象（未使用）。
	 * @return 包含 `{name, typeinfo, value}` 元组的向量（当前返回空）。
	 */
	template <typename T>
	std::vector<std::tuple<std::string, const std::type_info *, T>> OtherHelper::GetStructFields(const T &structure)
	{
		// 简化实现
		std::vector<std::tuple<std::string, const std::type_info *, T>> result;
		return result;
	}

	/**
	 * @brief 获取结构体字段的名称和类型信息（简化实现，返回空向量）。
	 * @tparam T       结构体类型。
	 * @param structure 结构体对象。
	 * @return 包含 `{name, typeinfo}` 元组的向量（当前返回空）。
	 */
	template <typename T>
	std::vector<std::tuple<std::string, const std::type_info *>> OtherHelper::GetStructNameAndType(const T &structure)
	{
		// 简化实现
		std::vector<std::tuple<std::string, const std::type_info *>> result;
		return result;
	}

	/**
	 * @brief 判断类型 T 是否为结构体（即非指针、非引用的类类型）。
	 * @tparam T 要检查的类型。
	 * @return 是结构体类型返回 true，否则返回 false。
	 */
	template <typename T>
	bool OtherHelper::IsStruct()
	{
		return std::is_class_v<T> && !std::is_pointer_v<T> && !std::is_reference_v<T>;
	}

	/**
	 * @brief 判断类型 T 是否为结构体或结构体数组。
	 * @tparam T 要检查的类型。
	 * @return T 为结构体或结构体数组时返回 true。
	 */
	template <typename T>
	bool OtherHelper::IsStructOrStructArray()
	{
		if constexpr (std::is_array_v<T>)
		{
			using ElementType = std::remove_extent_t<T>;
			return IsStruct<ElementType>();
		}
		else
		{
			return IsStruct<T>();
		}
	}

	/**
	 * @brief 判断对象 obj 是否为列表类型（简化实现，始终返回 false）。
	 * @tparam T 要检查的类型。
	 * @param obj 要判断的对象。
	 * @return 当前简化实现始终返回 false。
	 */
	template <typename T>
	bool OtherHelper::IsList(const T &obj)
	{
		// 检查是否为vector、list或array类型
		return false; // 简化实现
	}

	/**
	 * @brief 获取结构体类型的 RTTI 类型名。
	 * @tparam T        结构体类型。
	 * @param structure 结构体对象（仅用于类型推导）。
	 * @return 类型名字符串（由 `typeid(T).name()` 返回，不同编译器格式不同）。
	 */
	template <typename T>
	std::string OtherHelper::GetStructName(const T &structure)
	{
		return typeid(T).name();
	}

	/**
	 * @brief 尝试将字符串转换为枚举尺，失败时返回 false（简化实现）。
	 * @tparam T         目标枚举类型。
	 * @param value      待转换的字符串。
	 * @param enumValue  输出：成功时写入转换结果。
	 * @return 转换成功返回 true，失败返回 false。
	 */
	template <typename T>
	bool OtherHelper::TryConvertToEnum(const std::string &value, T &enumValue)
	{
		// 简化实现，C++枚举转换需要手动映射
		try
		{
			enumValue = ConvertToEnum<T>(value);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	/**
	 * @brief 将字符串转换为枚举尺（简化实现，需手动映射）。
	 * @tparam T    目标枚举类型。
	 * @param value 待转换的字符串。
	 * @return 转换后的枚举尺。
	 * @throws std::invalid_argument 未实现具体类型的转换时抛出。
	 */
	template <typename T>
	T OtherHelper::ConvertToEnum(const std::string &value)
	{
		// 简化实现，实际需要手动映射枚举值
		throw std::invalid_argument("Enum conversion not implemented for this type");
	}

	/**
	 * @brief 将 Eigen 矩阵的行列标题转换为输出文件行格式。
	 * @tparam MatrixType    矩阵类型（Eigen::MatrixXd 等）。
	 * @param title        矩阵戴转。
	 * @param rowtitle     行标题（列名）列表。
	 * @param columtitle   列标题（行名）列表。
	 * @param matrix       矩阵数据（当前占位副，输出内容为 0）。
	 * @return 格式化后多行字符串向量。
	 */
	template <typename MatrixType>
	std::vector<std::string> OtherHelper::ConvertMatrixTitleToOutfile(const std::string &title,
																	  const std::vector<std::string> &rowtitle,
																	  const std::vector<std::string> &columtitle,
																	  const MatrixType &matrix)
	{
		std::vector<std::string> output;

		// 构建标题行
		std::string titleLine = title;
		for (const auto &rowTitle : rowtitle)
		{
			titleLine += "\t" + rowTitle;
		}
		output.push_back(titleLine);

		// 构建数据行
		for (size_t i = 0; i < columtitle.size(); ++i)
		{
			std::string dataLine = columtitle[i];
			for (size_t j = 0; j < rowtitle.size(); ++j)
			{
				// 这里需要根据实际的矩阵类型来访问元素
				dataLine += "\t0"; // 占位符，实际实现需要访问matrix(i,j)
			}
			output.push_back(dataLine);
		}

		return output;
	}

	/**
	 * @brief 对 `std::vector<T>` 进行去重，返回排序后的唯一元素向量。
	 * @tparam T     元素类型，需支持排序操作符 `<`。
	 * @param values 原始向量。
	 * @return 去重并排序后的元素向量。
	 * @code
	 * auto v = OtherHelper::Distinct(std::vector<int>{3,1,2,1,3});
	 * // v == {1, 2, 3}
	 * @endcode
	 */
	template <typename T>
	std::vector<T> OtherHelper::Distinct(const std::vector<T> &values)
	{
		std::vector<T> result = values;
		std::sort(result.begin(), result.end());
		result.erase(std::unique(result.begin(), result.end()), result.end());
		return result;
	}

	/**
	 * @brief 找出 `std::vector<T>` 中出现两次及以上的重复元素。
	 * @tparam T     元素类型，需支持 `unordered_map`。
	 * @param values 原始向量。
	 * @return 包含所有重复元素（每个只出现一次）的向量。
	 * @code
	 * auto v = OtherHelper::Duplicates(std::vector<int>{1,2,2,3,3,3});
	 * // v 包含 2 和 3
	 * @endcode
	 */
	template <typename T>
	std::vector<T> OtherHelper::Duplicates(const std::vector<T> &values)
	{
		std::unordered_map<T, int> counts;
		std::vector<T> duplicates;

		for (const auto &value : values)
		{
			counts[value]++;
		}

		for (const auto &pair : counts)
		{
			if (pair.second > 1)
			{
				duplicates.push_back(pair.first);
			}
		}

		return duplicates;
	}

	/**
	 * @brief 对 `std::array<T, N>` 进行去重，返回固定大小的数组（多余位置用默认尺填充）。
	 * @tparam T 元素类型。
	 * @tparam N 数组大小。
	 * @param values 原始数组。
	 * @return 去重后的固定大小数组。
	 */
	template <typename T, size_t N>
	std::array<T, N> OtherHelper::Distinct(const std::array<T, N> &values)
	{
		std::vector<T> temp(values.begin(), values.end());
		auto distinctVec = Distinct(temp);

		std::array<T, N> result{};
		size_t copySize = std::min<size_t>(distinctVec.size(), N);
		std::copy(distinctVec.begin(), distinctVec.begin() + copySize, result.begin());

		return result;
	}

	/**
	 * @brief 找出 `std::array<T, N>` 中出现两次及以上的重复元素。
	 * @tparam T 元素类型。
	 * @tparam N 数组大小。
	 * @param values 原始数组。
	 * @return 包含重复元素的向量。
	 */
	template <typename T, size_t N>
	std::vector<T> OtherHelper::Duplicates(const std::array<T, N> &values)
	{
		std::vector<T> temp(values.begin(), values.end());
		return Duplicates(temp);
	}

	//// 在头文件中提供模板实现
	// template<typename T>
	// OtherHelper::FixedQueue<T>::FixedQueue(int capacity, const T& value) : num(capacity) {
	//	for (int i = 0; i < capacity; ++i) {
	//		stack.push(value);
	//	}
	// }

	// template<typename T>
	// void OtherHelper::FixedQueue<T>::Push(const T& element) {
	//	if (static_cast<int>(stack.size()) >= num) {
	//		stack.pop(); // 移除最早的元素
	//	}
	//	stack.push(element);
	// }

	// template<typename T>
	// int OtherHelper::FixedQueue<T>::Count() const {
	//	return num;
	// }

	// template<typename T>
	// int OtherHelper::FixedQueue<T>::PushCount() const {
	//	return static_cast<int>(stack.size());
	// }

	// template<typename T>
	// T OtherHelper::FixedQueue<T>::operator[](int index) const {
	//	std::queue<T> tempQueue = stack;
	//	std::vector<T> elements;

	//	while (!tempQueue.empty()) {
	//		elements.push_back(tempQueue.front());
	//		tempQueue.pop();
	//	}

	//	if (index >= 0 && index < static_cast<int>(elements.size())) {
	//		return elements[index];
	//	}

	//	throw std::out_of_range("Index out of range in FixedQueue");
	//}

	//// 显式模板实例化，用于常用类型
	// template class OtherHelper::FixedQueue<int>
	// template class OtherHelper::FixedQueue<double>
	// template class OtherHelper::FixedQueue<float>
	// template class OtherHelper::FixedQueue<std::string>
	//  显式模板实例化
	template std::vector<int> OtherHelper::Distinct(const std::vector<int> &);
	template std::vector<double> OtherHelper::Distinct(const std::vector<double> &);
	template std::vector<float> OtherHelper::Distinct(const std::vector<float> &);

	template std::vector<int> OtherHelper::Duplicates(const std::vector<int> &);
	template std::vector<double> OtherHelper::Duplicates(const std::vector<double> &);
	template std::vector<float> OtherHelper::Duplicates(const std::vector<float> &);

} // namespace Qahse::IO::IO