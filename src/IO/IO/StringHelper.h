
//**********************************************************************************************************************************
//LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,������
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
#ifndef STRINGHELPER_H
#define STRINGHELPER_H


namespace HawtC3::IO::IO
{
	class StringHelper
	{
	public:
		static std::string ToUpper(const std::string& str);
		static void ToUpper(std::string& str);

		static std::string ToLower(const std::string& str);
		static void ToLower(std::string& str);

		static bool IsPalindrome(const std::string& str);

		static std::vector<std::string> Split(const std::string& str, char delimiter);
		static std::vector<std::string> Split(const std::string& str, const char* delimiter);

		static std::string Trim(const std::string& str);
		static std::string TrimStart(const std::string& str);
		static std::string TrimEnd(const std::string& str);




		static std::string Replace(const std::string& str, const std::string& oldstr, const std::string& newstr);
		static std::string Replace(const std::string& str, const char& oldstr, const char& newstr);
		static std::string Replace(const std::string& str, const char& oldstr, const std::string& newstr);
		static std::string Replace(const std::string& str, const std::string& oldstr, const char& newstr);

		static std::string RemoveNull(const std::vector<std::string>& str) 
		{
			std::string result;
			for (const auto& s : str) {
				if (!s.empty()) {
					result += s;
				}
			}
			return result;

		}


		static bool StringToBool(const std::string& str);
		static std::vector<std::string> ReadAllLines(const std::string& path);
		static void WriteAllLines(const std::string& path, const std::vector<std::string>& lines);
	};
}
#endif // STRINGHELPER_H


