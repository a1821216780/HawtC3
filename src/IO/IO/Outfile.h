//**********************************************************************************************************************************
//LICENSING
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


#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <Eigen/Dense>
#include "../Interface/IOutFile.h"
#include "../Log/LogHelper.h"
#include "OtherHelper.h"

using namespace Qahse::IO::Interface;

namespace Qahse::IO::IO
{
	/**
	 * @brief 表示用于将数据写入流的输出文件，支持各种格式化选项。
	 *
	 * OutFile 类提供将数据写入文件或流的功能，支持数值格式化选项，
	 * 包括科学计数法和固定小数位数。它支持写入单个值、向量和矩阵，
	 * 并包含管理输出流生命周期的方法。
	 *
	 * 主要特性：
	 * - 支持科学计数法和固定小数位数格式化
	 * - 可写入字符串、数值、向量和矩阵数据
	 * - 自动管理文件扩展名
	 * - 集成日志系统防止文件名冲突
	 * - 支持Eigen矩阵格式输出
	 */
	class OutFile : public IOutFile
	{
	private:
		std::unique_ptr<std::ofstream> _writer;
		std::string filename_;

		std::string filemat_;

		int decimalPlaces_;
		int scientificDigits_;
		bool useScientific_;

		inline void initializeFormat(int decimalPlaces, int Scientific);
		inline std::string  formatDouble(double value) const;
		inline std::string GetFileExtension(const std::string& path) const;
		inline void checkFileNameConflict() const;

	public:
		/**
		 * @brief 获取或设置与当前操作关联的流名称
		 */
		std::string StreamName;

		/**
		 * @brief 获取或设置流的文件路径
		 */
		std::string StreanFilePath;

		/**
		 * @brief 获取与当前实例关联的文件名
		 */
		std::string GetFilename() override;

		std::string  GetStreamFilePath() override;

		void SetStreamFilePath(const std::string& path) override;
		/**
		 * @brief 构造函数
		 * @param path 将写入输出的文件路径。如果缺少文件扩展名，则添加默认扩展名".3zout"
		 * @param decimalPlaces 用于格式化数值输出的小数位数。如果指定了Scientific，则忽略此参数
		 * @param Scientific 用于科学计数法格式化的有效数字位数。如果设置为-1，则不使用科学计数法
		 * @throws std::runtime_error 如果内部写入器初始化失败
		 */
		explicit OutFile(const std::string& path, int decimalPlaces = 6, int Scientific = -1);

		OutFile(const OutFile&) = delete;

		virtual ~OutFile();

		// IOutFile接口实现
		void Write(const std::string& message, const std::string& fg = "\t") override;
		void Write(double message, const std::string& fg = "\t") override;
		void WriteLine() override;
		void WriteLine(double message) override;
		void WriteLine(const std::string& message) override;
		void Outfinish(bool remove = true) override;

		/**
		 * @brief 将Eigen向量写入输出，元素之间用制表符分隔
		 * @param message 包含要写入元素的向量
		 * @param fg 元素之间使用的分隔符字符串。默认为制表符
		 */
		void Write(const Eigen::VectorXd& message, const std::string& fg = "\t");
		void Write(const Eigen::VectorXf& message, const std::string& fg = "\t");

		/**
		 * @brief 将Eigen矩阵写入输出，每行之间用换行符分隔
		 * @param message 要写入其行的矩阵
		 * @param fg 用于分隔每行内元素的分隔符。默认为制表符
		 */
		void Write(const Eigen::MatrixXd& message, const std::string& fg = "\t");
		void Write(const Eigen::MatrixXf& message, const std::string& fg = "\t");

		/**
		 * @brief 批量写入多个字符串消息
		 * @param fg 应用于每个消息的格式化字符串
		 * @param messages 要写入的消息向量
		 */
		void Write(const std::string& fg, const std::vector<std::string>& messages);

		/**
		 * @brief 批量写入多个数值
		 * @param fg 用于格式化每个值的前景字符串
		 * @param messages 要写入输出的double值向量
		 */
		void Write(const std::string& fg, const std::vector<double>& messages);

		/**
		 * @brief 写入格式化的标题和其他信息
		 * @param title 要写入的标题
		 */
		void WriteTitle(const std::string& title);

		/**
		 * @brief 将Eigen向量写入输出并换行
		 * @param message 包含要写入数据的向量
		 * @param decimalPlaces 格式化输出的小数位数。默认为10
		 */
		void WriteLine(const Eigen::VectorXd& message, int decimalPlaces = 10);
		void WriteLine(const Eigen::VectorXf& message, int decimalPlaces = 10);

		/**
		 * @brief 将Eigen矩阵写入输出并换行
		 * @param message 要写入的矩阵
		 */
		void WriteLine(const Eigen::MatrixXd& message);
		void WriteLine(const Eigen::MatrixXf& message);

		/**
		 * @brief 写入字符串数组，每个字符串后跟换行符
		 * @param messages 要写入的字符串向量
		 */
		void WriteLine(const std::vector<std::string>& messages, bool row = false);

		/**
		 * @brief 写入数值数组并换行
		 * @param messages 要处理和写入的double数组
		 */
		void WriteLine(const std::vector<double>& messages);

		/**
		 * @brief 写入二维数组并换行
		 * @param message 要处理并写入输出的二维double数组
		 */
		void WriteLine(const std::vector<std::vector<double>>& message);
	};

}
