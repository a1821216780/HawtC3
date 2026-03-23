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

#include <string>
#include <vector>
#include <sstream>
#include <type_traits>
#include <optional>
#include <algorithm>
#include <typeinfo>
#include <stdexcept>
#include <iostream>
#include <tuple>
#include "StringHelper.h"

namespace HawtC3::IO::IO
{
	/// <summary>
/// 将字符串中的所有字符转换为大写形式。
/// </summary>
/// <param name="str">要转换为大写的输入字符串。</param>
/// <returns>转换为大写后的新字符串。</returns>
	std::string StringHelper::ToUpper(const std::string& str) {
		std::string result = str;
		for (char& c : result) {
			c = toupper(c);
		}
		return result;
	}
	/// <summary>
	/// 将字符串中的所有字符转换为小写字母。
	/// </summary>
	/// <param name="str">要转换为小写的输入字符串。</param>
	/// <returns>转换为小写后的新字符串。</returns>
	std::string StringHelper::ToLower(const std::string& str) {
		std::string result = str;
		for (char& c : result) {
			c = tolower(c);
		}
		return result;
	}

	/// <summary>
	/// 判断给定字符串是否为回文字符串。
	/// </summary>
	/// <param name="str">要检查的字符串。</param>
	/// <returns>如果字符串是回文，返回 true；否则返回 false。</returns>
	bool StringHelper::IsPalindrome(const std::string& str) {
		if (str.empty()) return true;
		size_t left = 0;
		size_t right = str.length() - 1;
		while (left < right) {
			if (str[left] != str[right]) {
				return false;
			}
			left++;
			right--;
		}
		return true;
	}

	// 生成分割字符串为字符串数组，类似c#Split函数


	std::vector<std::string> StringHelper::Split(const std::string& str, char delimiter) {
		std::vector<std::string> elements;
		std::stringstream ss(str);
		std::string item;
		while (std::getline(ss, item, delimiter)) {
			elements.push_back(item);
		}
		return elements;
	}
	bool StringHelper::StringToBool(const std::string& str)
	{
		// 尝试将字符串转换为整数
		int value = std::stoi(str);
		// 如果值为非零，则返回true，否则返回false
		return value != 0;
	}
}





