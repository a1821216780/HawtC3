#pragma once

//**********************************************************************************************************************************
// LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,������
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

#include <Eigen/Dense>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <cctype>

#include "../Log/LogHelper.h"

namespace Qahse::IO::IO
{

	class Extensions
	{
	public:
		// Matrix extensions for double
		static double Max(const Eigen::MatrixXd &value);
		static double Min(const Eigen::MatrixXd &value);
		static double Mean(const Eigen::MatrixXd &value);
		static double AbsMax(const Eigen::MatrixXd &value);
		static double AbsMin(const Eigen::MatrixXd &value);
		static Eigen::VectorXd Std(const Eigen::MatrixXd &value, int dim = 1);

		// Matrix extensions for float
		static float Max(const Eigen::MatrixXf &value);
		static float Min(const Eigen::MatrixXf &value);
		static float Mean(const Eigen::MatrixXf &value);
		static float AbsMax(const Eigen::MatrixXf &value);
		static float AbsMin(const Eigen::MatrixXf &value);
		static Eigen::VectorXf Std(const Eigen::MatrixXf &value, int dim = 1);

		// String extensions
		static std::string GetABSPath(const std::string &str);
		static std::string GetDirectoryName(const std::string &str);
		static std::string GetFileNameWithoutExtension(const std::string &str);

		// String array operations
		static std::vector<std::string> RemoveNull(const std::vector<std::string> &array);
		static std::vector<std::string> TrimNull(const std::vector<std::string> &array);

		// String to value conversions
		static int ToInt(const std::string &str);
		static std::vector<int> ToIntArray(const std::string &str, const std::vector<char> &split);
		static double ToDouble(const std::string &str);
		static std::vector<double> ToDoubleArray(const std::string &str, const std::vector<char> &split);
		static bool ToBool(const std::string &str);
		static std::vector<bool> ToBoolArray(const std::string &str, const std::vector<char> &split);
		static std::vector<std::string> ToStringArray(const std::string &str, const std::vector<char> &split);

	private:
		Extensions() = delete; // class
	};

} // namespace IO
