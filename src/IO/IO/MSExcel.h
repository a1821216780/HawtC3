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

#ifndef MSEXCEL_H
#define MSEXCEL_H
#include <OpenXLSX.hpp>
#include <Eigen/Dense>
#include <string>
#include <vector>

namespace Qahse::IO::IO
{
    class MSExcel
    {
    public:
        // Constructor & Destructor
        explicit MSExcel(const std::string &path, const std::string &mode);
        ~MSExcel();

        // Prevent copying
        MSExcel(const MSExcel &) = delete;
        MSExcel &operator=(const MSExcel &) = delete;

        // Sheet operations
        void AddSheet(const std::string &name);
        bool SheetExist(const std::string &name);
        OpenXLSX::XLWorksheet GetSheet(const std::string &name);

        // Sheet information methods
        int RowCount(const std::string &sheetname);
        int RowCount(const OpenXLSX::XLWorksheet &worksheet);
        int ColumnCount(const std::string &sheetname);
        int ColumnCount(const OpenXLSX::XLWorksheet &worksheet);

        // Close Excel file
        void Close();

        // Template declarations for cell operations
        template <typename T>
        void WCellValue(const std::string &sheetname, const T &value, int row, int column, bool columnwr = false);

        template <typename T>
        T RCellValue(const std::string &sheetname, int row, int column, int rowcount = 0, int columncount = 0);

    private:
        std::string path;
        OpenXLSX::XLWorkbook workbook;
        OpenXLSX::XLDocument doc;
    };

    // Template specialization declarations
    template <>
    void MSExcel::WCellValue<double>(const std::string &sheetname, const double &value, int row, int column, bool columnwr);
    template <>
    void MSExcel::WCellValue<int>(const std::string &sheetname, const int &value, int row, int column, bool columnwr);
    template <>
    void MSExcel::WCellValue<std::string>(const std::string &sheetname, const std::string &value, int row, int column, bool columnwr);
    template <>
    void MSExcel::WCellValue<bool>(const std::string &sheetname, const bool &value, int row, int column, bool columnwr);
    template <>
    void MSExcel::WCellValue<std::vector<double>>(const std::string &sheetname, const std::vector<double> &value, int row, int column, bool columnwr);
    template <>
    void MSExcel::WCellValue<Eigen::VectorXd>(const std::string &sheetname, const Eigen::VectorXd &value, int row, int column, bool columnwr);
    template <>
    void MSExcel::WCellValue<Eigen::MatrixXd>(const std::string &sheetname, const Eigen::MatrixXd &value, int row, int column, bool columnwr);

    template <>
    double MSExcel::RCellValue<double>(const std::string &sheetname, int row, int column, int rowcount, int columncount);
    template <>
    int MSExcel::RCellValue<int>(const std::string &sheetname, int row, int column, int rowcount, int columncount);
    template <>
    std::string MSExcel::RCellValue<std::string>(const std::string &sheetname, int row, int column, int rowcount, int columncount);
    template <>
    bool MSExcel::RCellValue<bool>(const std::string &sheetname, int row, int column, int rowcount, int columncount);
    template <>
    std::vector<double> MSExcel::RCellValue<std::vector<double>>(const std::string &sheetname, int row, int column, int rowcount, int columncount);
    template <>
    Eigen::VectorXd MSExcel::RCellValue<Eigen::VectorXd>(const std::string &sheetname, int row, int column, int rowcount, int columncount);
    template <>
    Eigen::MatrixXd MSExcel::RCellValue<Eigen::MatrixXd>(const std::string &sheetname, int row, int column, int rowcount, int columncount);
}
#endif