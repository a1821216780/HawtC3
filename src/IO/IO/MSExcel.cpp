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
using namespace HawtC3::IO::Log;
using namespace HawtC3::IO::IO;
namespace HawtC3::IO::IO
{
	//class MSExcel
	//{
	//public:
	//	string path;
	//	OpenXLSX::XLWorkbook workbook;
	//	XLDocument doc;
	MSExcel::MSExcel(const std::string& path,const std::string& mode) {
		try
		{
			this->path = path;
			//if(OtherHelper)
			//this->doc.r
			//workbook = doc.workbook();
			std::string lowerMode = mode;
			std::transform(lowerMode.begin(), lowerMode.end(), lowerMode.begin(), ::tolower);

			if (lowerMode == "read" || lowerMode == "r")
			{
				if (OtherHelper::FileExists(Extensions::GetABSPath(path)))
				{
					this->doc.open(path);
					workbook = doc.workbook();
				}
				else {
					LogHelper::ErrorLog("MSExcel: Cannot find file " + path);
				}
			}
			else if (lowerMode == "write" || lowerMode == "w")
			{
				this->doc.create(path,XLForceOverwrite);
				workbook = doc.workbook();
			}
			else if (lowerMode == "append" || lowerMode == "a")
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
			else {
				LogHelper::ErrorLog("IO.BinaryFile Cant find mode=" + mode, "", "", 20, "MSExcel::MSExcel");
			}
		}
		catch (const std::exception& ex) {
			LogHelper::ErrorLog("MSExcel \n" + std::string(ex.what()));
		}
	}

	MSExcel::~MSExcel()
	{
		try {
			//保存并关闭Excel文件
			this->Close();
		}

		catch (const std::exception& ex) {
			LogHelper::ErrorLog("MSExcel destructor: " + std::string(ex.what()));
		}
	}

	void MSExcel::AddSheet(const std::string& name) {
		if (!workbook.sheetExists(name)) {
			workbook.addWorksheet(name);
		}
	}

	OpenXLSX::XLWorksheet MSExcel::GetSheet(const std::string& name) {
		if (!workbook.sheetExists(name))
		{
			AddSheet(name);
		}

		return workbook.worksheet(name);
	}
	bool MSExcel::SheetExist(const std::string& name)
	{
		return workbook.sheetExists(name);
	}

	int MSExcel::RowCount(const std::string& sheetname) {
		if (!workbook.sheetExists(sheetname))
		{
			LogHelper::ErrorLog("MSExcel: Cannot find worksheet " + sheetname);
			return 0;
		}
		return workbook.worksheet(sheetname).rowCount();
	}

	int MSExcel::RowCount(const OpenXLSX::XLWorksheet& worksheet) {
		return worksheet.rowCount();
	}

	int MSExcel::ColumnCount(const std::string& sheetname) {
		if (!workbook.sheetExists(sheetname)) {
			LogHelper::ErrorLog("MSExcel: Cannot find worksheet " + sheetname);
			return 0;
		}
		return workbook.worksheet(sheetname).columnCount();
	}

	int MSExcel::ColumnCount(const OpenXLSX::XLWorksheet& worksheet) {
		return worksheet.columnCount();
	}

	void MSExcel::Close() {
		if (doc.isOpen()) {
			doc.save();
			doc.close();
		}
	}



	// Template specializations for WCellValue
	template<>
	void MSExcel::WCellValue<double>(const std::string& sheetname, const double& value, int row, int column, bool columnwr) {
		auto worksheet = GetSheet(sheetname);
		worksheet.cell(row + 1, column + 1).value() = value;
	}

	template<>
	void MSExcel::WCellValue<int>(const std::string& sheetname, const int& value, int row, int column, bool columnwr) {
		auto worksheet = GetSheet(sheetname);
		worksheet.cell(row + 1, column + 1).value() = value;
	}

	template<>
	void MSExcel::WCellValue<std::string>(const std::string& sheetname, const std::string& value, int row, int column, bool columnwr) {
		auto worksheet = GetSheet(sheetname);
		worksheet.cell(row + 1, column + 1).value() = value;
	}

	template<>
	void MSExcel::WCellValue<bool>(const std::string& sheetname, const bool& value, int row, int column, bool columnwr) {
		auto worksheet = GetSheet(sheetname);
		worksheet.cell(row + 1, column + 1).value() = value;
	}

	template<>
	void MSExcel::WCellValue<std::vector<double>>(const std::string& sheetname, const std::vector<double>& value,
		int row, int column, bool columnwr) {
		auto worksheet = GetSheet(sheetname);
		for (size_t i = 0; i < value.size(); ++i) {
			if (columnwr) {
				worksheet.cell(row + i + 1, column + 1).value() = value[i];
			}
			else {
				worksheet.cell(row + 1, column + i + 1).value() = value[i];
			}
		}
	}

	template<>
	void MSExcel::WCellValue<Eigen::VectorXd>(const std::string& sheetname, const Eigen::VectorXd& value,
		int row, int column, bool columnwr) {
		auto worksheet = GetSheet(sheetname);
		for (Eigen::Index i = 0; i < value.size(); ++i) {
			if (columnwr) {
				worksheet.cell(row + i + 1, column + 1).value() = value(i);
			}
			else {
				worksheet.cell(row + 1, column + i + 1).value() = value(i);
			}
		}
	}

	template<>
	void MSExcel::WCellValue<Eigen::MatrixXd>(const std::string& sheetname, const Eigen::MatrixXd& value,
		int row, int column, bool /*columnwr*/) {
		auto worksheet = GetSheet(sheetname);
		for (Eigen::Index i = 0; i < value.rows(); ++i) {
			for (Eigen::Index j = 0; j < value.cols(); ++j) {
				worksheet.cell(row + i + 1, column + j + 1).value() = value(i, j);
			}
		}
	}

	// Template specializations for RCellValue
	template<>
	double MSExcel::RCellValue<double>(const std::string& sheetname, int row, int column, int /*rowcount*/, int /*columncount*/) {
		auto worksheet = GetSheet(sheetname);
		return worksheet.cell(row + 1, column + 1).value();
	}

	template<>
	int MSExcel::RCellValue<int>(const std::string& sheetname, int row, int column, int /*rowcount*/, int /*columncount*/) {
		auto worksheet = GetSheet(sheetname);
		return worksheet.cell(row + 1, column + 1).value();
	}

	template<>
	std::string MSExcel::RCellValue<std::string>(const std::string& sheetname, int row, int column, int /*rowcount*/, int /*columncount*/) {
		auto worksheet = GetSheet(sheetname);
		return worksheet.cell(row + 1, column + 1).value();
	}

	template<>
	bool MSExcel::RCellValue<bool>(const std::string& sheetname, int row, int column, int /*rowcount*/, int /*columncount*/) {
		auto worksheet = GetSheet(sheetname);
		return worksheet.cell(row + 1, column + 1).value();
	}

	template<>
	std::vector<double> MSExcel::RCellValue<std::vector<double>>(const std::string& sheetname, int row, int column,
		int rowcount, int /*columncount*/) {
		auto worksheet = GetSheet(sheetname);
		std::vector<double> result;

		if (rowcount == 0) {
			LogHelper::WriteLogO("No rowcount specified, reading until empty cell");
			int i = 1;
			while (true) {
				auto cell = worksheet.cell(row + i, column + 1);

				result.push_back(std::stod(cell.value()));
				++i;

			}
		}
		else {
			result.reserve(rowcount);
			for (int i = 0; i < rowcount; ++i) {
				result.push_back(worksheet.cell(row + i + 1, column + 1).value());
			}
		}

		return result;
	}

	template<>
	Eigen::VectorXd MSExcel::RCellValue<Eigen::VectorXd>(const std::string& sheetname, int row, int column,
		int rowcount, int /*columncount*/) {
		auto values = RCellValue<std::vector<double>>(sheetname, row, column, rowcount);
		Eigen::VectorXd result(values.size());
		for (size_t i = 0; i < values.size(); ++i) {
			result(i) = values[i];
		}
		return result;
	}

	template<>
	Eigen::MatrixXd MSExcel::RCellValue<Eigen::MatrixXd>(const std::string& sheetname, int row, int column,
		int rowcount, int columncount) {
		if (rowcount == 0 || columncount == 0) {
			LogHelper::ErrorLog("MSExcel: Must specify both rowcount and columncount for matrix reading");
			return Eigen::MatrixXd();
		}

		auto worksheet = GetSheet(sheetname);
		Eigen::MatrixXd result(rowcount, columncount);

		for (int i = 0; i < rowcount; ++i) {
			for (int j = 0; j < columncount; ++j) {
				result(i, j) = worksheet.cell(row + i + 1, column + j + 1).value();
			}
		}

		return result;
	}



	//	} // namespace IO
	//	; 
	//}
}