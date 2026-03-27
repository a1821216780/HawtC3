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

#pragma once
#ifndef YAML_TYPE_CONVERTER_H
#define YAML_TYPE_CONVERTER_H

#include <string>
#include <vector>
#include <tuple>
#include <type_traits>
#include <Eigen/Dense>

namespace Qahse {
namespace IO {
namespace Yaml {

	/// <summary>
	/// 提供将YAML格式字符串转换为各种数据类型的静态方法
	/// </summary>
	/// <remarks>
	/// 此类包含用于将YAML字符串解析为基本类型、数组和复杂结构（如矩阵和向量）的方法。
	/// 对应C#版本的YMLConvertToObjectiveExtensions扩展方法类。
	///
	/// 使用示例：
	/// <code>
	/// bool flag = YamlTypeConverter::YmlToBool("true", "启用标志");
	/// int count = YamlTypeConverter::YmlToInt("42", "计数");
	/// double value = YamlTypeConverter::YmlToDouble("3.14", "圆周率");
	/// Eigen::VectorXd vec = YamlTypeConverter::YmlToVector("[1.0, 2.0, 3.0]", "坐标");
	/// auto [mat, rows, cols] = YamlTypeConverter::YmlToMatrix("- [1, 2]\n- [3, 4]", "矩阵");
	/// </code>
	/// </remarks>
	class YamlTypeConverter {
	public:

		/// <summary>
		/// 将YAML字符串转换为布尔值
		/// </summary>
		/// <param name="yml">要转换的YAML字符串</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <param name="defaultValue">转换失败时的默认值（默认false）</param>
		/// <returns>解析后的布尔值</returns>
		static bool YmlToBool(const std::string& yml, const std::string& name = "", bool defaultValue = false);

		/// <summary>
		/// 将YAML格式字符串转换为布尔值数组
		/// </summary>
		/// <param name="yml">包含布尔值的YAML格式字符串，如"[true, false, true]"</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>解析后的布尔值数组</returns>
		static std::vector<bool> YmlToBoolArray(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML字符串转换为整数
		/// </summary>
		/// <param name="yml">要转换的YAML字符串</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>解析后的整数值，失败返回0</returns>
		static int YmlToInt(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML格式字符串转换为整数数组
		/// </summary>
		/// <param name="yml">包含整数的YAML格式字符串，如"[1, 2, 3]"</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>解析后的整数数组</returns>
		static std::vector<int> YmlToIntArray(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML字符串转换为双精度浮点数
		/// </summary>
		/// <param name="yml">要转换的YAML字符串</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>解析后的double值，失败返回0.0</returns>
		static double YmlToDouble(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML格式字符串转换为双精度浮点数数组
		/// </summary>
		/// <param name="yml">包含数值的YAML格式字符串，如"[1.5, 2.7, 3.14]"</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>解析后的double数组</returns>
		static std::vector<double> YmlToDoubleArray(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML字符串转换为单精度浮点数
		/// </summary>
		/// <param name="yml">要转换的YAML字符串</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>解析后的float值，失败返回0.0f</returns>
		static float YmlToFloat(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML格式字符串转换为单精度浮点数数组
		/// </summary>
		/// <param name="yml">包含数值的YAML格式字符串，如"[1.1, 2.2, 3.3]"</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>解析后的float数组</returns>
		static std::vector<float> YmlToFloatArray(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML字符串转换为Eigen密集向量
		/// </summary>
		/// <param name="yml">包含数值的YAML格式字符串，如"[1.0, 2.0, 3.0]"</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>解析后的Eigen::VectorXd向量</returns>
		static Eigen::VectorXd YmlToVector(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML字符串原样返回（去除首尾空白）
		/// </summary>
		/// <param name="yml">要处理的YAML字符串</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>处理后的字符串</returns>
		static std::string YmlToString(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML格式字符串转换为字符串数组
		/// </summary>
		/// <param name="yml">包含字符串列表的YAML格式字符串，如"[apple, banana, cherry]"</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>解析后的字符串数组</returns>
		static std::vector<std::string> YmlToStringArray(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML格式字符串转换为二维double数组
		/// </summary>
		/// <param name="yml">包含矩阵数据的YAML字符串，行由"-  "分隔</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>元组(二维数组, 行数, 列数)</returns>
		static std::tuple<std::vector<std::vector<double>>, int, int>
			YmlTo2DDoubleArray(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML格式字符串转换为Eigen矩阵
		/// </summary>
		/// <param name="yml">包含矩阵数据的YAML字符串</param>
		/// <param name="name">用于日志记录的名称标识</param>
		/// <returns>元组(Eigen::MatrixXd矩阵, 行数, 列数)</returns>
		static std::tuple<Eigen::MatrixXd, int, int>
			YmlToMatrix(const std::string& yml, const std::string& name = "");

		/// <summary>
		/// 将YAML字符串转换为枚举值（通过整数转换）
		/// </summary>
		/// <typeparam name="T">枚举类型</typeparam>
		/// <param name="yml">包含枚举名称或整数值的YAML字符串</param>
		/// <returns>对应的枚举值</returns>
		/// <remarks>
		/// C++不支持运行时枚举名称解析，此方法通过整数值进行转换。
		/// 使用时需要确保YAML中存储的是枚举对应的整数值。
		///
		/// 使用示例：
		/// <code>
		/// enum class Status { Active = 0, Inactive = 1, Pending = 2 };
		/// Status s = YamlTypeConverter::YmlToEnum&lt;Status&gt;("1"); // 返回Status::Inactive
		/// </code>
		/// </remarks>
		template <typename T>
		static T YmlToEnum(const std::string& yml) {
			static_assert(std::is_enum_v<T>, "T must be an enum type");
			int intVal = 0;
			try {
				intVal = std::stoi(yml);
			}
			catch (...) {
				// 转换失败，返回默认值
			}
			return static_cast<T>(intVal);
		}

	private:
		/// <summary>
		/// 按字符分隔符拆分字符串
		/// </summary>
		static std::vector<std::string> splitByChars(const std::string& str, const std::string& delimiters);

		/// <summary>
		/// 按子字符串分隔符拆分字符串
		/// </summary>
		static std::vector<std::string> splitByString(const std::string& str, const std::string& delimiter);

		/// <summary>
		/// 移除数组中的空字符串元素
		/// </summary>
		static std::vector<std::string> removeEmpty(const std::vector<std::string>& vec);

		/// <summary>
		/// 去除数组中每个字符串的首尾空白
		/// </summary>
		static std::vector<std::string> trimAll(const std::vector<std::string>& vec);

		/// <summary>
		/// 去除字符串首尾空白
		/// </summary>
		static std::string trim(const std::string& str);

		// 禁止实例化
		YamlTypeConverter() = delete;
	};

} // namespace Yaml
} // namespace IO
} // namespace Qahse

#endif // YAML_TYPE_CONVERTER_H
