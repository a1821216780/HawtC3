//**********************************************************************************************************************************
//LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of Qahse.IO.Yaml
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

#include "YamlTypeConverter.h"
#include "../Log/LogHelper.h"
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace Qahse::IO::Log;
using namespace std;

namespace Qahse {
namespace IO {
namespace Yaml {

// ============================================================================
// 辅助工具函数
// ============================================================================

/// <summary>
/// 按字符分隔符拆分字符串
/// </summary>
std::vector<std::string> YamlTypeConverter::splitByChars(const std::string& str, const std::string& delimiters) {
	std::vector<std::string> tokens;
	size_t start = 0;
	for (size_t i = 0; i <= str.size(); ++i) {
		if (i == str.size() || delimiters.find(str[i]) != std::string::npos) {
			tokens.push_back(str.substr(start, i - start));
			start = i + 1;
		}
	}
	return tokens;
}

/// <summary>
/// 按子字符串分隔符拆分字符串
/// </summary>
std::vector<std::string> YamlTypeConverter::splitByString(const std::string& str, const std::string& delimiter) {
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t pos = 0;
	while ((pos = str.find(delimiter, start)) != std::string::npos) {
		tokens.push_back(str.substr(start, pos - start));
		start = pos + delimiter.size();
	}
	tokens.push_back(str.substr(start));
	return tokens;
}

/// <summary>
/// 移除数组中的空字符串
/// </summary>
std::vector<std::string> YamlTypeConverter::removeEmpty(const std::vector<std::string>& vec) {
	std::vector<std::string> result;
	for (const auto& s : vec) {
		if (!s.empty()) {
			result.push_back(s);
		}
	}
	return result;
}

/// <summary>
/// 去除字符串首尾空白
/// </summary>
std::string YamlTypeConverter::trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\n\r\f\v");
	if (start == std::string::npos) return "";
	size_t end = str.find_last_not_of(" \t\n\r\f\v");
	return str.substr(start, end - start + 1);
}

/// <summary>
/// 去除数组中每个字符串的首尾空白
/// </summary>
std::vector<std::string> YamlTypeConverter::trimAll(const std::vector<std::string>& vec) {
	std::vector<std::string> result;
	result.reserve(vec.size());
	for (const auto& s : vec) {
		result.push_back(trim(s));
	}
	return result;
}


// ============================================================================
// YAML字符串到基本类型的转换方法
// ============================================================================

/// <summary>
/// 将YAML字符串转换为布尔值
/// 支持 "true"/"false"（不区分大小写），转换失败返回默认值并记录错误日志
/// </summary>
bool YamlTypeConverter::YmlToBool(const std::string& yml, const std::string& name, bool defaultValue) {
	if (yml.empty()) return defaultValue;

	std::string lower = yml;
	std::transform(lower.begin(), lower.end(), lower.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	lower = trim(lower);

	if (lower == "true") return true;
	if (lower == "false") return false;

	LogHelper::ErrorLog(
		"Convert " + name + " false! the false string is:" + yml,
		"", "", 20, "YmlToBool"
	);
	return defaultValue;
}

/// <summary>
/// 将YAML格式字符串转换为布尔值数组
/// 输入格式如 "[true, false, true]"
/// </summary>
std::vector<bool> YamlTypeConverter::YmlToBoolArray(const std::string& yml, const std::string& name) {
	if (yml.empty()) return {};

	auto parts = trimAll(removeEmpty(splitByChars(yml, ",[]")));
	std::vector<bool> result;
	result.reserve(parts.size());

	for (const auto& s : parts) {
		std::string lower = s;
		std::transform(lower.begin(), lower.end(), lower.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		if (lower == "true") {
			result.push_back(true);
		}
		else if (lower == "false") {
			result.push_back(false);
		}
		else {
			LogHelper::ErrorLog(
				"Convert " + name + " false! the false string is:" + yml,
				"", "", 20, "YmlToBoolArray"
			);
			result.push_back(false);
		}
	}
	return result;
}

/// <summary>
/// 将YAML字符串转换为整数，转换失败返回0并记录错误日志
/// </summary>
int YamlTypeConverter::YmlToInt(const std::string& yml, const std::string& name) {
	if (yml.empty()) return 0;

	try {
		return std::stoi(trim(yml));
	}
	catch (...) {
		LogHelper::ErrorLog(
			"Convert " + name + " false! the false string is:" + yml,
			"", "", 20, "YmlToInt"
		);
		return 0;
	}
}

/// <summary>
/// 将YAML格式字符串转换为整数数组
/// 输入格式如 "[1, 2, 3, 4]"
/// </summary>
std::vector<int> YamlTypeConverter::YmlToIntArray(const std::string& yml, const std::string& name) {
	if (yml.empty()) return {};

	auto parts = trimAll(removeEmpty(splitByChars(yml, ",[]")));
	std::vector<int> result;
	result.reserve(parts.size());

	for (const auto& s : parts) {
		try {
			result.push_back(std::stoi(s));
		}
		catch (...) {
			LogHelper::ErrorLog(
				"Convert " + name + " false! the false string is:" + yml,
				"", "", 20, "YmlToIntArray"
			);
			result.push_back(0);
		}
	}
	return result;
}

/// <summary>
/// 将YAML字符串转换为双精度浮点数，转换失败返回0.0并记录错误日志
/// </summary>
double YamlTypeConverter::YmlToDouble(const std::string& yml, const std::string& name) {
	if (yml.empty()) return 0.0;

	try {
		return std::stod(trim(yml));
	}
	catch (...) {
		LogHelper::ErrorLog(
			"Convert " + name + " false! the false string is:" + yml,
			"", "", 20, "YmlToDouble"
		);
		return 0.0;
	}
}

/// <summary>
/// 将YAML格式字符串转换为双精度浮点数数组
/// 输入格式如 "[1.5, 2.7, 3.14]"
/// </summary>
std::vector<double> YamlTypeConverter::YmlToDoubleArray(const std::string& yml, const std::string& name) {
	if (yml.empty()) return {};

	auto parts = trimAll(removeEmpty(splitByChars(yml, ",[]")));
	std::vector<double> result;
	result.reserve(parts.size());

	for (const auto& s : parts) {
		try {
			result.push_back(std::stod(s));
		}
		catch (...) {
			LogHelper::ErrorLog(
				"Convert " + name + " false! the false string is:" + yml,
				"", "", 20, "YmlToDoubleArray"
			);
			result.push_back(0.0);
		}
	}
	return result;
}

/// <summary>
/// 将YAML字符串转换为单精度浮点数，转换失败返回0.0f并记录错误日志
/// </summary>
float YamlTypeConverter::YmlToFloat(const std::string& yml, const std::string& name) {
	if (yml.empty()) return 0.0f;

	try {
		return std::stof(trim(yml));
	}
	catch (...) {
		LogHelper::ErrorLog(
			"Convert " + name + " false! the false string is:" + yml,
			"", "", 20, "YmlToFloat"
		);
		return 0.0f;
	}
}

/// <summary>
/// 将YAML格式字符串转换为单精度浮点数数组
/// 输入格式如 "[1.1, 2.2, 3.3]"
/// </summary>
std::vector<float> YamlTypeConverter::YmlToFloatArray(const std::string& yml, const std::string& name) {
	if (yml.empty()) return {};

	auto parts = trimAll(removeEmpty(splitByChars(yml, ",[]")));
	std::vector<float> result;
	result.reserve(parts.size());

	for (const auto& s : parts) {
		try {
			result.push_back(std::stof(s));
		}
		catch (...) {
			LogHelper::ErrorLog(
				"Convert " + name + " false! the false string is:" + yml,
				"", "", 20, "YmlToFloatArray"
			);
			result.push_back(0.0f);
		}
	}
	return result;
}


// ============================================================================
// YAML字符串到复杂类型的转换方法
// ============================================================================

/// <summary>
/// 将YAML格式字符串转换为Eigen密集向量
/// 输入格式如 "[1.0, 2.0, 3.0]"
/// </summary>
Eigen::VectorXd YamlTypeConverter::YmlToVector(const std::string& yml, const std::string& name) {
	if (yml.empty()) return Eigen::VectorXd();

	auto parts = trimAll(removeEmpty(splitByChars(yml, ",[]")));
	Eigen::VectorXd result(parts.size());

	for (size_t i = 0; i < parts.size(); i++) {
		try {
			result(static_cast<int>(i)) = std::stod(parts[i]);
		}
		catch (...) {
			LogHelper::ErrorLog(
				"Convert " + name + " false! the false string is:" + yml,
				"", "", 20, "YmlToVector"
			);
			result(static_cast<int>(i)) = 0.0;
		}
	}
	return result;
}

/// <summary>
/// 将YAML字符串原样返回（去除首尾空白），空字符串返回空
/// </summary>
std::string YamlTypeConverter::YmlToString(const std::string& yml, const std::string& name) {
	if (yml.empty()) return "";
	return yml;
}

/// <summary>
/// 将YAML格式字符串转换为字符串数组
/// 输入格式如 "[apple, banana, cherry]"
/// </summary>
std::vector<std::string> YamlTypeConverter::YmlToStringArray(const std::string& yml, const std::string& name) {
	if (yml.empty()) return {};
	return trimAll(removeEmpty(splitByChars(yml, ",[]")));
}

/// <summary>
/// 将YAML格式字符串转换为二维double数组
/// 每行由 "-  " 分隔，行内值由逗号分隔
/// </summary>
/// <returns>元组(二维数组, 行数, 列数)</returns>
std::tuple<std::vector<std::vector<double>>, int, int>
YamlTypeConverter::YmlTo2DDoubleArray(const std::string& yml, const std::string& name) {
	if (yml.empty()) return { {}, 0, 0 };

	// 按 "-  " 分隔行
	auto rows = removeEmpty(trimAll(removeEmpty(splitByString(yml, "-  "))));
	int rowCount = static_cast<int>(rows.size());
	if (rowCount == 0) return { {}, 0, 0 };

	// 解析第一行确定列数
	auto firstRowParts = trimAll(removeEmpty(splitByChars(rows[0], ",[]")));
	int colCount = static_cast<int>(firstRowParts.size());

	std::vector<std::vector<double>> result(rowCount, std::vector<double>(colCount, 0.0));

	for (int i = 0; i < rowCount; i++) {
		auto parts = trimAll(removeEmpty(splitByChars(rows[i], ",[]")));
		for (int j = 0; j < colCount && j < static_cast<int>(parts.size()); j++) {
			try {
				result[i][j] = std::stod(parts[j]);
			}
			catch (...) {
				LogHelper::ErrorLog(
					"Convert " + name + " false! the false string is:" + yml,
					"", "", 20, "YmlTo2DDoubleArray"
				);
			}
		}
	}
	return { result, rowCount, colCount };
}

/// <summary>
/// 将YAML格式字符串转换为Eigen矩阵
/// 每行由 "-  " 分隔，行内值由逗号分隔
/// </summary>
/// <returns>元组(Eigen::MatrixXd矩阵, 行数, 列数)</returns>
std::tuple<Eigen::MatrixXd, int, int>
YamlTypeConverter::YmlToMatrix(const std::string& yml, const std::string& name) {
	if (yml.empty()) return { Eigen::MatrixXd(), 0, 0 };

	// 按 "-  " 分隔行
	auto rows = removeEmpty(trimAll(removeEmpty(splitByString(yml, "-  "))));
	int rowCount = static_cast<int>(rows.size());
	if (rowCount == 0) return { Eigen::MatrixXd(), 0, 0 };

	// 解析第一行确定列数
	auto firstRowParts = trimAll(removeEmpty(splitByChars(rows[0], ",[]")));
	int colCount = static_cast<int>(firstRowParts.size());

	Eigen::MatrixXd result = Eigen::MatrixXd::Zero(rowCount, colCount);

	for (int i = 0; i < rowCount; i++) {
		auto parts = trimAll(removeEmpty(splitByChars(rows[i], ",[]")));
		for (int j = 0; j < colCount && j < static_cast<int>(parts.size()); j++) {
			try {
				result(i, j) = std::stod(parts[j]);
			}
			catch (...) {
				LogHelper::ErrorLog(
					"Convert " + name + " false! the false string is:" + yml,
					"", "", 20, "YmlToMatrix"
				);
			}
		}
	}
	return { result, rowCount, colCount };
}

} // namespace Yaml
} // namespace IO
} // namespace Qahse
