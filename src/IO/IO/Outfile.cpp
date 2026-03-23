//**********************************************************************************************************************************
//LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of HawtC3.IO.Log.
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


using namespace HawtC3::IO::IO;
using namespace HawtC3::IO::Log;

OutFile::OutFile(const std::string& path, int decimalPlaces, int Scientific)
{

	// 处理文件扩展名
	std::string finalPath =Extensions::GetABSPath(path) ;
	if (GetFileExtension(path).empty()) {
		finalPath = path + ".3zout";
	}

	filename_ = finalPath;
	StreanFilePath = finalPath;
	StreamName = std::filesystem::path(finalPath).filename().string();

	// 初始化格式化字符串
	initializeFormat(decimalPlaces, Scientific);

	// 检查文件名冲突
	checkFileNameConflict();

	// 创建文件流
	try {

		_writer = std::make_unique<std::ofstream>(finalPath);
		if (!_writer->is_open() || !_writer->good()) {
			throw std::runtime_error("无法打开文件: " + finalPath);
		}
	}
	catch (const std::exception& e) {
		LogHelper::ErrorLog("当前路径 " + finalPath + " 被其他程序占用，或没有该文件夹无法继续！","","",20,"OutFile::OutFile");
		throw;
	}

	if (!_writer) {
		throw std::runtime_error("读写器没有初始化");
	}

	// 添加到全局文件列表
	LogData::OutFilelist.push_back(this);
}

std::string OutFile::GetFilename()
{
	return filename_;
}

std::string OutFile::GetStreamFilePath()
{
	return StreanFilePath;
}

void OutFile::SetStreamFilePath(const std::string& path)
{
	StreanFilePath = path;
}

OutFile::~OutFile() {
	if (_writer && _writer->is_open()) {
		Outfinish();
	}
}

void OutFile::initializeFormat(int decimalPlaces, int Scientific) {
	if (Scientific != -1)
	{ // 科学计数法
		useScientific_ = true;
		decimalPlaces_ = decimalPlaces;
		scientificDigits_ = Scientific;

		// 构建格式字符串
		filemat_ = "{:." + std::to_string(scientificDigits_) + "E}";
	}
	else if (decimalPlaces != -1) { // 有限小数计数法
		useScientific_ = false;
		decimalPlaces_ = decimalPlaces;
		scientificDigits_ = Scientific;

		filemat_ = "{:." + std::to_string(decimalPlaces_) + "f}";

	}
	else
	{	
		LogHelper::ErrorLog("OutFile 不允许不指定小数位数！","","",20,"initializeFormat");
		throw std::invalid_argument("必须指定 decimalPlaces 或 Scientific 参数之一");
	}
}

inline std::string OutFile::formatDouble(double value) const
{
	std::string res=fmt::format(filemat_, value);
	return res;
}

std::string OutFile::GetFileExtension(const std::string& path) const {
	size_t dotPos = path.find_last_of('.');
	if (dotPos != std::string::npos && dotPos < path.length() - 1) {
		return path.substr(dotPos);
	}
	return "";
}

void OutFile::checkFileNameConflict() const {
	for (const auto& existingFile : LogData::OutFilelist) {
		if (this->filename_ == existingFile->GetFilename()) {
			LogHelper::ErrorLog("当前文件名称 " + std::filesystem::path(filename_).filename().string() +
				" 与 LogData.OutFilelist 当中的文件路径和名称重合，这是不允许的！，这个错误只会发生在调试模式");
		}
	}
}

void OutFile::Write(const std::string& message, const std::string& fg) {
	*_writer << message << fg;
}

void OutFile::Write(double message, const std::string& fg) {
	*_writer << formatDouble(message) << fg;
}

void OutFile::Write(const Eigen::VectorXd& message, const std::string& fg) {
	for (int i = 0; i < message.size(); i++) {
		*_writer << formatDouble(message[i]) << fg;
	}
}

void OutFile::Write(const Eigen::VectorXf& message, const std::string& fg) {
	for (int i = 0; i < message.size(); i++) {
		*_writer << formatDouble(static_cast<double>(message[i])) << fg;
	}
}

void OutFile::Write(const Eigen::MatrixXd& message, const std::string& fg) {
	for (int i = 0; i < message.rows(); i++) {
		for (int j = 0; j < message.cols(); j++) {
			*_writer << formatDouble(message(i, j)) << fg;
		}
		WriteLine();
	}
}

void OutFile::Write(const Eigen::MatrixXf& message, const std::string& fg) {
	for (int i = 0; i < message.rows(); i++) {
		for (int j = 0; j < message.cols(); j++) {
			*_writer << formatDouble(static_cast<double>(message(i, j))) << fg;
		}
		WriteLine();
	}
}

void OutFile::Write(const std::string& fg, const std::vector<std::string>& messages) {
	for (const auto& message : messages) {
		Write(message, fg);
	}
}

void OutFile::Write(const std::string& fg, const std::vector<double>& messages) {
	for (const auto& message : messages) {
		Write(message, fg);
	}
}

void OutFile::WriteLine() {
	*_writer << '\n';
}

void OutFile::WriteLine(double message) {
	*_writer << formatDouble(message) << '\n';
}

void OutFile::WriteLine(const std::string& message) {
	*_writer << message << '\n';
}

void OutFile::WriteTitle(const std::string& title) {
	*_writer << "HawtC2_ProjectName: " << title << " 更多消息和教程请访问：http://www.hawtc.cn/" << '\n';
	*_writer << "Powered By HawtC3.HawtC2." << OtherHelper::GetCurrentProjectName() << " @CopyRight 赵子祯" << '\n';
	// 获取当前时间
	auto now = std::chrono::system_clock::now();
	std::time_t time_t = std::chrono::system_clock::to_time_t(now);
	std::tm tm = OtherHelper::GetSafeLocalTime(time_t);
	*_writer << "计算时间：" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '\n';
}

void OutFile::WriteLine(const std::vector<std::string>& messages,bool row)
{
	if (row)
	{
		for (const auto& message : messages)
		{ 
			*_writer << message << '\t';
		}
		*_writer << '\n';
	}
	else
	{
		for (const auto& message : messages)
		{
			*_writer << message << '\n';
		}
	}

}

void OutFile::WriteLine(const Eigen::VectorXd& message, int decimalPlaces) {
	// 创建行矩阵并输出
	Eigen::MatrixXd rowMatrix = message.transpose();

	// 临时修改精度设置 
	std::ostringstream oss;
	if (useScientific_) {
		oss << std::scientific << std::setprecision(decimalPlaces);
	}
	else {
		oss << std::fixed << std::setprecision(decimalPlaces);
	}

	for (int j = 0; j < rowMatrix.cols(); j++) {
		oss << rowMatrix(0, j);
		if (j < rowMatrix.cols() - 1) {
			oss << "\t";
		}
	}

	*_writer << oss.str() << '\n';
}

void OutFile::WriteLine(const Eigen::VectorXf& message, int decimalPlaces) {
	// 转换为double向量并调用对应方法
	Eigen::VectorXd doubleVector = message.cast<double>();
	WriteLine(doubleVector, decimalPlaces);
}

void OutFile::WriteLine(const Eigen::MatrixXd& message) {
	for (int i = 0; i < message.rows(); i++) {
		for (int j = 0; j < message.cols(); j++) {
			*_writer << formatDouble(message(i, j));
			if (j < message.cols() - 1) {
				*_writer << "\t";
			}
		}
		*_writer << '\n';
	}
}

void OutFile::WriteLine(const Eigen::MatrixXf& message) {
	// 转换为double矩阵并调用对应方法
	Eigen::MatrixXd doubleMatrix = message.cast<double>();
	WriteLine(doubleMatrix);
}

void OutFile::WriteLine(const std::vector<double>& messages) {
	// 转换为Eigen向量并调用对应方法
	Eigen::VectorXd eigenVector(messages.size());
	for (size_t i = 0; i < messages.size(); i++) {
		eigenVector[i] = messages[i];
	}
	WriteLine(eigenVector);
}

void OutFile::WriteLine(const std::vector<std::vector<double>>& message) {
	// 转换为Eigen矩阵并调用对应方法
	if (message.empty() || message[0].empty()) {
		return;
	}

	int rows = message.size();
	int cols = message[0].size();
	Eigen::MatrixXd eigenMatrix(rows, cols);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			eigenMatrix(i, j) = message[i][j];
		}
	}

	WriteLine(eigenMatrix);
}

void OutFile::Outfinish(bool remove) {
	if (_writer && _writer->is_open()) {
		_writer->close();
	}

	// 从列表当中移除当前的文件流
	if (remove)
	{
		//auto& fileList = LogData::OutFilelist;
		//fileList.erase(
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
