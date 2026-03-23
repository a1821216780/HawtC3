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



//#include <windows.h>



#include <process.h>
//#include <Psapi.h>
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



using namespace HawtC3::IO::Log;

namespace HawtC3::IO::IO
{

	// ProgressBar 类实现
	OtherHelper::ProgressBar::ProgressBar(int totalIterations, int level)
		: _totalIterations(totalIterations), _currentIteration(0) {
		levels = std::string(level * 2, ' '); // 每个级别缩进2个空格
	}

	void OtherHelper::ProgressBar::UpdateProgress() {
		_currentIteration++;

		float progress = static_cast<float>(_currentIteration) / _totalIterations;
		int barLength = static_cast<int>(progress * TotalBars);

		std::cout << "\r" << levels << "[";

		for (int i = 0; i < TotalBars; ++i) {
			if (i < barLength) {
				std::cout << ProgressBarChar;
			}
			else {
				std::cout << BackgroundChar;
			}
		}

		std::cout << "] " << static_cast<int>(progress * 100) << "% "
			<< "(" << _currentIteration << "/" << _totalIterations << ")";

		if (_currentIteration == _totalIterations) {
			std::cout << '\n';
		}

		std::cout.flush();
	}

	// 主要功能方法实现
	std::string OtherHelper::FormortPath(const std::string& path) {
		try {
			std::filesystem::path fsPath(path);
			return std::filesystem::absolute(fsPath).string();
		}
		catch (const std::exception& e) {
			std::cerr << "Error formatting path: " << e.what() << '\n';
			return path;
		}
	}

	std::tuple<int, std::string> OtherHelper::FindBestMatch(const std::vector<std::string>& strings, const std::string& target) {
		if (strings.empty()) {
			return std::make_tuple(-1, "");
		}

		int bestIndex = 0;
		int minDistance = LevenshteinDistance(strings[0], target);

		for (size_t i = 1; i < strings.size(); ++i) {
			int distance = LevenshteinDistance(strings[i], target);
			if (distance < minDistance) {
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
	//#define EIGEN_USE_MKL_ALL
	//		}
	//		else if (acc1 == "openblas"|| acc1 == "blas")
	//		{
	//#define EIGEN_USE_BLAS
	//		}
	//		else if (acc1 == "cuda")
	//		{
	//#define EIGEN_USE_CUDA
	//		}
	//		else if (acc1 == "none")
	//		{
	//#undef EIGEN_USE_MKL_ALL
	//#undef EIGEN_USE_BLAS
	//#undef EIGEN_USE_CUDA
	//		}
	//		else
	//		{
	//			LogHelper::ErrorLog("未知加速器！" + acc1, "", "", 20, "OtherHelper::Usemkl");
	//		}
	//	}

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

	void OtherHelper::SysRun(const std::string& exePath, const std::string& filePath) {
		std::string command = "\"" + exePath + "\" \"" + filePath + "\"";


		int result = std::system(command.c_str());
		if (result != 0) {
			std::cerr << "Failed to execute: " << command << '\n';
		}
	}

	void OtherHelper::RunCmd(const std::string& command) {
		std::string fullCommand = "cmd /c " + command;

		std::system(command.c_str());
	}

	void OtherHelper::RunPowershell(const std::string& cmd) {
		std::string command = "powershell -Command \"" + cmd + "\"";

#ifdef _WIN32
		std::system(command.c_str());
#else
		std::cout << "PowerShell not available on this platform" << '\n';
#endif
	}

	std::string OtherHelper::GetCurrentProjectName() {

		return "HawtC3";

	}

	std::string OtherHelper::GetCurrentExeName() {

		return "HawtC3.exe";
	}

	std::string OtherHelper::GetCurrentVersion(const std::string& path)
	{


		return "3.0.0";
	}

	std::string OtherHelper::GetCurrentAssemblyVersion()
	{
		return GetCurrentVersion("");
	}

	std::string OtherHelper::GetCurrentBuildMode() {
#ifdef _DEBUG
		return "Debug";
#else
		return "Release";
#endif
	}


	std::string OtherHelper::GetBuildMode() {
#ifdef _WIN64
		return "_x64";
#elif defined(_WIN32)
		return "_x32";
#else
		return sizeof(void*) == 8 ? "_x64" : "_x32";
#endif
	}

	std::vector<std::string> OtherHelper::FindFilesWithExtension(const std::string& directoryPath, const std::string& fileExtension) {
		std::vector<std::string> result;

		try {
			if (DirectoryExists(directoryPath)) {
				for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
					if (entry.is_regular_file()) {
						std::string ext = entry.path().extension().string();
						std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

						std::string targetExt = fileExtension;
						std::transform(targetExt.begin(), targetExt.end(), targetExt.begin(), ::tolower);

						if (ext == targetExt) {
							result.push_back(entry.path().string());
						}
					}
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error finding files: " << e.what() << '\n';
		}

		return result;
	}

	void OtherHelper::SetCurrentDirectoryW(const std::string& mainFilePath)
	{
		try {
			std::filesystem::path filePath(mainFilePath);
			std::filesystem::path dirPath = filePath.parent_path();

			if (DirectoryExists(dirPath.string())) {
				std::filesystem::current_path(dirPath);
			}
			else {
				std::cerr << "Directory does not exist: " << dirPath << '\n';
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error setting current directory: " << e.what() << '\n';
		}
	}

	std::vector<std::string> OtherHelper::ReadOutputWord(const std::vector<std::string>& data, int index, bool deleteSame) {
		std::vector<std::string> result;

		for (size_t i = static_cast<size_t>(index); i < data.size(); ++i) {
			std::string line = TrimString(data[i]);

			if (line == "END" || line == "end") {
				break;
			}

			if (!line.empty()) {
				result.push_back(line);
			}
		}

		if (deleteSame) {
			std::sort(result.begin(), result.end());
			result.erase(std::unique(result.begin(), result.end()), result.end());
		}

		return result;
	}

	std::vector<int> OtherHelper::GetMatchingLineIndexes(const std::vector<std::string>& input, const std::string& searchTerm,
		const std::string& path, bool error, bool show) {
		std::vector<int> result;

		for (size_t i = 0; i < input.size(); ++i) {
			if (input[i].find(searchTerm) != std::string::npos) {
				result.push_back(static_cast<int>(i));
			}
		}

		if (result.empty()) {
			if (error) {
				std::cerr << "Search term '" << searchTerm << "' not found in file: " << path << '\n';
			}
			else if (show) {
				std::cout << "Warning: Search term '" << searchTerm << "' not found in file: " << path << '\n';
			}
			result.push_back(-1);
		}

		return result;
	}

	bool OtherHelper::GetMatchingLineIndexes(const std::string& input, const std::string& searchTerm) {
		return input.find(searchTerm) != std::string::npos;
	}

	int OtherHelper::GetThreadCount() {
		return static_cast<int>(std::thread::hardware_concurrency());
	}

	std::string OtherHelper::GetFileExtension(const std::string& path) {
		std::filesystem::path filePath(path);
		std::string extension = filePath.extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		return extension;
	}

	void OtherHelper::SetFileExtension(std::string& path, const std::string& newExtension) {
		try {
			std::filesystem::path oldPath(path);
			std::filesystem::path newPath = oldPath;
			newPath.replace_extension(newExtension);

			if (FileExists(oldPath.string())) {
				std::filesystem::rename(oldPath, newPath);
				path = newPath.string();
			}
			else {
				std::cerr << "File does not exist: " << path << '\n';
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error changing file extension: " << e.what() << '\n';
		}
	}
	std::string OtherHelper::FillString(const std::string& source, const std::string& spilt, int num)
	{
		if (source.empty() || spilt.empty() || num <= 0) {
			return source;
		}
		std::string result = "";
		for (size_t i = 0; i < source.length(); ++i) {
			result += source[i];
			if (i < source.length() - 1) {
				for (int j = 0; j < num; ++j) {
					result += spilt;
				}
			}
		}
		return result;
	}


	std::string OtherHelper::RandomString(int len) {
		const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		std::string result;
		result.reserve(len);

		// 使用C标准库的rand函数，避免C++11 random引起的编译问题
		static bool seeded = false;
		if (!seeded) {
			std::srand(static_cast<unsigned int>(std::time(nullptr)));
			seeded = true;
		}

		for (int i = 0; i < len; ++i) {
			result += chars[std::rand() % chars.size()];
		}

		return result;
	}

	std::string OtherHelper::CenterText(const std::string& input, int width, char symbol) {
		if (static_cast<int>(input.length()) >= width) {
			return input;
		}

		int totalPadding = width - static_cast<int>(input.length());
		int leftPadding = totalPadding / 2;
		int rightPadding = totalPadding - leftPadding;

		return std::string(leftPadding, symbol) + input + std::string(rightPadding, symbol);
	}

	void OtherHelper::CopyFileW(const std::string& sourceDirectory, const std::string& targetDirectory,
		const std::string& fileType, bool overwrite) {
		try {
			if (!DirectoryExists(targetDirectory)) {
				CreateDirectories(targetDirectory);
			}

			std::filesystem::copy_options options = std::filesystem::copy_options::recursive;
			if (overwrite) {
				options |= std::filesystem::copy_options::overwrite_existing;
			}

			if (fileType == "*") {
				std::filesystem::copy(sourceDirectory, targetDirectory, options);
			}
			else {
				// 复制特定类型的文件
				for (const auto& entry : std::filesystem::recursive_directory_iterator(sourceDirectory)) {
					if (entry.is_regular_file()) {
						std::string ext = entry.path().extension().string();
						if (ext == fileType || ("*" + ext) == fileType) {
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
		catch (const std::exception& e) {
			std::cerr << "Error copying files: " << e.what() << '\n';
		}
	}

	void OtherHelper::SetCursorPosition(int left, int top) {

		// ANSI escape sequences for UNIX-like systems
		if (left != 0) {
			if (left > 0) {
				std::cout << "\033[" << left << "C"; // Move right
			}
			else {
				std::cout << "\033[" << (-left) << "D"; // Move left
			}
		}
		if (top != 0) {
			if (top > 0) {
				std::cout << "\033[" << top << "B"; // Move down
			}
			else {
				std::cout << "\033[" << (-top) << "A"; // Move up
			}
		}
	}
	// 辅助函数：安全获取本地时间
	std::tm OtherHelper::GetSafeLocalTime(const std::time_t& time) {
		std::tm tm_result = {};
#ifdef _WIN32
		localtime_s(&tm_result, &time);
#else
		localtime_r(&time, &tm_result);
#endif
		return tm_result;
	}

	// 辅助函数：安全获取本地时间
	std::tm* OtherHelper::GetSafeLocalTime(const std::time_t& time, const bool ptr) {
		std::tm tm_result = {};
#ifdef _WIN32
		localtime_s(&tm_result, &time);
#else
		localtime_r(&time, &tm_result);
#endif
		return &tm_result;
	}

	// 时间相关函数实现
	std::string OtherHelper::GetCurrentYear() {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_year + 1900);
	}


	int OtherHelper::GetCurrentYear(bool temp111) {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_year + 1900;
	}

	std::string OtherHelper::GetCurrentMonth() {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_mon + 1);
	}

	int OtherHelper::GetCurrentMonth(bool temp111) {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_mon + 1;
	}

	std::string OtherHelper::GetCurrentDay() {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_mday);
	}

	int OtherHelper::GetCurrentDay(bool temp111) {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_mday;
	}

	std::string OtherHelper::GetCurrentHour() {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_hour);
	}

	int OtherHelper::GetCurrentHour(bool temp111) {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_hour;
	}

	std::string OtherHelper::GetCurrentMinute() {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_min);
	}

	int OtherHelper::GetCurrentMinute(bool temp111) {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_min;
	}

	std::string OtherHelper::GetCurrentSecond() {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return std::to_string(tm.tm_sec);
	}

	int OtherHelper::GetCurrentSecond(bool temp111) {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);
		return tm.tm_sec;
	}


	std::string OtherHelper::GetCurrentTimeW() {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto tm = GetSafeLocalTime(time_t);

		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
		return oss.str();
	}

	std::string OtherHelper::GetBuildTime() {
		return __DATE__ " " __TIME__;
	}

	// 私有辅助函数实现
	int OtherHelper::LevenshteinDistance(const std::string& a, const std::string& b) {
		const size_t m = a.length();
		const size_t n = b.length();

		if (m == 0) return static_cast<int>(n);
		if (n == 0) return static_cast<int>(m);

		std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

		for (size_t i = 0; i <= m; ++i) {
			dp[i][0] = static_cast<int>(i);
		}
		for (size_t j = 0; j <= n; ++j) {
			dp[0][j] = static_cast<int>(j);
		}

		for (size_t i = 1; i <= m; ++i) {
			for (size_t j = 1; j <= n; ++j) {
				int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
				//dp[i][j] = std::min
				//(
				//	{
				//	dp[i - 1][j] + 1,      // deletion
				//	dp[i][j - 1] + 1,      // insertion
				//	dp[i - 1][j - 1] + cost // substitution
				//	}
				//);
				/*dp[i][j] = std::min(std::min(dp[i - 1][j] + 1, dp[i][j - 1] + 1), dp[i - 1][j - 1] + cost);*/
				auto min = {
						dp[i - 1][j] + 1,      // deletion
						dp[i][j - 1] + 1,      // insertion
						dp[i - 1][j - 1] + cost // substitution
				};
				dp[i][j] = std::min_element(min.begin(), min.end(), [](int a, int b) { return a < b; })[0];
				//dp[i][j] = std::min(dp[i - 1][j] + 1,
				//	std::min(dp[i][j - 1] + 1,
				//		dp[i - 1][j - 1] + cost));
			}
		}

		return dp[m][n];
	}

	std::vector<std::string> OtherHelper::SplitString(const std::string& str, const std::string& delimiter) {
		std::vector<std::string> tokens;
		size_t start = 0;
		size_t end = str.find(delimiter);

		while (end != std::string::npos) {
			tokens.push_back(str.substr(start, end - start));
			start = end + delimiter.length();
			end = str.find(delimiter, start);
		}

		tokens.push_back(str.substr(start));
		return tokens;
	}

	std::string OtherHelper::TrimString(const std::string& str) {
		const std::string whitespace = " \t\n\r\f\v";

		size_t start = str.find_first_not_of(whitespace);
		if (start == std::string::npos) {
			return "";
		}

		size_t end = str.find_last_not_of(whitespace);
		return str.substr(start, end - start + 1);
	}

	std::string OtherHelper::ToLowerString(const std::string& str) {
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		return result;
	}

	bool OtherHelper::FileExists(const std::string& path) {
		return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
	}

	bool OtherHelper::DirectoryExists(const std::string& path) {
		return std::filesystem::exists(path) && std::filesystem::is_directory(path);
	}

	void OtherHelper::CreateDirectories(const std::string& path) {
		try {
			std::filesystem::create_directories(path);
		}
		catch (const std::exception& e) {
			std::cerr << "Error creating directories: " << e.what() << '\n';
		}
	}


	// 模板方法实现
	template<typename T>
	bool OtherHelper::AreEqual(const T& left, const T& right, const std::vector<std::string>& ignoreFieldsName) {
		// 简化实现，对于基本类型直接比较
		if constexpr (std::is_arithmetic_v<T>) {
			return left == right;
		}
		else {
			// 对于复杂类型，需要自定义比较逻辑
			return left == right;
		}
	}


	template<>
	std::string OtherHelper::Tostring<int>(const int& message, char fg, bool coloum)
	{
		return std::to_string(message);
	}


	template<>
	std::string OtherHelper::Tostring<bool>(const bool& message, char fg, bool coloum)
	{
		return std::to_string(message);
	}

	template<>
	std::string OtherHelper::Tostring<double>(const double& message, char fg, bool coloum)
	{

		return std::to_string(message);
	}
	template<>
	std::string OtherHelper::Tostring<float>(const float& message, char fg, bool coloum)
	{

		return std::to_string(message);
	}
	template<>
	std::string OtherHelper::Tostring<std::string>(const std::string& message, char fg, bool coloum)
	{

		return message;
	}
	template<>
	std::string OtherHelper::Tostring<Eigen::VectorXd>(const Eigen::VectorXd& message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum) {
			for (int i = 0; i < message.size(); ++i)
			{
				oss << message(i);
				if (i < message.size() - 1) {
					oss << fg;
				}
			}
		}
		else {
			for (int i = 0; i < message.size(); ++i) {
				oss << message(i);
				if (i < message.size() - 1) {
					oss << fg;
				}
			}
		}
		return oss.str();


	}
	template<>
	std::string OtherHelper::Tostring<Eigen::VectorXf>(const Eigen::VectorXf& message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum) {
			for (int i = 0; i < message.size(); ++i) {
				oss << message(i);
				if (i < message.size() - 1) {
					oss << fg;
				}
			}
		}
		else {
			for (int i = 0; i < message.size(); ++i) {
				oss << message(i);
			}
		}
		return oss.str();


	}



	template<>
	std::string OtherHelper::Tostring<Eigen::MatrixXd>(const Eigen::MatrixXd& message, char fg, bool coloum)
	{
		std::ostringstream oss;

		// Output matrix in column-major format
		for (int i = 0; i < message.rows(); ++i)
		{
			for (int j = 0; j < message.cols(); ++j)
			{
				oss << message(j, j);
				if (j < message.rows() - 1) {
					oss << fg;
				}
			}
			if (i < message.cols() - 1) {
				oss << '\n';
			}
		}

		return oss.str();
	}

	template<>
	std::string OtherHelper::Tostring<Eigen::MatrixXf>(const Eigen::MatrixXf& message, char fg, bool coloum)
	{
		std::ostringstream oss;

		// Output matrix in row-major format
		for (int i = 0; i < message.rows(); ++i) {
			for (int j = 0; j < message.cols(); ++j) {
				oss << message(i, j);
				if (j < message.cols() - 1) {
					oss << fg;
				}
			}
			if (i < message.rows() - 1) {
				oss << '\n';
			}
		}

		return oss.str();
	}

	/// <summary>
/// 将对象转换为字符串表示
/// </summary>
	template<>
	static std::string OtherHelper::Tostring<std::vector<double>>(const std::vector<double>& message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum) {
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1) {
					oss << '\n';
				}
			}
		}
		else {
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1) {
					oss << fg;
				}
			}
		}
		return oss.str();
	}
	template<>
	static std::string OtherHelper::Tostring<std::vector<float>>(const std::vector<float>& message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum) {
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1) {
					oss << '\n';
				}
			}
		}
		else {
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1) {
					oss << fg;
				}
			}
		}
		return oss.str();
	}

	template<>
	static std::string OtherHelper::Tostring<std::vector<int>>(const std::vector<int>& message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum) {
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1) {
					oss << '\n';
				}
			}
		}
		else {
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1) {
					oss << fg;
				}
			}
		}
		return oss.str();
	}

	template<>
	static std::string OtherHelper::Tostring<std::vector<bool>>(const std::vector<bool>& message, char fg, bool coloum)
	{
		std::ostringstream oss;
		if (coloum) {
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1) {
					oss << '\n';
				}
			}
		}
		else {
			for (size_t i = 0; i < message.size(); i++)
			{
				oss << std::to_string(message.at(i));
				if (i < message.size() - 1) {
					oss << fg;
				}
			}
		}
		return oss.str();
	}


	// 显式模板实例化
	template bool OtherHelper::AreEqual<int>(const int&, const int&, const std::vector<std::string>&);
	template bool OtherHelper::AreEqual<double>(const double&, const double&, const std::vector<std::string>&);
	template bool OtherHelper::AreEqual<float>(const float&, const float&, const std::vector<std::string>&);
	template bool OtherHelper::AreEqual<std::string>(const std::string&, const std::string&, const std::vector<std::string>&);






	template<typename T>
	T OtherHelper::ParseLine(const std::vector<std::string>& lines, const std::string& filename,
		std::tuple<int, std::string> pp, std::optional<T> moren,
		int num, char fg, char fg1, int station,
		const std::vector<std::string>* namelist, bool row,
		const std::string* titleLine, bool warning,
		const std::string& errorInf) {
		try {
			int index = std::get<0>(pp);
			std::string key = std::get<1>(pp);

			if (index >= 0 && index < static_cast<int>(lines.size())) {
				std::string line = lines[index];

				// 查找关键字
				size_t keyPos = line.find(key);
				if (keyPos != std::string::npos) {
					// 提取值部分
					std::string valuePart = line.substr(keyPos + key.length());

					// 去除前导空白
					valuePart = TrimString(valuePart);

					// 按分隔符分割
					std::vector<std::string> tokens = SplitString(valuePart, std::string(1, fg));

					if (station < static_cast<int>(tokens.size())) {
						std::string valueStr = TrimString(tokens[station]);

						// 类型转换
						if constexpr (std::is_same_v<T, int>) {
							return std::stoi(valueStr);
						}
						else if constexpr (std::is_same_v<T, double>) {
							return std::stod(valueStr);
						}
						else if constexpr (std::is_same_v<T, float>) {
							return std::stof(valueStr);
						}
						else if constexpr (std::is_same_v<T, std::string>) {
							return valueStr;
						}
						else {
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
			if (moren.has_value()) {
				return moren.value();
			}
			else {
				if (warning) {
					std::cerr << "Warning: Failed to parse line in " << filename
						<< " for key: " << std::get<1>(pp) << '\n';
				}
				return T{};
			}

		}
		catch (const std::exception& e) {
			if (warning) {
				std::cerr << "Error parsing line in " << filename << ": " << e.what() << '\n';
			}

			if (moren.has_value()) {
				return moren.value();
			}
			else {
				return T{};
			}
		}
	}

	template<typename T>
	std::vector<std::tuple<std::string, const std::type_info*, T>> OtherHelper::GetStructFields(const T& structure) {
		// 简化实现
		std::vector<std::tuple<std::string, const std::type_info*, T>> result;
		return result;
	}

	template<typename T>
	std::vector<std::tuple<std::string, const std::type_info*>> OtherHelper::GetStructNameAndType(const T& structure) {
		// 简化实现
		std::vector<std::tuple<std::string, const std::type_info*>> result;
		return result;
	}

	template<typename T>
	bool OtherHelper::IsStruct()
	{
		return std::is_class_v<T> && !std::is_pointer_v<T> && !std::is_reference_v<T>;
	}

	template<typename T>
	bool OtherHelper::IsStructOrStructArray() {
		if constexpr (std::is_array_v<T>) {
			using ElementType = std::remove_extent_t<T>;
			return IsStruct<ElementType>();
		}
		else {
			return IsStruct<T>();
		}
	}

	template<typename T>
	bool OtherHelper::IsList(const T& obj) {
		// 检查是否为vector、list或array类型
		return false; // 简化实现
	}

	template<typename T>
	std::string OtherHelper::GetStructName(const T& structure) {
		return typeid(T).name();
	}

	template<typename T>
	bool OtherHelper::TryConvertToEnum(const std::string& value, T& enumValue) {
		// 简化实现，C++枚举转换需要手动映射
		try {
			enumValue = ConvertToEnum<T>(value);
			return true;
		}
		catch (...) {
			return false;
		}
	}

	template<typename T>
	T OtherHelper::ConvertToEnum(const std::string& value) {
		// 简化实现，实际需要手动映射枚举值
		throw std::invalid_argument("Enum conversion not implemented for this type");
	}

	template<typename MatrixType>
	std::vector<std::string> OtherHelper::ConvertMatrixTitleToOutfile(const std::string& title,
		const std::vector<std::string>& rowtitle,
		const std::vector<std::string>& columtitle,
		const MatrixType& matrix) {
		std::vector<std::string> output;

		// 构建标题行
		std::string titleLine = title;
		for (const auto& rowTitle : rowtitle) {
			titleLine += "\t" + rowTitle;
		}
		output.push_back(titleLine);

		// 构建数据行
		for (size_t i = 0; i < columtitle.size(); ++i) {
			std::string dataLine = columtitle[i];
			for (size_t j = 0; j < rowtitle.size(); ++j) {
				// 这里需要根据实际的矩阵类型来访问元素
				dataLine += "\t0"; // 占位符，实际实现需要访问matrix(i,j)
			}
			output.push_back(dataLine);
		}

		return output;
	}

	template<typename T>
	std::vector<T> OtherHelper::Distinct(const std::vector<T>& values) {
		std::vector<T> result = values;
		std::sort(result.begin(), result.end());
		result.erase(std::unique(result.begin(), result.end()), result.end());
		return result;
	}

	template<typename T>
	std::vector<T> OtherHelper::Duplicates(const std::vector<T>& values) {
		std::unordered_map<T, int> counts;
		std::vector<T> duplicates;

		for (const auto& value : values) {
			counts[value]++;
		}

		for (const auto& pair : counts) {
			if (pair.second > 1) {
				duplicates.push_back(pair.first);
			}
		}

		return duplicates;
	}

	template<typename T, size_t N>
	std::array<T, N> OtherHelper::Distinct(const std::array<T, N>& values) {
		std::vector<T> temp(values.begin(), values.end());
		auto distinctVec = Distinct(temp);

		std::array<T, N> result{};
		size_t copySize = std::min<size_t>(distinctVec.size(), N);
		std::copy(distinctVec.begin(), distinctVec.begin() + copySize, result.begin());

		return result;
	}

	template<typename T, size_t N>
	std::vector<T> OtherHelper::Duplicates(const std::array<T, N>& values) {
		std::vector<T> temp(values.begin(), values.end());
		return Duplicates(temp);
	}

	//// 在头文件中提供模板实现
	//template<typename T>
	//OtherHelper::FixedQueue<T>::FixedQueue(int capacity, const T& value) : num(capacity) {
	//	for (int i = 0; i < capacity; ++i) {
	//		stack.push(value);
	//	}
	//}

	//template<typename T>
	//void OtherHelper::FixedQueue<T>::Push(const T& element) {
	//	if (static_cast<int>(stack.size()) >= num) {
	//		stack.pop(); // 移除最早的元素
	//	}
	//	stack.push(element);
	//}

	//template<typename T>
	//int OtherHelper::FixedQueue<T>::Count() const {
	//	return num;
	//}

	//template<typename T>
	//int OtherHelper::FixedQueue<T>::PushCount() const {
	//	return static_cast<int>(stack.size());
	//}

	//template<typename T>
	//T OtherHelper::FixedQueue<T>::operator[](int index) const {
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
	//template class OtherHelper::FixedQueue<int>
	//template class OtherHelper::FixedQueue<double>
	//template class OtherHelper::FixedQueue<float>
	//template class OtherHelper::FixedQueue<std::string>
	// 显式模板实例化
	template std::vector<int> OtherHelper::Distinct(const std::vector<int>&);
	template std::vector<double> OtherHelper::Distinct(const std::vector<double>&);
	template std::vector<float> OtherHelper::Distinct(const std::vector<float>&);

	template std::vector<int> OtherHelper::Duplicates(const std::vector<int>&);
	template std::vector<double> OtherHelper::Duplicates(const std::vector<double>&);
	template std::vector<float> OtherHelper::Duplicates(const std::vector<float>&);

} // namespace HawtC3::IO::IO