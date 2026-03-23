
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
#pragma once
#ifndef CHECKERROR_H
#define CHECKERROR_H
#include <string>
#include <vector>
#include <filesystem>
#include "../Log/LogHelper.h"
#include <Eigen/Dense>


namespace HawtC3::IO::IO {

	/// <summary>
	/// Provides utility methods for validating and handling file paths, directories, and simulation time checks.
	/// </summary>
	class CheckError {
	public:
		/// <summary>
		/// 这个函数的作用是检查当前的仿真时间是否是否包含在波浪文件当中
		/// </summary>
		static void Checkwavetime(const std::vector<double>& timespan, double& t, bool ContinueCal = true);

		/// <summary>
		/// Verifies the existence of the directory for the specified file path
		/// </summary>
		static void CheckPath(const std::string& name, bool createdir);
		/// <summary>
		/// Validates and adjusts the file path to ensure it has the specified file extension
		/// </summary>
		static void CheckPath(std::string& path, const std::string& extension1,
			bool show = true, const std::string& information = "文件拓展名称为空 ,我们将增加拓展名称后缀");

		/// <summary>
		/// Verifies the existence of a directory at the specified path
		/// </summary>
		static void CheckDir(const std::string& name, bool createDir);

		/// <summary>
		/// Validates that the specified function pointer is not null
		/// </summary>
		static void CheckInptr(void* ptr, const std::string& name, const std::string& dllpath);



		/// <summary>
		/// Determines whether the specified file exists at the given path
		/// </summary>
		static bool Filexists(const std::string& filePath, bool error = true, bool showwaring = true,
			const std::string& inf = "", const std::string& FunctionName = "");

		/// <summary>
		/// Validates and adjusts the specified file path based on the provided parameters
		/// </summary>
		static void Filexists(const std::string& oripath, std::string& Path, bool createdir = false,
			const std::string& extren = "", bool outfile = false, bool error = true);

		static int Checkwavetime_son;
		static bool Checkwavetime_show;
	};
}

#endif // !CHECKERROR_H