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


#include "../Log/LogHelper.h"
#include "Extensions.h"
#include <filesystem>
#include <algorithm>
#include <numeric>
#include <cctype>
#include <sstream>


using namespace HawtC3::IO::Log;
using namespace std;

namespace HawtC3 {
namespace IO {
namespace IO {

double Extensions::Max(const Eigen::MatrixXd& value) {
    Eigen::VectorXd x(value.cols());
    for (int i = 0; i < value.cols(); i++) {
        x[i] = value.col(i).maxCoeff();
    }
    return x.maxCoeff();
}

double Extensions::Min(const Eigen::MatrixXd& value) {
    Eigen::VectorXd x(value.cols());
    for (int i = 0; i < value.cols(); i++) {
        x[i] = value.col(i).minCoeff();
    }
    return x.minCoeff();
}

double Extensions::Mean(const Eigen::MatrixXd& value) {
    Eigen::VectorXd x(value.cols());
    for (int i = 0; i < value.cols(); i++) {
        x[i] = value.col(i).mean();
    }
    return x.mean();
}

double Extensions::AbsMax(const Eigen::MatrixXd& value) {
    Eigen::VectorXd x(value.cols());
    for (int i = 0; i < value.cols(); i++) {
        x[i] = value.col(i).cwiseAbs().maxCoeff();
    }
    return x.maxCoeff();
}

double Extensions::AbsMin(const Eigen::MatrixXd& value) {
    Eigen::VectorXd x(value.cols());
    for (int i = 0; i < value.cols(); i++) {
        x[i] = value.col(i).cwiseAbs().minCoeff();
    }
    return x.minCoeff();
}

Eigen::VectorXd Extensions::Std(const Eigen::MatrixXd& value, int dim) {
    if (dim == 2) {
        Eigen::VectorXd x(value.cols());
        for (int i = 0; i < value.cols(); i++) {
            Eigen::VectorXd col = value.col(i);
            double mean = col.mean();
            x[i] = std::sqrt((col.array() - mean).square().sum() / (col.size() - 1));
        }
        return x;
    }
    else if (dim == 1) {
        Eigen::VectorXd x(value.rows());
        for (int i = 0; i < value.rows(); i++) {
            Eigen::VectorXd row = value.row(i);
            double mean = row.mean();
            x[i] = std::sqrt((row.array() - mean).square().sum() / (row.size() - 1));
        }
        return x;
    }
    else {
        LogHelper::ErrorLog("Std方法的dim参数值不正确，必须为1或2。");
        return Eigen::VectorXd(); // 返回一个空向量
    }
}

// Float matrix operations implementations
float Extensions::Max(const Eigen::MatrixXf& value) {
    Eigen::VectorXf x(value.cols());
    for (int i = 0; i < value.cols(); i++) {
        x[i] = value.col(i).maxCoeff();
    }
    return x.maxCoeff();
}

float Extensions::Min(const Eigen::MatrixXf& value) {
    Eigen::VectorXf x(value.cols());
    for (int i = 0; i < value.cols(); i++) {
        x[i] = value.col(i).minCoeff();
    }
    return x.minCoeff();
}

float Extensions::Mean(const Eigen::MatrixXf& value) {
    Eigen::VectorXf x(value.cols());
    for (int i = 0; i < value.cols(); i++) {
        x[i] = value.col(i).mean();
    }
    return x.mean();
}

float Extensions::AbsMax(const Eigen::MatrixXf& value) {
    Eigen::VectorXf x(value.cols());
    for (int i = 0; i < value.cols(); i++) {
        x[i] = value.col(i).cwiseAbs().maxCoeff();
    }
    return x.maxCoeff();
}

float Extensions::AbsMin(const Eigen::MatrixXf& value) {
    Eigen::VectorXf x(value.cols());
    for (int i = 0; i < value.cols(); i++) {
        x[i] = value.col(i).cwiseAbs().minCoeff();
    }
    return x.minCoeff();
}

Eigen::VectorXf Extensions::Std(const Eigen::MatrixXf& value, int dim) {
    if (dim == 2) {
        Eigen::VectorXf x(value.cols());
        for (int i = 0; i < value.cols(); i++) {
            Eigen::VectorXf col = value.col(i);
            float mean = col.mean();
            x[i] = std::sqrt((col.array() - mean).square().sum() / (col.size() - 1));
        }
        return x;
    }
    else if (dim == 1) {
        Eigen::VectorXf x(value.rows());
        for (int i = 0; i < value.rows(); i++) {
            Eigen::VectorXf row = value.row(i);
            float mean = row.mean();
            x[i] = std::sqrt((row.array() - mean).square().sum() / (row.size() - 1));
        }
        return x;
    }
    else
    {
        LogHelper::ErrorLog("Std方法的dim参数值不正确，必须为1或2。","","",20,"Extensions::Std");
		return Eigen::VectorXf(); // 返回一个空向量
    }
}

// String operations implementations
std::string Extensions::GetABSPath(const std::string& str) {
    return std::filesystem::absolute(str).string();
}

std::string Extensions::GetDirectoryName(const std::string& str) {
    auto path = std::filesystem::path(GetABSPath( str)).parent_path();
    if (path.empty()) {
        throw std::invalid_argument("无法获取路径的目录名。");
    }
    return path.string();
}

std::string Extensions::GetFileNameWithoutExtension(const std::string& str) {
    return std::filesystem::path(str).stem().string();
}

std::vector<std::string> Extensions::RemoveNull(const std::vector<std::string>& array) {
    std::vector<std::string> result;
    std::copy_if(array.begin(), array.end(), std::back_inserter(result),
        [](const std::string& s) { return !s.empty(); });
    return result;
}

std::vector<std::string> Extensions::TrimNull(const std::vector<std::string>& array) {
    std::vector<std::string> result;
    std::transform(array.begin(), array.end(), std::back_inserter(result),
        [](std::string s) {
            s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
            s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
            return s;
        });
    return result;
}

int Extensions::ToInt(const std::string& str) {
    std::string trimmed = str;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
    return std::stoi(trimmed);
}

std::vector<int> Extensions::ToIntArray(const std::string& str, const std::vector<char>& split) {
   
    std::vector<int> result = {};
    std::string current = "";

    for (char c : str) {
        // 检查当前字符是否是分隔符
        bool isDelimiter = std::find(split.begin(), split.end(), c) != split.end();

        if (isDelimiter) {
            // 如果当前字符串不为空且不全是空格，转换为整数
            if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos) {
                try {
                    result.push_back(std::stoi(current));
                }
                catch (const std::exception&) {
                    // 忽略无法转换的字符串
                }
            }
            current.clear();
        }
        else {
            current += c;
        }
    }

    // 处理最后一个子字符串
    if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos) {
        try {
            result.push_back(std::stoi(current));
        }
        catch (const std::exception&) {
            // 忽略无法转换的字符串
        }
    }

    return result;
}

double Extensions::ToDouble(const std::string& str) {
    std::string trimmed = str;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
    double result = std::stod(trimmed);
    return result;
}

std::vector<double> Extensions::ToDoubleArray(const std::string& str, const std::vector<char>& split) {
    std::vector<double> result = {};
    std::string current = "";

    for (char c : str) {
        // 检查当前字符是否是分隔符
        bool isDelimiter = std::find(split.begin(), split.end(), c) != split.end();

        if (isDelimiter) {
            // 如果当前字符串不为空且不全是空格，转换为整数
            if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos) {
                try {
                    result.push_back(std::stod(current));
                }
                catch (const std::exception&) {
                    // 忽略无法转换的字符串
                }
            }
            current.clear();
        }
        else {
            current += c;
        }
    }

    // 处理最后一个子字符串
    if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos) {
        try {
            result.push_back(std::stod(current));
        }
        catch (const std::exception&) {
            // 忽略无法转换的字符串
        }
    }

    return result;
}

bool Extensions::ToBool(const std::string& str) {
    std::string trimmed = str;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
    
    std::transform(trimmed.begin(), trimmed.end(), trimmed.begin(), ::tolower);
    if (trimmed == "true") return true;
    if (trimmed == "false") return false;
    throw std::invalid_argument("Invalid boolean string");
}

std::vector<bool> Extensions::ToBoolArray(const std::string& str, const std::vector<char>& split) {
    std::vector<std::string> parts;
    std::stringstream ss(str);
    std::string item;
    
    // Create a string containing all split characters
    std::string splitChars(split.begin(), split.end());
    
    while (std::getline(ss, item, split[0])) {
        if (!item.empty()) {
            // Remove any other split characters
            item.erase(std::remove_if(item.begin(), item.end(),
                [&splitChars](char c) { return splitChars.find(c) != std::string::npos; }),
                item.end());
            if (!item.empty()) {
                parts.push_back(item);
            }
        }
    }

    std::vector<bool> result;
    std::transform(parts.begin(), parts.end(), std::back_inserter(result),
        [](const std::string& s) { return ToBool(s); });
    return result;
}

std::vector<std::string> Extensions::ToStringArray(const std::string& str, const std::vector<char>& split) {
    std::vector<string> result = {};
    std::string current = "";

    for (char c : str) {
        // 检查当前字符是否是分隔符
        bool isDelimiter = std::find(split.begin(), split.end(), c) != split.end();

        if (isDelimiter) {
            // 如果当前字符串不为空且不全是空格，转换为整数
            if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos) {
                try {
                    result.push_back(current);
                }
                catch (const std::exception&) {
                    // 忽略无法转换的字符串
                }
            }
            current.clear();
        }
        else {
            current += c;
        }
    }

    // 处理最后一个子字符串
    if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos) {
        try {
            result.push_back(current);
        }
        catch (const std::exception&) {
            // 忽略无法转换的字符串
        }
    }

    return result;
}

} // namespace IO
} // namespace IO
} // namespace HawtC3