//**********************************************************************************************************************************
// LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of Qahse.IO.Log.
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
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <algorithm>

#include "Outfile.h"
#include "Extensions.h"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/os.h>
#include <fmt/chrono.h>

using namespace Qahse::IO::IO;
using namespace Qahse::IO::Log;

namespace Qahse::IO::IO
{
	/**
	 * @brief 构造并初始化输出文件流。
	 * @details 自动处理文件扩展名（若路径无扩展名则自动添加 `.3zout`），
	 *          初始化 fmt 格式化字符串，检查文件名冲突，创建输出文件流并将当前对象加入全局列表。
	 * @param path          输出文件路径（可不含扩展名）。
	 * @param decimalPlaces 小数位数（固定小数记法，与 Scientific 互斥），传 -1 则不使用实数记法。
	 * @param Scientific    科学计数法有效位数（与 decimalPlaces 互斥），传 -1 则不使用科学计数法。
	 * @throws std::invalid_argument 相关参数均为 -1 时抛出。
	 * @code
	 * OutFile out("result", 6, -1);           // 固定小数模式，6 位小数
	 * OutFile sci("result_sci", -1, 4);       // 科学计数法，4 位有效数字
	 * @endcode
	 */
	OutFile::OutFile(const std::string &path, int decimalPlaces, int Scientific)
	{

		// 处理文件扩展名
		std::string finalPath = Extensions::GetABSPath(path); ///< 转换为绝对路径
		if (GetFileExtension(path).empty())
		{
			finalPath = path + ".3zout"; ///< 若无扩展名则自动添加 `.3zout`
		}

		filename_ = finalPath;											   ///< 存储文件结局完整路径
		StreanFilePath = finalPath;										   ///< 流文件路径（初始与 filename_ 相同）
		StreamName = std::filesystem::path(finalPath).filename().string(); ///< 仅含文件名的短名（不含目录）

		// 初始化格式化字符串
		initializeFormat(decimalPlaces, Scientific);

		// 检查文件名冲突
		checkFileNameConflict();

		// 创建文件流
		try
		{

			_writer = std::make_unique<std::ofstream>(finalPath);
			if (!_writer->is_open() || !_writer->good())
			{
				LogHelper::ErrorLog("无法打开文件: " + finalPath);
			}
		}
		catch (const std::exception &e)
		{
			LogHelper::ErrorLog("当前路径 " + finalPath + " 被其他程序占用，或没有该文件夹无法继续！", "", "", 20, "OutFile::OutFile");
			throw;
		}

		if (!_writer)
		{
			LogHelper::ErrorLog("读写器没有初始化");
		}

		// 添加到全局文件列表
		LogData::OutFilelist.push_back(this);
	}

	/**
	 * @brief 获取输出文件的完整路径。
	 * @return 包含路径和扩展名的完整文件名字符串。
	 * @code
	 * OutFile out("result", 6, -1);
	 * std::string name = out.GetFilename(); // 例如 "C:\\path\\result.3zout"
	 * @endcode
	 */
	std::string OutFile::GetFilename()
	{
		return filename_;
	}

	/**
	 * @brief 获取流文件路径。
	 * @return 当前设置的流文件路径字符串。
	 * @code
	 * std::string p = out.GetStreamFilePath();
	 * @endcode
	 */
	std::string OutFile::GetStreamFilePath()
	{
		return StreanFilePath;
	}

	/**
	 * @brief 设置流文件路径。
	 * @param path 新的流文件路径字符串。
	 */
	void OutFile::SetStreamFilePath(const std::string &path)
	{
		StreanFilePath = path;
	}

	/**
	 * @brief 析构函数，自动关闭文件流。
	 * @details 若文件流仍然开放，则调用 Outfinish() 安全关闭并从全局列表中移除。
	 */
	OutFile::~OutFile()
	{
		if (_writer && _writer->is_open())
		{
			Outfinish();
		}
	}

	/**
	 * @brief 初始化 fmt 格式化字符串。
	 * @details 根据参数选择固定小数记法或科学计数法，构建相应的 fmt 格式字符串并存入 `filemat_`。
	 * @param decimalPlaces 小数位数，传 -1 表示不使用固定小数格式。
	 * @param Scientific    科学计数法有效位数，传 -1 表示不使用科学计数法格式。
	 * @throws std::invalid_argument 独立参数均为 -1 时抛出。
	 */
	void OutFile::initializeFormat(int decimalPlaces, int Scientific)
	{
		if (Scientific != -1)
		{									// 科学计数法模式
			useScientific_ = true;			///< 启用科学计数法模式
			decimalPlaces_ = decimalPlaces; ///< 小数位数（科学计数法下无意义）
			scientificDigits_ = Scientific; ///< 科学计数法有效位数

			// 构建科学计数法格式字符串
			filemat_ = "{:." + std::to_string(scientificDigits_) + "E}"; ///< fmt 格式字符串，例如 "{:.4E}"
		}
		else if (decimalPlaces != -1)
		{									// 有限小数记法
			useScientific_ = false;			///< 禁用科学计数法模式
			decimalPlaces_ = decimalPlaces; ///< 小数点后保留位数
			scientificDigits_ = Scientific; ///< 保存传入的 Scientific 值（-1）

			filemat_ = "{:." + std::to_string(decimalPlaces_) + "f}"; ///< fmt 格式字符串，例如 "{:.6f}"
		}
		else
		{
			LogHelper::ErrorLog("OutFile 不允许不指定小数位数！", "", "", 20, "initializeFormat");
			throw std::invalid_argument("必须指定 decimalPlaces 或 Scientific 参数之一");
		}
	}

	/**
	 * @brief 使用 fmt 库将 double 数值格式化为字符串（内联辅助函数）。
	 * @details 根据 `filemat_` 格式字符串进行格式化，即固定小数或科学计数法格式。
	 * @param value 要格式化的 double 数值。
	 * @return 格式化后的字符串，例如 `"3.141593"` 或 `"3.1416E+00"`。
	 */
	inline std::string OutFile::formatDouble(double value) const
	{
		std::string res = fmt::format(filemat_, value);
		return res;
	}

	/**
	 * @brief 获取文件路径中的扩展名（包括点号）。
	 * @param path 要解析的文件路径字符串。
	 * @return 扩展名字符串（如 `".3zout"`），若无扩展名则返回空字符串。
	 * @code
	 * std::string ext = out.GetFileExtension("result.3zout"); // ext == ".3zout"
	 * std::string none = out.GetFileExtension("result");      // none == ""
	 * @endcode
	 */
	std::string OutFile::GetFileExtension(const std::string &path) const
	{
		size_t dotPos = path.find_last_of('.');
		if (dotPos != std::string::npos && dotPos < path.length() - 1)
		{
			return path.substr(dotPos);
		}
		return "";
	}

	/**
	 * @brief 检查当前文件名是否与全局 `LogData::OutFilelist` 中的现有文件名冲突。
	 * @details 若发现重复文件名，将记录错误日志。该情况通常只出现在调试模式下。
	 */
	void OutFile::checkFileNameConflict() const
	{
		for (const auto &existingFile : LogData::OutFilelist)
		{
			if (this->filename_ == existingFile->GetFilename())
			{
				LogHelper::ErrorLog("当前文件名称 " + std::filesystem::path(filename_).filename().string() +
									" 与 LogData.OutFilelist 当中的文件路径和名称重合，这是不允许的！，这个错误只会发生在调试模式");
			}
		}
	}

	/**
	 * @brief 将字符串内容写入文件（不换行）。
	 * @param message 要写入的字符串内容。
	 * @param fg      写入内容后附加的分隔符或尾缀字符串，默认为空字符串。
	 * @code
	 * out.Write("Channel1", "\t"); // 写入 "Channel1\t"
	 * @endcode
	 */
	void OutFile::Write(const std::string &message, const std::string &fg)
	{
		*_writer << message << fg;
	}

	/**
	 * @brief 将 double 数值格式化后写入文件（不换行）。
	 * @param message 要写入的 double 值。
	 * @param fg      写入内容后附加的分隔符，默认为空字符串。
	 * @code
	 * out.Write(3.14, "\t"); // 写入 "3.140000\t"(具体格式取决于初始化时的参数）
	 * @endcode
	 */
	void OutFile::Write(double message, const std::string &fg)
	{
		*_writer << formatDouble(message) << fg;
	}

	/**
	 * @brief 将 Eigen::VectorXd 向量按元素逐个写入文件（不换行）。
	 * @param message 要写入的 double 向量。
	 * @param fg      元素间的分隔符，默认为空字符串。
	 * @code
	 * Eigen::VectorXd v(3); v << 1.0, 2.0, 3.0;
	 * out.Write(v, "\t"); // 写入 "1.000000\t2.000000\t3.000000\t"
	 * @endcode
	 */
	void OutFile::Write(const Eigen::VectorXd &message, const std::string &fg)
	{
		for (int i = 0; i < message.size(); i++)
		{
			*_writer << formatDouble(message[i]) << fg;
		}
	}

	/**
	 * @brief 将 Eigen::VectorXf 向量按元素逐个写入文件（自动转为 double）。
	 * @param message 要写入的 float 向量。
	 * @param fg      元素间的分隔符。
	 */
	void OutFile::Write(const Eigen::VectorXf &message, const std::string &fg)
	{
		for (int i = 0; i < message.size(); i++)
		{
			*_writer << formatDouble(static_cast<double>(message[i])) << fg;
		}
	}

	/**
	 * @brief 将 Eigen::MatrixXd 矩阵按行写入文件，行内元素用 fg 分隔，行末进行换行。
	 * @param message 要写入的 double 矩阵。
	 * @param fg      行内元素间的分隔符。
	 * @code
	 * Eigen::MatrixXd m(2,2); m << 1,2,3,4;
	 * out.Write(m, "\t");
	 * // 写入:
	 * // "1.000000\t2.000000\t"
	 * // "3.000000\t4.000000\t"
	 * @endcode
	 */
	void OutFile::Write(const Eigen::MatrixXd &message, const std::string &fg)
	{
		for (int i = 0; i < message.rows(); i++)
		{
			for (int j = 0; j < message.cols(); j++)
			{
				*_writer << formatDouble(message(i, j)) << fg;
			}
			WriteLine();
		}
	}

	/**
	 * @brief 将 Eigen::MatrixXf 矩阵按行写入文件（自动转为 double）。
	 * @param message 要写入的 float 矩阵。
	 * @param fg      行内元素间的分隔符。
	 */
	void OutFile::Write(const Eigen::MatrixXf &message, const std::string &fg)
	{
		for (int i = 0; i < message.rows(); i++)
		{
			for (int j = 0; j < message.cols(); j++)
			{
				*_writer << formatDouble(static_cast<double>(message(i, j))) << fg;
			}
			WriteLine();
		}
	}

	/**
	 * @brief 将字符串数组逐个写入文件（不换行）。
	 * @param fg       每个元素写入后附加的分隔符。
	 * @param messages 要写入的字符串数组。
	 */
	void OutFile::Write(const std::string &fg, const std::vector<std::string> &messages)
	{
		for (const auto &message : messages)
		{
			Write(message, fg);
		}
	}

	/**
	 * @brief 将 double 数组逐个格式化后写入文件（不换行）。
	 * @param fg       每个元素写入后附加的分隔符。
	 * @param messages 要写入的 double 数组。
	 */
	void OutFile::Write(const std::string &fg, const std::vector<double> &messages)
	{
		for (const auto &message : messages)
		{
			Write(message, fg);
		}
	}

	/**
	 * @brief 写入一个换行符。
	 */
	void OutFile::WriteLine()
	{
		*_writer << '\n';
	}

	/**
	 * @brief 将 double 数值格式化后写入文件并换行。
	 * @param message 要写入的 double 数值。
	 */
	void OutFile::WriteLine(double message)
	{
		*_writer << formatDouble(message) << '\n';
	}

	/**
	 * @brief 将字符串内容写入文件并换行。
	 * @param message 要写入的字符串。
	 */
	void OutFile::WriteLine(const std::string &message)
	{
		*_writer << message << '\n';
	}

	/**
	 * @brief 写入文件头部信息（项目名、版权、计算时间）。
	 * @details 将以下信息逐行写入：
	 *  1. 项目名称和官网地址
	 *  2. 版权信息
	 *  3. 计算时间戳（`YYYY-MM-DD HH:MM:SS`）
	 * @param title 输出文件对应的项目名称，如风机型号或仿真类型。
	 * @code
	 * out.WriteTitle("NREL_5MW");
	 * // 写入: "HawtC2_ProjectName: NREL_5MW ..."
	 * @endcode
	 */
	void OutFile::WriteTitle(const std::string &title)
	{
		*_writer << "HawtC2_ProjectName: " << title << " 更多消息和教程请访问：http://www.hawtc.cn/" << '\n';
		*_writer << "Powered By Qahse.HawtC2." << OtherHelper::GetCurrentProjectName() << " @CopyRight 赵子祯" << '\n';
		// 获取当前时间
		auto now = std::chrono::system_clock::now();
		std::time_t time_t = std::chrono::system_clock::to_time_t(now);
		std::tm tm = OtherHelper::GetSafeLocalTime(time_t);
		*_writer << "计算时间：" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '\n';
	}

	/**
	 * @brief 将字符串数组写入文件。
	 * @details 支持行模式（tab 分隔 + 换行）和列模式（每元素后换行）。
	 * @param messages 要写入的字符串数组。
	 * @param row      true 表示行模式（元素用 tab 分隔，结尾换行）；false 表示列模式（每元素独占一行）。
	 * @code
	 * out.WriteLine({"T", "Fx", "Fy"}, true);  // 写入 "T\tFx\tFy\n"
	 * out.WriteLine({"T", "Fx", "Fy"}, false); // 写入 "T\nFx\nFy\n"
	 * @endcode
	 */
	void OutFile::WriteLine(const std::vector<std::string> &messages, bool row)
	{
		if (row)
		{
			for (const auto &message : messages)
			{
				*_writer << message << '\t';
			}
			*_writer << '\n';
		}
		else
		{
			for (const auto &message : messages)
			{
				*_writer << message << '\n';
			}
		}
	}

	/**
	 * @brief 将 Eigen::VectorXd 向量写入文件一行，tab 分隔，可指定小数位数覆盖默认设置。
	 * @param message       要写入的 double 向量。
	 * @param decimalPlaces 临时覆盖的小数位数（僅对本次写入有效）。
	 * @code
	 * Eigen::VectorXd v(3); v << 1.0, 2.0, 3.0;
	 * out.WriteLine(v, 4); // 写入 "1.0000\t2.0000\t3.0000\n"
	 * @endcode
	 */
	void OutFile::WriteLine(const Eigen::VectorXd &message, int decimalPlaces)
	{
		// 创建行矩阵并输出
		Eigen::MatrixXd rowMatrix = message.transpose();

		// 临时修改精度设置
		std::ostringstream oss;
		if (useScientific_)
		{
			oss << std::scientific << std::setprecision(decimalPlaces);
		}
		else
		{
			oss << std::fixed << std::setprecision(decimalPlaces);
		}

		for (int j = 0; j < rowMatrix.cols(); j++)
		{
			oss << rowMatrix(0, j);
			if (j < rowMatrix.cols() - 1)
			{
				oss << "\t";
			}
		}

		*_writer << oss.str() << '\n';
	}

	/**
	 * @brief 将 Eigen::VectorXf 向量写入文件一行（自动转为 double）。
	 * @param message       要写入的 float 向量。
	 * @param decimalPlaces 小数位数覆盖。
	 */
	void OutFile::WriteLine(const Eigen::VectorXf &message, int decimalPlaces)
	{
		// 转换为double向量并调用对应方法
		Eigen::VectorXd doubleVector = message.cast<double>();
		WriteLine(doubleVector, decimalPlaces);
	}

	/**
	 * @brief 将 Eigen::MatrixXd 矩阵写入文件，行内元素用 tab 分隔，行末换行。
	 * @param message 要写入的 double 矩阵。
	 */
	void OutFile::WriteLine(const Eigen::MatrixXd &message)
	{
		for (int i = 0; i < message.rows(); i++)
		{
			for (int j = 0; j < message.cols(); j++)
			{
				*_writer << formatDouble(message(i, j));
				if (j < message.cols() - 1)
				{
					*_writer << "\t";
				}
			}
			*_writer << '\n';
		}
	}

	/**
	 * @brief 将 Eigen::MatrixXf 矩阵写入文件（自动转为 double）。
	 * @param message 要写入的 float 矩阵。
	 */
	void OutFile::WriteLine(const Eigen::MatrixXf &message)
	{
		// 转换为double矩阵并调用对应方法
		Eigen::MatrixXd doubleMatrix = message.cast<double>();
		WriteLine(doubleMatrix);
	}

	/**
	 * @brief 将 `std::vector<double>` 写入文件一行（转换为 Eigen 向量后调用对应重载）。
	 * @param messages 要写入的 double 向量。
	 */
	void OutFile::WriteLine(const std::vector<double> &messages)
	{
		// 转换为Eigen向量并调用对应方法
		Eigen::VectorXd eigenVector(messages.size());
		for (size_t i = 0; i < messages.size(); i++)
		{
			eigenVector[i] = messages[i];
		}
		WriteLine(eigenVector);
	}

	/**
	 * @brief 将二维 `std::vector<vector<double>>` 写入文件（转换为 Eigen 矩阵后调用对应重载）。
	 * @param message 要写入的二维 double 矩阵（外层为行，内层为列）。
	 * @code
	 * std::vector<std::vector<double>> mat = {{1.0, 2.0}, {3.0, 4.0}};
	 * out.WriteLine(mat);
	 * // 写入: "1.000000\t2.000000\n3.000000\t4.000000\n"
	 * @endcode
	 */
	void OutFile::WriteLine(const std::vector<std::vector<double>> &message)
	{
		// 转换为Eigen矩阵并调用对应方法
		if (message.empty() || message[0].empty())
		{
			return;
		}

		int rows = message.size();
		int cols = message[0].size();
		Eigen::MatrixXd eigenMatrix(rows, cols);

		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				eigenMatrix(i, j) = message[i][j];
			}
		}

		WriteLine(eigenMatrix);
	}

	/**
	 * @brief 关闭文件流，并可选从全局文件列表中移除当前文件流对象。
	 * @details 如果文件流尚未关闭，则先保存并关闭。
	 *          `remove=true` 时，将当前对象从 `LogData::OutFilelist` 中移除。
	 * @param remove 是否从 `LogData::OutFilelist` 中移除当前对象，默认为 `true`。
	 * @code
	 * out.Outfinish();       // 关闭并从列表移除
	 * out.Outfinish(false);  // 仅关闭文件流，不移除列表项
	 * @endcode
	 */
	void OutFile::Outfinish(bool remove)
	{
		if (_writer && _writer->is_open())
		{
			_writer->close();
		}

		// 从列表当中移除当前的文件流
		if (remove)
		{
			// auto& fileList = LogData::OutFilelist;
			// fileList.erase(
			//	std::remove_if(fileList.begin(), fileList.end(),
			//		[this](OutFile& file) {
			//			return this->filename_ == file.GetFilename();
			//		}),
			//	fileList.end()
			//);
			for (size_t i = 0; i < LogData::OutFilelist.size(); i++)
			{
				if (this->GetFilename() == LogData::OutFilelist[i]->GetFilename())
				{
					LogData::OutFilelist.erase(LogData::OutFilelist.begin() + i);
					break;
				}
			}
		}
	}
}