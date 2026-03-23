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

#include "BinaryFileHandler.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <stdexcept>

using namespace HawtC3::IO::Log;
using namespace HawtC3::IO::Math;

namespace HawtC3 {
namespace IO {
namespace IO {

BinaryFile::BinaryFile(const std::string& filePath, const std::string& mode) 
{
    this->filePath = filePath;
    this->isReading = false;
    this->isWriting = false;
    try {
        std::string lowerMode = mode;
        std::transform(lowerMode.begin(), lowerMode.end(), lowerMode.begin(), ::tolower);
        
        if (lowerMode == "read" || lowerMode == "r") {
            reader = std::make_unique<std::ifstream>(filePath, std::ios::binary);
            if (!reader->good()) {
                LogHelper::ErrorLog("Failed to open file for reading: " + filePath,"","",20,"BinaryFile::BinaryFile");
            }
            isReading = true;
        }
        else if (lowerMode == "write" || lowerMode == "w") {
            writer = std::make_unique<std::ofstream>(filePath, std::ios::binary | std::ios::trunc);
            if (!writer->good()) {
                LogHelper::ErrorLog("Failed to open file for writing: " + filePath, "", "", 20, "BinaryFile::BinaryFile");
            }
            isWriting = true;
        }
        else if (lowerMode == "append" || lowerMode == "a") {
            writer = std::make_unique<std::ofstream>(filePath, std::ios::binary | std::ios::app);
            if (!writer->good()) {
                LogHelper::ErrorLog("Failed to open file for appending: " + filePath, "", "", 20, "BinaryFile::BinaryFile");
            }
            isWriting = true;
        }
        else {
            LogHelper::ErrorLog("IO.BinaryFile Cant find mode=" + mode,"","",20,"BinaryFile::BinaryFile");
            writer = std::make_unique<std::ofstream>(filePath, std::ios::binary | std::ios::app);
            if (writer->good()) {
                isWriting = true;
            }
        }
    }
    catch (const std::exception& ex) {
        LogHelper::ErrorLog("BinaryFile \n" + std::string(ex.what()), "", "", 20, "BinaryFile::BinaryFile");
    }
}

BinaryFile::~BinaryFile() {
    Close();
}

void BinaryFile::Close() {
    if (writer && writer->is_open()) {
        writer->close();
        writer.reset();
        isWriting = false;
    }
    if (reader && reader->is_open()) {
        reader->close();
        reader.reset();
        isReading = false;
    }
}

// 基本类型写入特化
template<>
void BinaryFile::WriteData<short>(const short& data) {
    WriteBasicType(data);
}

template<>
void BinaryFile::WriteData<int>(const int& data) {
    WriteBasicType(data);
}


template<>
void BinaryFile::WriteData<std::int64_t>(const std::int64_t& data) {
    WriteBasicType(data);
}

template<>
void BinaryFile::WriteData<float>(const float& data) {
    WriteBasicType(data);
}

template<>
void BinaryFile::WriteData<double>(const double& data) {
    WriteBasicType(data);
}

template<>
void BinaryFile::WriteData<char>(const char& data) {
    WriteBasicType(data);
}

template<>
void BinaryFile::WriteData<bool>(const bool& data) {
    int value = data ? 1 : 0;
    WriteBasicType(value);
}

template<>
void BinaryFile::WriteData<std::string>(const std::string& data) {
    WriteString(data);
}

// 数组类型写入特化
template<>
void BinaryFile::WriteData<std::vector<char>>(const std::vector<char>& data) {
    if (!isWriting || !writer) return;
    writer->write(reinterpret_cast<const char*>(data.data()), data.size());
}

template<>
void BinaryFile::WriteData<std::vector<std::string>>(const std::vector<std::string>& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.size()));
    for (const auto& str : data) {
        WriteString(str);
    }
}

template<>
void BinaryFile::WriteData<std::vector<float>>(const std::vector<float>& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.size()));
    for (const auto& value : data) {
        WriteBasicType(value);
    }
}

template<>
void BinaryFile::WriteData<std::vector<double>>(const std::vector<double>& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.size()));
    for (const auto& value : data) {
        WriteBasicType(value);
    }
}

template<>
void BinaryFile::WriteData<std::vector<int>>(const std::vector<int>& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.size()));
    for (const auto& value : data) {
        WriteBasicType(value);
    }
}

template<>
void BinaryFile::WriteData<std::vector<bool>>(const std::vector<bool>& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.size()));
    for (size_t i = 0; i < data.size(); ++i) {
        int value = data[i] ? 1 : 0;
        WriteBasicType(value);
    }
}

// 二维数组写入特化
template<>
void BinaryFile::WriteData<std::vector<std::vector<double>>>(const std::vector<std::vector<double>>& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.size()));
    WriteBasicType(static_cast<int>(data.empty() ? 0 : data[0].size()));
    for (const auto& row : data) {
        for (const auto& value : row) {
            WriteBasicType(value);
        }
    }
}

template<>
void BinaryFile::WriteData<std::vector<std::vector<float>>>(const std::vector<std::vector<float>>& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.size()));
    WriteBasicType(static_cast<int>(data.empty() ? 0 : data[0].size()));
    for (const auto& row : data) {
        for (const auto& value : row) {
            WriteBasicType(value);
        }
    }
}

template<>
void BinaryFile::WriteData<std::vector<std::vector<int>>>(const std::vector<std::vector<int>>& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.size()));
    WriteBasicType(static_cast<int>(data.empty() ? 0 : data[0].size()));
    for (const auto& row : data) {
        for (const auto& value : row) {
            WriteBasicType(value);
        }
    }
}

// Eigen矩阵和向量写入特化
template<>
void BinaryFile::WriteData<Eigen::MatrixXd>(const Eigen::MatrixXd& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.rows()));
    WriteBasicType(static_cast<int>(data.cols()));
    for (int i = 0; i < data.rows(); ++i) {
        for (int j = 0; j < data.cols(); ++j) {
            WriteBasicType(data(i, j));
        }
    }
}

template<>
void BinaryFile::WriteData<Eigen::MatrixXf>(const Eigen::MatrixXf& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.rows()));
    WriteBasicType(static_cast<int>(data.cols()));
    for (int i = 0; i < data.rows(); ++i) {
        for (int j = 0; j < data.cols(); ++j) {
            WriteBasicType(data(i, j));
        }
    }
}

template<>
void BinaryFile::WriteData<Eigen::VectorXd>(const Eigen::VectorXd& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.size()));
    for (int i = 0; i < data.size(); ++i) {
        WriteBasicType(data(i));
    }
}

template<>
void BinaryFile::WriteData<Eigen::VectorXf>(const Eigen::VectorXf& data) {
    if (!isWriting || !writer) return;
    WriteBasicType(static_cast<int>(data.size()));
    for (int i = 0; i < data.size(); ++i) {
        WriteBasicType(data(i));
    }
}

// 基本类型读取特化
template<>
short BinaryFile::ReadData<short>() {
    return ReadBasicType<short>();
}

template<>
int BinaryFile::ReadData<int>() {
    return ReadBasicType<int>();
}

template<>
std::int64_t BinaryFile::ReadData<std::int64_t>() {
    return ReadBasicType<std::int64_t>();
}

template<>
float BinaryFile::ReadData<float>() {
    return ReadBasicType<float>();
}

template<>
double BinaryFile::ReadData<double>() {
    return ReadBasicType<double>();
}

template<>
char BinaryFile::ReadData<char>() {
    return ReadBasicType<char>();
}

template<>
bool BinaryFile::ReadData<bool>() {
    int value = ReadBasicType<int>();
    return value == 1;
}

template<>
std::string BinaryFile::ReadData<std::string>() {
    return ReadString();
}

// 数组类型读取特化
template<>
std::vector<char> BinaryFile::ReadData<std::vector<char>>() {
    if (!isReading || !reader) return {};
    int length = ReadBasicType<int>();
    std::vector<char> result(length);
    reader->read(result.data(), length);
    return result;
}

template<>
std::vector<std::string> BinaryFile::ReadData<std::vector<std::string>>() {
    if (!isReading || !reader) return {};
    int length = ReadBasicType<int>();
    std::vector<std::string> result(length);
    for (int i = 0; i < length; ++i) {
        result[i] = ReadString();
    }
    return result;
}

template<>
std::vector<float> BinaryFile::ReadData<std::vector<float>>() {
    if (!isReading || !reader) return {};
    int length = ReadBasicType<int>();
    std::vector<float> result(length);
    for (int i = 0; i < length; ++i) {
        result[i] = ReadBasicType<float>();
    }
    return result;
}

template<>
std::vector<double> BinaryFile::ReadData<std::vector<double>>() {
    if (!isReading || !reader) return {};
    int length = ReadBasicType<int>();
    std::vector<double> result(length);
    for (int i = 0; i < length; ++i) {
        result[i] = ReadBasicType<double>();
    }
    return result;
}

template<>
std::vector<int> BinaryFile::ReadData<std::vector<int>>() {
    if (!isReading || !reader) return {};
    int length = ReadBasicType<int>();
    std::vector<int> result(length);
    for (int i = 0; i < length; ++i) {
        result[i] = ReadBasicType<int>();
    }
    return result;
}

template<>
std::vector<bool> BinaryFile::ReadData<std::vector<bool>>() {
    if (!isReading || !reader) return {};
    int length = ReadBasicType<int>();
    std::vector<bool> result(length);
    for (int i = 0; i < length; ++i) {
        int value = ReadBasicType<int>();
        result[i] = (value == 1);
    }
    return result;
}

// 二维数组读取特化
template<>
std::vector<std::vector<double>> BinaryFile::ReadData<std::vector<std::vector<double>>>() {
    if (!isReading || !reader) return {};
    int rows = ReadBasicType<int>();
    int cols = ReadBasicType<int>();
    std::vector<std::vector<double>> result(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i][j] = ReadBasicType<double>();
        }
    }
    return result;
}

template<>
std::vector<std::vector<float>> BinaryFile::ReadData<std::vector<std::vector<float>>>() {
    if (!isReading || !reader) return {};
    int rows = ReadBasicType<int>();
    int cols = ReadBasicType<int>();
    std::vector<std::vector<float>> result(rows, std::vector<float>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i][j] = ReadBasicType<float>();
        }
    }
    return result;
}

template<>
std::vector<std::vector<int>> BinaryFile::ReadData<std::vector<std::vector<int>>>() {
    if (!isReading || !reader) return {};
    int rows = ReadBasicType<int>();
    int cols = ReadBasicType<int>();
    std::vector<std::vector<int>> result(rows, std::vector<int>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i][j] = ReadBasicType<int>();
        }
    }
    return result;
}

// Eigen矩阵和向量读取特化
template<>
Eigen::MatrixXd BinaryFile::ReadData<Eigen::MatrixXd>() {
    if (!isReading || !reader) return {};
    int rows = ReadBasicType<int>();
    int cols = ReadBasicType<int>();
    Eigen::MatrixXd result = LinearAlgebraHelper::zeros(rows, cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result(i, j) = ReadBasicType<double>();
        }
    }
    return result;
}

template<>
Eigen::MatrixXf BinaryFile::ReadData<Eigen::MatrixXf>() {
    if (!isReading || !reader) return {};
    int rows = ReadBasicType<int>();
    int cols = ReadBasicType<int>();
    Eigen::MatrixXf result = LinearAlgebraHelper::fzeros(rows, cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result(i, j) = ReadBasicType<float>();
        }
    }
    return result;
}

template<>
Eigen::VectorXd BinaryFile::ReadData<Eigen::VectorXd>() {
    if (!isReading || !reader) return {};
    int size = ReadBasicType<int>();
    Eigen::VectorXd result = LinearAlgebraHelper::zeros(size);
    for (int i = 0; i < size; ++i) {
        result(i) = ReadBasicType<double>();
    }
    return result;
}

template<>
Eigen::VectorXf BinaryFile::ReadData<Eigen::VectorXf>() {
    if (!isReading || !reader) return {};
    int size = ReadBasicType<int>();
    Eigen::VectorXf result = LinearAlgebraHelper::fzeros(size);
    for (int i = 0; i < size; ++i) {
        result(i) = ReadBasicType<float>();
    }
    return result;
}


// WriteLine 方法实现
template<typename T>
void BinaryFile::WriteLine(const T& data) {
    WriteData(data);
}

void BinaryFile::WriteLine(const std::string& format, double message) {
    std::string formatted = FormatNumber(format, message);
    WriteData(formatted);
}

void BinaryFile::WriteLine(const std::string& format, float message) {
    std::string formatted = FormatNumber(format, message);
    WriteData(formatted);
}

void BinaryFile::WriteLine() {
    // C++中对于二进制文件，空行的概念需要根据具体需求定义
    // 这里暂时不实现具体内容
}

// Write 方法实现
template<typename T>
void BinaryFile::Write(const T& data) {
    WriteData(data);
}

void BinaryFile::Write(const std::string& format, double message) {
    std::string formatted = FormatNumber(format, message);
    WriteData(formatted);
}

void BinaryFile::Write(const std::string& format, float message) {
    std::string formatted = FormatNumber(format, message);
    WriteData(formatted);
}

bool BinaryFile::Read2last() {

    if (!isReading || !reader) return true;

    std::streampos currentPos = reader->tellg();
    int testValue = -858993460;

    try
    {
        testValue = ReadBasicType<int>();
       

    }
    catch (...)
    {

    }




	bool endfile = reader->eof();
    reader->seekg(currentPos); // 恢复位置
	return endfile;

        
}

// 私有辅助方法实现
template<typename T>
void BinaryFile::WriteBasicType(const T& data) {
    if (!isWriting || !writer) return;
    writer->write(reinterpret_cast<const char*>(&data), sizeof(T));
}

template<typename T>
T BinaryFile::ReadBasicType() {
    if (!isReading || !reader) {
        LogHelper::ErrorLog("HawtC3.IO Unsupported data type or stream not available", "", "", 20, "T BinaryFile::ReadBasicType");
        return T{};
    }
    T value;
    reader->read(reinterpret_cast<char*>(&value), sizeof(T));
    if (!reader->good())
    {
        LogHelper::WarnLog("Failed to read data from file ,it may end of file!");
    }
    return value;
}

void BinaryFile::WriteString(const std::string& str) {
    if (!isWriting || !writer) return;
    
    // 写入字符串长度
    std::uint32_t length = static_cast<std::uint32_t>(str.length());
    WriteBasicType(length);
    
    // 写入字符串内容
    writer->write(str.c_str(), str.length());
}

std::string BinaryFile::ReadString() {
    if (!isReading || !reader) return "";
    
    // 读取字符串长度
    std::uint32_t length = ReadBasicType<std::uint32_t>();
    
    // 读取字符串内容
    std::string result(length, '\0');
    reader->read(&result[0], length);
    
    return result;
}

std::string BinaryFile::FormatNumber(const std::string& format, double value) {
    std::ostringstream oss;
    
    if (format == "F1") {
        oss << std::fixed << std::setprecision(1) << value;
    }
    else if (format == "F2") {
        oss << std::fixed << std::setprecision(2) << value;
    }
    else if (format == "E") {
        oss << std::scientific << value;
    }
    else if (format == "P") {
        oss << std::fixed << std::setprecision(2) << (value * 100) << " %";
    }
    else if (format == "P0") {
        oss << std::fixed << std::setprecision(0) << (value * 100) << "%";
    }
    else if (format == "P1") {
        oss << std::fixed << std::setprecision(1) << (value * 100) << "%";
    }
    else if (format == "C") {
        oss << "$" << std::fixed << std::setprecision(2) << value;
    }
    else if (format == "N") {
        // 简单的千位分隔符格式
        oss << std::fixed << std::setprecision(1) << value;
        // 这里可以添加更复杂的千位分隔符逻辑
    }
    else {
        oss << value;
    }
    
    return oss.str();
}

std::string BinaryFile::FormatNumber(const std::string& format, float value) {
    return FormatNumber(format, static_cast<double>(value));
}

// 显式实例化常用的模板
template void BinaryFile::WriteData<short>(const short&);
template void BinaryFile::WriteData<int>(const int&);
template void BinaryFile::WriteData<std::int64_t>(const std::int64_t&);
template void BinaryFile::WriteData<float>(const float&);
template void BinaryFile::WriteData<double>(const double&);
template void BinaryFile::WriteData<char>(const char&);
template void BinaryFile::WriteData<bool>(const bool&);
template void BinaryFile::WriteData<std::string>(const std::string&);

template short BinaryFile::ReadData<short>();
template int BinaryFile::ReadData<int>();
template std::int64_t BinaryFile::ReadData<std::int64_t>();
template float BinaryFile::ReadData<float>();
template double BinaryFile::ReadData<double>();
template char BinaryFile::ReadData<char>();
template bool BinaryFile::ReadData<bool>();
template std::string BinaryFile::ReadData<std::string>();

template void BinaryFile::WriteLine<int>(const int&);
template void BinaryFile::WriteLine<double>(const double&);
template void BinaryFile::WriteLine<std::string>(const std::string&);

template void BinaryFile::Write<int>(const int&);
template void BinaryFile::Write<double>(const double&);
template void BinaryFile::Write<std::string>(const std::string&);

} // namespace IO
} // namespace IO
} // namespace HawtC3