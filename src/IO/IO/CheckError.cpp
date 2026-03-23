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


#include "CheckError.h"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include "../Log/LogHelper.h"
#include "../IO/Extensions.h"

namespace fs = std::filesystem;
using namespace HawtC3::IO::Log;

namespace HawtC3::IO::IO {

	int CheckError::Checkwavetime_son = 1;
	bool CheckError::Checkwavetime_show = true;

	void CheckError::Checkwavetime(const std::vector<double>& timespan, double& t, bool ContinueCal) {
		if (timespan.empty()) return;

		t = t > 0 ? t - 0.0000001 : t;
		if (t < 0)
		{
			LogHelper::WarnLog("仿真时间小于0,请检查","",ConsoleColor::DarkYellow,0,"CheckError::Checkwavetime");
			return;
		}
		double ZHENSHU = std::floor(t / timespan.back());

		if (ZHENSHU != 0) {
			if (ContinueCal) {
				if (ZHENSHU == Checkwavetime_son) {
					if (Checkwavetime_show) {
						std::cout << '\n';
						Checkwavetime_son++;
						LogHelper::WarnLog("Current Time is the " + std::to_string(ZHENSHU) +
							" times exceed wave file, we will read file at start", "Checkwavetime");
					}
				}
				t = t - ZHENSHU * timespan.back();
			}
			else
			{
				LogHelper::ErrorLog("仿真时间超过波浪文件最大时间" +
					std::to_string(*std::max_element(timespan.begin(), timespan.end())) +
					"s 请将windL的CreadW设置为true再试", "", "", 20, "CheckError::Checkwavetime");
			}
		}
	}

	void CheckError::CheckPath(const std::string& name, bool createdir) {

		std::string name1 = Extensions::GetABSPath(name);
		fs::path path(name1);
		std::string temp = path.parent_path().string();

		if (temp.empty()) {
			LogHelper::ErrorLog("文件路径：" + name1 + "文件的父文件夹为空！或路径定义错误！", "", "", 20, "CheckError::CheckPath");
			return;
		}

		if (!fs::exists(temp)) {
			LogHelper::WriteLogO("文件路径：" + temp + "没有不存在！");
			if (createdir) {
				try {
					fs::create_directories(temp);
				}
				catch (const std::exception&) {
					LogHelper::WarnLog("您的权限不够！", "CheckPath");
					LogHelper::ErrorLog("文件路径：" + temp + "没有不存在,且系统权限不够，无法创建", "", "", 20, "CheckError::CheckPath");
				}
			}
		}
	}

	void CheckError::CheckDir(const std::string& name, bool createDir) {
		if (!fs::exists(name)) {
			LogHelper::WriteLogO("文件夹路径：" + name + "不存在！");
			if (createDir) {
				try {
					fs::create_directories(name);
				}
				catch (const std::exception&) {
					LogHelper::WarnLog("您的权限不够！", "CheckDir");
					LogHelper::ErrorLog("文件路径：" + name + "没有不存在,且系统权限不够，无法创建", "", "", 20, "CheckError::CheckDir");
				}
			}
		}
	}

	void CheckError::CheckInptr(void* ptr, const std::string& name, const std::string& dllpath) {
		if (ptr == nullptr) {
			// Note: IsDll64BitStr and GetBuildMode implementation needed
			// LogHelper::ErrorLog("The dll is " + IsDll64BitStr(dllpath) + 
			//     "bit ,but your build is " + Otherhelper::GetBuildMode() + 
			//     " bit,please check your dll path:\n  " + dllpath);
			LogHelper::ErrorLog("Cant find function name: " + name + " in dll path:\n  " + dllpath, "", "", 20, "CheckError::CheckInptr");
		}
	}

	void CheckError::CheckPath(std::string& path, const std::string& extension1,
		bool show, const std::string& information)
	{
		fs::path filePath(path);
		std::string extension = filePath.extension().string();

		if (extension.empty()) {
			if (show) {
				LogHelper::WarnLog(information + extension1, "CheckPath");
			}
			else {
				LogHelper::WriteLogO(information + extension1);
			}
			path += extension1;
		}
		else if (extension != extension1) {
			LogHelper::ErrorLog("当前文件的拓展名称" + extension + " 与指定名称" +
				extension1 + " 不同！请检查后再尝试", "", "", 20, "CheckError::CheckPath");
		}
	}

	bool CheckError::Filexists(const std::string& filePath, bool error, bool showwaring,
		const std::string& inf, const std::string& FunctionName)
	{
		if (fs::exists(filePath)) {
			return true;
		}

		std::string message = inf.empty() ?
			"当前文件：" + filePath + "不存在！请检查路径" : inf;

		if (error) {
			LogHelper::ErrorLog(FunctionName + ":" + message, "", "", 20, "CheckError::Filexists");
		}
		else if (showwaring) {
			LogHelper::WarnLog(FunctionName + ":" + message, "", ConsoleColor::DarkYellow, 0, "Filexists");
		}
		else {
			LogHelper::WriteLogO(FunctionName + ":" + message);
		}
		return false;
	}

	void CheckError::Filexists(const std::string& oripath, std::string& Path, bool createdir,
		const std::string& extren, bool outfile, bool error)
	{
		CheckPath(Path, extren, false);
		CheckPath(Path, createdir);

		fs::path originalPath(oripath);
		std::string oriDir = originalPath.parent_path().string();

		if (!fs::exists(Path)) {
			fs::path newPath = fs::path(oriDir) / Path;
			Path = fs::absolute(newPath).string();
			CheckPath(Path, createdir);

			if (!outfile && error && !fs::exists(Path)) {
				LogHelper::ErrorLog("当前文件： " + oripath +
					" \n   当中的路径参数： " + Path + " 无法读取", "", "", 20, "CheckError::Filexists");
			}
		}
		else {
			// Implement Otherhelper::FormortPath and GetABSPath equivalents if needed
			Path = fs::absolute(Path).string();
		}
	}
}