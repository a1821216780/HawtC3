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

#include <string>
#include <vector>
#include <sstream>
#include <type_traits>
#include <optional>
#include <algorithm>
#include <typeinfo>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <tuple>

#include "StringHelper.h"
#include "../log/LogHelper.h"
#include "Extensions.h"

using namespace Qahse::IO::Log;

namespace Qahse::IO::IO
{

	/// @brief 将字符串中的所有字符转换为大写字母
	/// @param str 要转换为大写的输入字符串
	/// @return 转换为大写后的新字符串
	std::string StringHelper::ToUpper(const std::string &str)
	{
		std::string result = str;
		for (char &c : result)
		{
			c = toupper(c);
		}
		return result;
	}

	/// @brief 将字符串中的所有字符转换为大写字母（就地修改）
	/// @param str 要转换为大写的输入字符串（引用传递）
	void StringHelper::ToUpper(std::string &str)
	{
		for (char &c : str)
		{
			c = toupper(c);
		}
	}

	/// @brief 将字符串中的所有字符转换为小写字母
	/// @param str 要转换为小写的输入字符串
	/// @return 转换为小写后的新字符串
	std::string StringHelper::ToLower(const std::string &str)
	{
		std::string result = str;
		for (char &c : result)
		{
			c = tolower(c);
		}
		return result;
	}

	/// @brief 将字符串中的所有字符转换为小写字母（就地修改）
	/// @param str 要转换为小写的输入字符串（引用传递）
	void StringHelper::ToLower(std::string &str)
	{
		for (char &c : str)
		{
			c = tolower(c);
		}
	}

	/// @brief 判断给定字符串是否为回文字符串(正读和反读都一样的字符串)
	/// @param str 要检查的字符串
	/// @return 如果字符串是回文，返回 true；否则返回 false
	bool StringHelper::IsPalindrome(const std::string &str)
	{
		if (str.empty())
			return true;
		size_t left = 0;
		size_t right = str.length() - 1;
		while (left < right)
		{
			if (str[left] != str[right])
			{
				return false;
			}
			left++;
			right--;
		}
		return true;
	}

	/// @brief 分割字符串为字符串数组，类似C#的Split函数
	/// @param str 要分割的字符串
	/// @param delimiter 分隔符
	/// @return 分割后的字符串数组
	std::vector<std::string> StringHelper::Split(const std::string &str, char delimiter)
	{
		std::vector<std::string> elements;
		std::stringstream ss(str);
		std::string item;
		while (std::getline(ss, item, delimiter))
		{
			if (!item.empty())
			{
				elements.push_back(item);
			}
		}
		return elements;
	}

	/// @brief 分割字符串为字符串数组，使用多个分隔符
	/// @param str 要分割的字符串
	/// @param delimiter 多个分隔符组成的字符串
	/// @return 分割后的字符串数组
	std::vector<std::string> StringHelper::Split(const std::string &str, const char *delimiter)
	{
		std::vector<std::string> elements;
		size_t start = 0;
		size_t end = str.find_first_of(delimiter);
		while (end != std::string::npos)
		{
			elements.push_back(str.substr(start, end - start));
			start = end + 1;
			end = str.find_first_of(delimiter, start);
		}
		elements.push_back(str.substr(start)); // 添加最后一个元素
		return elements;
	}

	/// @brief 去除字符串首尾的空白字符
	/// @param str 要处理的字符串
	/// @return 去除首尾空白字符后的字符串
	std::string StringHelper::Trim(const std::string &str)
	{
		const char *whitespace = " \t\n\r\f\v";
		size_t start = str.find_first_not_of(whitespace);
		if (start == std::string::npos)
		{
			return ""; // 字符串全是空白
		}
		size_t end = str.find_last_not_of(whitespace);
		return str.substr(start, end - start + 1);
	}

	/// @brief 去除字符串首部的空白字符
	/// @param str 要处理的字符串
	/// @return 去除首部空白字符后的字符串
	std::string StringHelper::TrimStart(const std::string &str)
	{
		const char *whitespace = " \t\n\r\f\v";
		size_t start = str.find_first_not_of(whitespace);
		if (start == std::string::npos)
		{
			return ""; // 字符串全是空白
		}
		return str.substr(start);
	}

	/// @brief 去除字符串尾部的空白字符
	/// @param str 要处理的字符串
	/// @return 去除尾部空白字符后的字符串
	std::string StringHelper::TrimEnd(const std::string &str)
	{
		const char *whitespace = " \t\n\r\f\v";
		size_t end = str.find_last_not_of(whitespace);
		if (end == std::string::npos)
		{
			return ""; // 字符串全是空白
		}
		return str.substr(0, end + 1);
	}

	/// @brief 替换字符串中的指定子字符串
	/// @param str 要处理的字符串
	/// @param oldstr 要被替换的子字符串
	/// @param newstr 替换后的子字符串
	/// @return 替换后的字符串
	std::string StringHelper::Replace(const std::string &str, const std::string &oldstr, const std::string &newstr)
	{
		std::string result = str;
		size_t pos = 0;
		while ((pos = result.find(oldstr, pos)) != std::string::npos)
		{
			result.replace(pos, oldstr.length(), newstr);
			pos += newstr.length(); // Move past the replacement
		}
		return result;
	}

	/// @brief 替换字符串中的指定字符
	/// @param str 要处理的字符串
	/// @param oldstr 要被替换的字符
	/// @param newstr 替换后的字符
	/// @return 替换后的字符串
	std::string StringHelper::Replace(const std::string &str, const char &oldstr, const char &newstr)
	{
		std::string result = str;
		std::replace(result.begin(), result.end(), oldstr, newstr);
		return result;
	}

	/// @brief 替换字符串中的指定字符为字符串
	/// @param str 要处理的字符串
	/// @param oldstr 要被替换的字符
	/// @param newstr 替换后的字符串
	/// @return 替换后的字符串
	std::string StringHelper::Replace(const std::string &str, const char &oldstr, const std::string &newstr)
	{
		std::string result = str;
		size_t pos = 0;
		while ((pos = result.find(oldstr, pos)) != std::string::npos)
		{
			result.replace(pos, 1, newstr);
			pos += newstr.length(); // Move past the replacement
		}
		return result;
	}

	/// @brief 替换字符串中的指定子字符串为字符
	/// @param str 要处理的字符串
	/// @param oldstr 要被替换的子字符串
	/// @param newstr 替换后的字符
	/// @return 替换后的字符串
	std::string StringHelper::Replace(const std::string &str, const std::string &oldstr, const char &newstr)
	{
		std::string result = str;
		size_t pos = 0;
		while ((pos = result.find(oldstr, pos)) != std::string::npos)
		{
			result.replace(pos, oldstr.length(), std::string(1, newstr));
			pos += 1; // Move past the replacement
		}
		return result;
	}

	/// @brief 将字符串转换为布尔值
	/// @param str 要转换的字符串
	/// @return 转换后的布尔值
	bool StringHelper::StringToBool(const std::string &str)
	{
		// 尝试将字符串转换为整数
		int value = std::stoi(str);
		// 如果值为非零，则返回true，否则返回false
		return value != 0;
	}

	/// @brief 	从指定文件路径读取所有行，并返回一个包含每行内容的字符串数组
	/// @param path 文件路径
	/// @return 包含每行内容的字符串数组
	std::vector<std::string> StringHelper::ReadAllLines(const std::string &path)
	{
		std::vector<std::string> lines;
		auto paths = Extensions::GetABSPath(path);
		// 如果文件不存在则报错
		std::ifstream file(paths);
		if (!file.is_open())
		{
			LogHelper::ErrorLog("无法打开文件: " + paths, "", "", 20, "StringHelper::ReadAllLines");
			LogHelper::ErrorLog("无法打开文件: " + paths);
		}
		std::string line;
		while (std::getline(file, line))
		{
			lines.push_back(line);
		}
		return lines;
	}

	/// @brief 将字符串数组写入指定文件路径，每个元素占一行
	/// @param path 文件路径
	/// @param lines 要写入文件的字符串数组
	void StringHelper::WriteAllLines(const std::string &path, const std::vector<std::string> &lines)
	{
		auto paths = Extensions::GetABSPath(path);
		std::ofstream file(paths);
		if (!file.is_open())
		{
			LogHelper::ErrorLog("无法打开文件: " + paths, "", "", 20, "StringHelper::WriteAllLines");
			LogHelper::ErrorLog("无法打开文件: " + paths);
		}
		for (const auto &line : lines)
		{
			file << line << std::endl;
		}
		file.close();
	}
}
