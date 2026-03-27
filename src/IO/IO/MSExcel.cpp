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

#include <iostream>
#include <numeric>
#include <random>
#include <Eigen/Dense>
#include <string>
#include <vector>
#include <memory>
#include <OpenXLSX.hpp>

#include "../Math/LinearAlgebraHelper.h"
#include "../Log/LogHelper.h"
#include "OtherHelper.h"
#include "Extensions.h"
#include "MSExcel.h"
#include "StringHelper.h"

using namespace std;
using namespace OpenXLSX;
using namespace Qahse::IO::Log;
using namespace Qahse::IO::IO;
namespace Qahse::IO::IO
{
	// class MSExcel
	//{
	// public:
	//	string path;
	//	OpenXLSX::XLWorkbook workbook;
	//	XLDocument doc;
	/**
	 * @brief 构造并初始化 MSExcel 对象，按指定模式打开或创建 Excel 文件。
	 * @details 支持以下模式（大小写不敏感）：
	 *  - \"read\" / \"r\"：以只读方式打开现有文件。若文件不存在，则记录错误日志。
	 *  - \"write\" / \"w\"：创建新文件（若已存在则强制覆盖）。
	 *  - \"append\" / \"a\"：以增量方式打开现有文件。若文件不存在，则先创建再打开。
	 * @param path 目标 Excel 文件的路径（支持 `.xlsx`）。
	 * @param mode 模式字符串，可选值为 \"read\"/\"r\"\u3001\"write\"/\"w\"\u3001\"append\"/\"a\"。
	 * @code
	 * MSExcel excel("output.xlsx", "w"); // 创建新文件
	 * MSExcel excel2("data.xlsx",  "r"); // 打开现有文件
	 * @endcode
	 */
	MSExcel::MSExcel(const std::string &path, const std::string &mode)
	{
		try
		{
			this->path = path; ///< 存储目标文件路径
			// if(OtherHelper)
			// this->doc.r
			// workbook = doc.workbook();
			std::string lowerMode = mode; ///< 转小写后的模式字符串，用于大小写不敏感比较
			std::transform(lowerMode.begin(), lowerMode.end(), lowerMode.begin(), ::tolower);

			if (lowerMode == "read" || lowerMode == "r") // 只读模式
			{
				if (OtherHelper::FileExists(Extensions::GetABSPath(path)))
				{
					this->doc.open(path);
					workbook = doc.workbook();
				}
				else
				{
					LogHelper::ErrorLog("MSExcel: Cannot find file " + path);
				}
			}
			else if (lowerMode == "write" || lowerMode == "w") // 写入模式，强制覆盖现有文件
			{
				this->doc.create(path, XLForceOverwrite);
				workbook = doc.workbook();
			}
			else if (lowerMode == "append" || lowerMode == "a") // 增量模式
			{
				if (OtherHelper::FileExists(Extensions::GetABSPath(path)))
				{
					this->doc.open(path);
					workbook = doc.workbook();
				}
				else
				{
					LogHelper::ErrorLog("当前文件不存在！无法执行增量数据操作！");
					this->doc.create(path, XLForceOverwrite);
					workbook = doc.workbook();
				}
			}
			else
			{
				LogHelper::ErrorLog("IO.BinaryFile Cant find mode=" + mode, "", "", 20, "MSExcel::MSExcel");
			}
		}
		catch (const std::exception &ex)
		{
			LogHelper::ErrorLog("MSExcel \n" + std::string(ex.what()));
		}
	}

	/**
	 * @brief 析构函数，自动保存并关闭 Excel 文件。
	 */
	MSExcel::~MSExcel()
	{
		try
		{
			// 保存并关闭Excel文件
			this->Close();
		}

		catch (const std::exception &ex)
		{
			LogHelper::ErrorLog("MSExcel destructor: " + std::string(ex.what()));
		}
	}

	/**
	 * @brief 如果指定名称的工作表不存在，则在工作簿中新建工作表。
	 * @param name 要新建的工作表名称。
	 * @code
	 * excel.AddSheet("Sheet2"); // 添加名为 Sheet2 的工作表
	 * @endcode
	 */
	void MSExcel::AddSheet(const std::string &name)
	{
		if (!workbook.sheetExists(name))
		{
			workbook.addWorksheet(name);
		}
	}

	/**
	 * @brief 获取指定名称的工作表。若不存在则自动创建。
	 * @param name 工作表名称。
	 * @return XLWorksheet 对象，可用于单元格读写。
	 * @code
	 * auto ws = excel.GetSheet("Sheet1");
	 * ws.cell(1, 1).value() = "Header";
	 * @endcode
	 */
	OpenXLSX::XLWorksheet MSExcel::GetSheet(const std::string &name)
	{
		if (!workbook.sheetExists(name))
		{
			AddSheet(name);
		}

		return workbook.worksheet(name);
	}
	/**
	 * @brief 检查指定名称的工作表是否存在。
	 * @param name 工作表名称。
	 * @return 工作表存在返回 true，否则返回 false。
	 */
	bool MSExcel::SheetExist(const std::string &name)
	{
		return workbook.sheetExists(name);
	}

	/**
	 * @brief 获取指定名称工作表的行数。
	 * @param sheetname 工作表名称。
	 * @return 行数整数；工作表不存在时返回 0。
	 */
	int MSExcel::RowCount(const std::string &sheetname)
	{
		if (!workbook.sheetExists(sheetname))
		{
			LogHelper::ErrorLog("MSExcel: Cannot find worksheet " + sheetname);
			return 0;
		}
		return workbook.worksheet(sheetname).rowCount();
	}

	/**
	 * @brief 获取工作表对象的行数。
	 * @param worksheet XLWorksheet 对象。
	 * @return 行数整数。
	 */
	int MSExcel::RowCount(const OpenXLSX::XLWorksheet &worksheet)
	{
		return worksheet.rowCount();
	}

	/**
	 * @brief 获取指定名称工作表的列数。
	 * @param sheetname 工作表名称。
	 * @return 列数整数；工作表不存在时返回 0。
	 */
	int MSExcel::ColumnCount(const std::string &sheetname)
	{
		if (!workbook.sheetExists(sheetname))
		{
			LogHelper::ErrorLog("MSExcel: Cannot find worksheet " + sheetname);
			return 0;
		}
		return workbook.worksheet(sheetname).columnCount();
	}

	/**
	 * @brief 获取工作表对象的列数。
	 * @param worksheet XLWorksheet 对象。
	 * @return 列数整数。
	 */
	int MSExcel::ColumnCount(const OpenXLSX::XLWorksheet &worksheet)
	{
		return worksheet.columnCount();
	}

	/**
	 * @brief 保存并关闭 Excel 文件。
	 * @details 如果文件当前已打开，则先执行保存再关闭。
	 * @code
	 * excel.Close(); // 保存并关闭
	 * @endcode
	 */
	void MSExcel::Close()
	{
		if (doc.isOpen())
		{
			doc.save();
			doc.close();
		}
	}

	// WCellValue 模板特化：将各类型的数据写入指定工作表的单元格
	/**
	 * @brief 将 double 数标写入指定工作表的单元格。
	 * @param sheetname 目标工作表名称。
	 * @param value     要写入的 double 数值。
	 * @param row       目标单元格的行索引（0 基索引）。
	 * @param column    目标单元格的列索引（0 基索引）。
	 * @param columnwr  向量写入旹向（标量无意义）。
	 * @code
	 * excel.WCellValue<double>("Sheet1", 3.14, 0, 0);
	 * @endcode
	 */
	template <>
	void MSExcel::WCellValue<double>(const std::string &sheetname, const double &value, int row, int column, bool columnwr)
	{
		auto worksheet = GetSheet(sheetname);
		worksheet.cell(row + 1, column + 1).value() = value;
	}

	/**
	 * @brief 将整数标尺写入指定工作表的单元格。
	 * @param sheetname 目标工作表名称。
	 * @param value     要写入的 int 整数。
	 * @param row       行索引（0 基）。
	 * @param column    列索引（0 基）。
	 * @param columnwr  向量写入方向（标量无意义）。
	 */
	template <>
	void MSExcel::WCellValue<int>(const std::string &sheetname, const int &value, int row, int column, bool columnwr)
	{
		auto worksheet = GetSheet(sheetname);
		worksheet.cell(row + 1, column + 1).value() = value;
	}

	/**
	 * @brief 将字符串写入指定工作表的单元格。
	 * @param sheetname 目标工作表名称。
	 * @param value     要写入的字符串。
	 * @param row       行索引（0 基）。
	 * @param column    列索引（0 基）。
	 * @param columnwr  向量写入方向（标量无意义）。
	 */
	template <>
	void MSExcel::WCellValue<std::string>(const std::string &sheetname, const std::string &value, int row, int column, bool columnwr)
	{
		auto worksheet = GetSheet(sheetname);
		worksheet.cell(row + 1, column + 1).value() = value;
	}

	/**
	 * @brief 将布尔尺写入指定工作表的单元格。
	 * @param sheetname 目标工作表名称。
	 * @param value     要写入的 bool 尺。
	 * @param row       行索引（0 基）。
	 * @param column    列索引（0 基）。
	 * @param columnwr  向量写入方向（标量无意义）。
	 */
	template <>
	void MSExcel::WCellValue<bool>(const std::string &sheetname, const bool &value, int row, int column, bool columnwr)
	{
		auto worksheet = GetSheet(sheetname);
		worksheet.cell(row + 1, column + 1).value() = value;
	}

	/**
	 * @brief 将 `std::vector<double>` 向量写入工作表。
	 * @param sheetname 目标工作表名称。
	 * @param value     要写入的 double 向量。
	 * @param row       起始行索引（0 基）。
	 * @param column    起始列索引（0 基）。
	 * @param columnwr  true 表示按列写入（处于同一列逐行向下）；false 表示按行写入（处于同一行逐列向右）。
	 * @code
	 * std::vector<double> v = {1.0, 2.0, 3.0};
	 * excel.WCellValue<std::vector<double>>("Sheet1", v, 0, 0, true); // 写入 A1:A3 列
	 * @endcode
	 */
	template <>
	void MSExcel::WCellValue<std::vector<double>>(const std::string &sheetname, const std::vector<double> &value,
												  int row, int column, bool columnwr)
	{
		auto worksheet = GetSheet(sheetname);
		for (size_t i = 0; i < value.size(); ++i)
		{
			if (columnwr)
			{
				worksheet.cell(row + i + 1, column + 1).value() = value[i];
			}
			else
			{
				worksheet.cell(row + 1, column + i + 1).value() = value[i];
			}
		}
	}

	/**
	 * @brief 将 Eigen::VectorXd 向量写入工作表。
	 * @param sheetname 目标工作表名称。
	 * @param value     要写入的 Eigen::VectorXd 向量。
	 * @param row       起始行索引（0 基）。
	 * @param column    起始列索引（0 基）。
	 * @param columnwr  true 按列写入，false 按行写入。
	 */
	template <>
	void MSExcel::WCellValue<Eigen::VectorXd>(const std::string &sheetname, const Eigen::VectorXd &value,
											  int row, int column, bool columnwr)
	{
		auto worksheet = GetSheet(sheetname);
		for (Eigen::Index i = 0; i < value.size(); ++i)
		{
			if (columnwr)
			{
				worksheet.cell(row + i + 1, column + 1).value() = value(i);
			}
			else
			{
				worksheet.cell(row + 1, column + i + 1).value() = value(i);
			}
		}
	}

	/**
	 * @brief 将 Eigen::MatrixXd 矩阵写入工作表（行优先）。
	 * @param sheetname 目标工作表名称。
	 * @param value     要写入的 Eigen::MatrixXd 矩阵。
	 * @param row       工作表起始行索引（0 基）。
	 * @param column    工作表起始列索引（0 基）。
	 * @code
	 * Eigen::MatrixXd m(2, 3);
	 * m << 1,2,3,4,5,6;
	 * excel.WCellValue<Eigen::MatrixXd>("Sheet1", m, 0, 0);
	 * @endcode
	 */
	template <>
	void MSExcel::WCellValue<Eigen::MatrixXd>(const std::string &sheetname, const Eigen::MatrixXd &value,
											  int row, int column, bool /*columnwr*/)
	{
		auto worksheet = GetSheet(sheetname);
		for (Eigen::Index i = 0; i < value.rows(); ++i)
		{
			for (Eigen::Index j = 0; j < value.cols(); ++j)
			{
				worksheet.cell(row + i + 1, column + j + 1).value() = value(i, j);
			}
		}
	}

	// RCellValue 模板特化：从指定工作表单元格读取各类型数据
	/**
	 * @brief 从工作表指定单元格读取 double 标尺值。
	 * @param sheetname   目标工作表名称。
	 * @param row         行索引（0 基）。
	 * @param column      列索引（0 基）。
	 * @param rowcount    自动读取的行数（标量无意义）。
	 * @param columncount 自动读取的列数（标量无意义）。
	 * @return 单元格中的 double 数值。
	 * @code
	 * double v = excel.RCellValue<double>("Sheet1", 0, 0);
	 * @endcode
	 */
	template <>
	double MSExcel::RCellValue<double>(const std::string &sheetname, int row, int column, int /*rowcount*/, int /*columncount*/)
	{
		auto worksheet = GetSheet(sheetname);
		return worksheet.cell(row + 1, column + 1).value();
	}

	/**
	 * @brief 从工作表指定单元格读取 int 整数。
	 * @param sheetname   目标工作表名称。
	 * @param row         行索引（0 基）。
	 * @param column      列索引（0 基）。
	 * @return 单元格中的 int 整数。
	 */
	template <>
	int MSExcel::RCellValue<int>(const std::string &sheetname, int row, int column, int /*rowcount*/, int /*columncount*/)
	{
		auto worksheet = GetSheet(sheetname);
		return worksheet.cell(row + 1, column + 1).value();
	}

	/**
	 * @brief 从工作表指定单元格读取字符串。
	 * @param sheetname   目标工作表名称。
	 * @param row         行索引（0 基）。
	 * @param column      列索引（0 基）。
	 * @return 单元格中的字符串内容。
	 */
	template <>
	std::string MSExcel::RCellValue<std::string>(const std::string &sheetname, int row, int column, int /*rowcount*/, int /*columncount*/)
	{
		auto worksheet = GetSheet(sheetname);
		return worksheet.cell(row + 1, column + 1).value();
	}

	/**
	 * @brief 从工作表指定单元格读取 bool 尺。
	 * @param sheetname   目标工作表名称。
	 * @param row         行索引（0 基）。
	 * @param column      列索引（0 基）。
	 * @return 单元格中的 bool 尺。
	 */
	template <>
	bool MSExcel::RCellValue<bool>(const std::string &sheetname, int row, int column, int /*rowcount*/, int /*columncount*/)
	{
		auto worksheet = GetSheet(sheetname);
		return worksheet.cell(row + 1, column + 1).value();
	}

	/**
	 * @brief 从工作表读取指定起始位置的 `std::vector<double>` 列向量。
	 * @param sheetname   目标工作表名称。
	 * @param row         起始行索引（0 基）。
	 * @param column      列索引（0 基）。
	 * @param rowcount    要读取的行数；传 0 表示读到空单元格为止（对无穷循环有要求）。
	 * @return 包含指定列中数据的 double 向量。
	 * @code
	 * auto col = excel.RCellValue<std::vector<double>>("Sheet1", 1, 0, 10);
	 * // 读取 A2:A11 共 10 行数据
	 * @endcode
	 */
	template <>
	std::vector<double> MSExcel::RCellValue<std::vector<double>>(const std::string &sheetname, int row, int column,
																 int rowcount, int /*columncount*/)
	{
		auto worksheet = GetSheet(sheetname); ///< 获得指定工作表对象
		std::vector<double> result;			  ///< 存储读取结果的 double 共 N 项向量

		if (rowcount == 0) // 未指定行数，读到空单元格为止
		{
			LogHelper::WriteLogO("No rowcount specified, reading until empty cell");
			int i = 1; ///< 行偏移，从 1 开始（即从x row+1 起读）
			while (true)
			{
				auto cell = worksheet.cell(row + i, column + 1); ///< 当前单元格对象

				result.push_back(std::stod(cell.value()));
				++i;
			}
		}
		else
		{
			result.reserve(rowcount); ///< 预分配内存，避免多次重分配
			for (int i = 0; i < rowcount; ++i)
			{
				result.push_back(worksheet.cell(row + i + 1, column + 1).value());
			}
		}

		return result;
	}

	/**
	 * @brief 从工作表读取指定起始位置的 `Eigen::VectorXd` 列向量。
	 * @param sheetname 目标工作表名称。
	 * @param row       起始行索引（0 基）。
	 * @param column    列索引（0 基）。
	 * @param rowcount  要读取的行数。
	 * @return 包含数据的 `Eigen::VectorXd` 对象。
	 */
	template <>
	Eigen::VectorXd MSExcel::RCellValue<Eigen::VectorXd>(const std::string &sheetname, int row, int column,
														 int rowcount, int /*columncount*/)
	{
		auto values = RCellValue<std::vector<double>>(sheetname, row, column, rowcount); ///< 先读取为 std::vector<double>
		Eigen::VectorXd result(values.size());											 ///< 构造对应大小的 Eigen 向量
		for (size_t i = 0; i < values.size(); ++i)
		{
			result(i) = values[i]; ///< 逐个拷贝数据
		}
		return result;
	}

	/**
	 * @brief 从工作表读取指定区域的 `Eigen::MatrixXd` 矩阵。
	 * @param sheetname   目标工作表名称。
	 * @param row         起始行索引（0 基）。
	 * @param column      起始列索引（0 基）。
	 * @param rowcount    要读取的行数；不能为 0。
	 * @param columncount 要读取的列数；不能为 0。
	 * @return 具有指定分割区域数据的 `Eigen::MatrixXd` 矩阵。
	 * @code
	 * auto m = excel.RCellValue<Eigen::MatrixXd>("Sheet1", 0, 0, 3, 4);
	 * // 读取 A1:D3 区域的 3x4 矩阵
	 * @endcode
	 */
	template <>
	Eigen::MatrixXd MSExcel::RCellValue<Eigen::MatrixXd>(const std::string &sheetname, int row, int column,
														 int rowcount, int columncount)
	{
		if (rowcount == 0 || columncount == 0)
		{
			LogHelper::ErrorLog("MSExcel: Must specify both rowcount and columncount for matrix reading");
			return Eigen::MatrixXd(); // 返回空矩阵
		}

		auto worksheet = GetSheet(sheetname);		   ///< 获得指定工作表对象
		Eigen::MatrixXd result(rowcount, columncount); ///< 预分配矩阵内存

		for (int i = 0; i < rowcount; ++i)
		{
			for (int j = 0; j < columncount; ++j)
			{
				result(i, j) = worksheet.cell(row + i + 1, column + j + 1).value(); ///< 需要加 1 因为 OpenXLSX 使用 1 基索引
			}
		}

		return result;
	}

	//	} // namespace IO
	//	;
	//}
}