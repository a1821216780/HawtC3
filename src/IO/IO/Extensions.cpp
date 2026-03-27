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

#include "../Log/LogHelper.h"
#include <filesystem>
#include <algorithm>
#include <numeric>
#include <cctype>
#include <sstream>

#include "Extensions.h"

using namespace Qahse::IO::Log;
using namespace std;

namespace Qahse::IO::IO
{
    /**
     * @brief 计算 double 矩阵所有元素中的最大值（按列逐列扫描）
     * @param value 输入 double 矩阵
     * @return 所有元素中的最大值
     * @code
     * Eigen::MatrixXd m(2, 2);
     * m << 1.0, -2.0, 3.0, 4.0;
     * double res = Extensions::Max(m); // 4.0
     * @endcode
     */
    double Extensions::Max(const Eigen::MatrixXd &value)
    {
        Eigen::VectorXd x(value.cols()); ///< 存储每列的最大值
        for (int i = 0; i < value.cols(); i++)
        {
            x[i] = value.col(i).maxCoeff(); ///< 第 i 列的最大值
        }
        return x.maxCoeff(); ///< 所有列最大值中的最大值
    }

    /**
     * @brief 计算 double 矩阵所有元素中的最小值（按列逐列扫描）
     * @param value 输入 double 矩阵
     * @return 所有元素中的最小值
     * @code
     * Eigen::MatrixXd m(2, 2);
     * m << 1.0, -2.0, 3.0, 4.0;
     * double res = Extensions::Min(m); // -2.0
     * @endcode
     */
    double Extensions::Min(const Eigen::MatrixXd &value)
    {
        Eigen::VectorXd x(value.cols()); ///< 存储每列的最小值
        for (int i = 0; i < value.cols(); i++)
        {
            x[i] = value.col(i).minCoeff(); ///< 第 i 列的最小值
        }
        return x.minCoeff(); ///< 所有列最小值中的最小值
    }

    /**
     * @brief 计算 double 矩阵全局均值（先求各列均值，再求均值）
     * @param value 输入 double 矩阵
     * @return 全局均值
     * @code
     * Eigen::MatrixXd m(2, 2);
     * m << 1.0, 2.0, 3.0, 4.0;
     * double res = Extensions::Mean(m); // 2.5
     * @endcode
     */
    double Extensions::Mean(const Eigen::MatrixXd &value)
    {
        Eigen::VectorXd x(value.cols()); ///< 存储每列的均值
        for (int i = 0; i < value.cols(); i++)
        {
            x[i] = value.col(i).mean(); ///< 第 i 列的均值
        }
        return x.mean(); ///< 各列均值的均值
    }

    /**
     * @brief 计算 double 矩阵所有元素绝对值中的最大值
     * @param value 输入 double 矩阵
     * @return 所有元素绝对值中的最大值
     * @code
     * Eigen::MatrixXd m(2, 2);
     * m << 1.0, -5.0, 3.0, 4.0;
     * double res = Extensions::AbsMax(m); // 5.0
     * @endcode
     */
    double Extensions::AbsMax(const Eigen::MatrixXd &value)
    {
        Eigen::VectorXd x(value.cols()); ///< 存储每列的绝对值最大值
        for (int i = 0; i < value.cols(); i++)
        {
            x[i] = value.col(i).cwiseAbs().maxCoeff(); ///< 第 i 列取绝对值后的最大值
        }
        return x.maxCoeff(); ///< 所有列绝对值最大值中的最大值
    }

    /**
     * @brief 计算 double 矩阵所有元素绝对值中的最小值
     * @param value 输入 double 矩阵
     * @return 所有元素绝对值中的最小值
     * @code
     * Eigen::MatrixXd m(2, 2);
     * m << 1.0, -5.0, 3.0, 4.0;
     * double res = Extensions::AbsMin(m); // 1.0
     * @endcode
     */
    double Extensions::AbsMin(const Eigen::MatrixXd &value)
    {
        Eigen::VectorXd x(value.cols()); ///< 存储每列的绝对值最小值
        for (int i = 0; i < value.cols(); i++)
        {
            x[i] = value.col(i).cwiseAbs().minCoeff(); ///< 第 i 列取绝对值后的最小值
        }
        return x.minCoeff(); ///< 所有列绝对值最小值中的最小值
    }

    /**
     * @brief 计算 double 矩阵沿指定维度的样本标准差（无偏，除以 N-1）
     * @param value 输入 double 矩阵
     * @param dim 计算维度：1 表示按行计算，2 表示按列计算
     * @return 各行或各列的样本标准差向量
     * @code
     * Eigen::MatrixXd m(3, 2);
     * m << 1, 2,
     *      3, 4,
     *      5, 6;
     * Eigen::VectorXd stdCol = Extensions::Std(m, 2); // 各列标准差
     * Eigen::VectorXd stdRow = Extensions::Std(m, 1); // 各行标准差
     * @endcode
     */
    Eigen::VectorXd Extensions::Std(const Eigen::MatrixXd &value, int dim)
    {
        if (dim == 2)
        {
            Eigen::VectorXd x(value.cols()); ///< 存储各列的样本标准差
            for (int i = 0; i < value.cols(); i++)
            {
                Eigen::VectorXd col = value.col(i);                                       ///< 第 i 列数据
                double mean = col.mean();                                                 ///< 第 i 列均值
                x[i] = std::sqrt((col.array() - mean).square().sum() / (col.size() - 1)); ///< 无偏样本标准差
            }
            return x;
        }
        else if (dim == 1)
        {
            Eigen::VectorXd x(value.rows()); ///< 存储各行的样本标准差
            for (int i = 0; i < value.rows(); i++)
            {
                Eigen::VectorXd row = value.row(i);                                       ///< 第 i 行数据
                double mean = row.mean();                                                 ///< 第 i 行均值
                x[i] = std::sqrt((row.array() - mean).square().sum() / (row.size() - 1)); ///< 无偏样本标准差
            }
            return x;
        }
        else
        {
            LogHelper::ErrorLog("Std方法的dim参数值不正确，必须为1或2。");
            return Eigen::VectorXd(); // 参数非法时返回空向量
        }
    }

    /**
     * @brief 计算 float 矩阵按列最大值中的最大值
     * @param value 输入 float 矩阵
     * @return 所有元素中的最大值
     * @code
     * Eigen::MatrixXf m(2, 2);
     * m << 1.0f, -2.0f, 3.0f, 4.0f;
     * float res = Extensions::Max(m); // 4.0f
     * @endcode
     */
    float Extensions::Max(const Eigen::MatrixXf &value)
    {
        Eigen::VectorXf x(value.cols()); ///< 存储每列最大值
        for (int i = 0; i < value.cols(); i++)
        {
            x[i] = value.col(i).maxCoeff(); ///< 当前列最大值
        }
        return x.maxCoeff(); ///< 所有列最大值中的最大值
    }

    /**
     * @brief 计算 float 矩阵按列最小值中的最小值
     * @param value 输入 float 矩阵
     * @return 所有元素中的最小值
     * @code
     * Eigen::MatrixXf m(2, 2);
     * m << 1.0f, -2.0f, 3.0f, 4.0f;
     * float res = Extensions::Min(m); // -2.0f
     * @endcode
     */
    float Extensions::Min(const Eigen::MatrixXf &value)
    {
        Eigen::VectorXf x(value.cols()); ///< 存储每列最小值
        for (int i = 0; i < value.cols(); i++)
        {
            x[i] = value.col(i).minCoeff(); ///< 当前列最小值
        }
        return x.minCoeff(); ///< 所有列最小值中的最小值
    }

    /**
     * @brief 计算 float 矩阵按列均值的均值
     * @param value 输入 float 矩阵
     * @return 所有元素的均值
     * @code
     * Eigen::MatrixXf m(2, 2);
     * m << 1.0f, -2.0f, 3.0f, 4.0f;
     * float res = Extensions::Mean(m); // 1.5f
     * @endcode
     */
    float Extensions::Mean(const Eigen::MatrixXf &value)
    {
        Eigen::VectorXf x(value.cols()); ///< 存储每列均值
        for (int i = 0; i < value.cols(); i++)
        {
            x[i] = value.col(i).mean(); ///< 当前列均值
        }
        return x.mean(); ///< 所有列均值的均值
    }

    /**
     * @brief 计算 float 矩阵按列绝对值最大值中的最大值
     * @param value 输入 float 矩阵
     * @return 所有元素绝对值中的最大值
     * @code
     * Eigen::MatrixXf m(2, 2);
     * m << 1.0f, -5.0f, 3.0f, 4.0f;
     * float res = Extensions::AbsMax(m); // 5.0f
     * @endcode
     */
    float Extensions::AbsMax(const Eigen::MatrixXf &value)
    {
        Eigen::VectorXf x(value.cols()); ///< 存储每列绝对值最大值
        for (int i = 0; i < value.cols(); i++)
        {
            x[i] = value.col(i).cwiseAbs().maxCoeff(); ///< 当前列绝对值最大值
        }
        return x.maxCoeff(); ///< 所有列绝对值最大值中的最大值
    }

    /**
     * @brief 计算 float 矩阵按列绝对值最小值中的最小值
     * @param value 输入 float 矩阵
     * @return 所有元素绝对值中的最小值
     * @code
     * Eigen::MatrixXf m(2, 2);
     * m << 1.0f, -5.0f, 3.0f, 4.0f;
     * float res = Extensions::AbsMin(m); // 1.0f
     * @endcode
     */
    float Extensions::AbsMin(const Eigen::MatrixXf &value)
    {
        Eigen::VectorXf x(value.cols()); ///< 存储每列绝对值最小值
        for (int i = 0; i < value.cols(); i++)
        {
            x[i] = value.col(i).cwiseAbs().minCoeff(); ///< 当前列绝对值最小值
        }
        return x.minCoeff(); ///< 所有列绝对值最小值中的最小值
    }

    /**
     * @brief 计算 float 矩阵沿指定维度的样本标准差
     * @param value 输入 float 矩阵
     * @param dim 计算维度：1 表示按行，2 表示按列
     * @return 各行或各列的标准差向量
     * @code
     * Eigen::MatrixXf m(2, 2);
     * m << 1.0f, 2.0f, 3.0f, 4.0f;
     * Eigen::VectorXf stdCol = Extensions::Std(m, 2); // 按列标准差
     * Eigen::VectorXf stdRow = Extensions::Std(m, 1); // 按行标准差
     * @endcode
     */
    Eigen::VectorXf Extensions::Std(const Eigen::MatrixXf &value, int dim)
    {
        if (dim == 2)
        {
            Eigen::VectorXf x(value.cols()); ///< 存储各列的标准差
            for (int i = 0; i < value.cols(); i++)
            {
                Eigen::VectorXf col = value.col(i);                                       ///< 当前列数据
                float mean = col.mean();                                                  ///< 当前列均值
                x[i] = std::sqrt((col.array() - mean).square().sum() / (col.size() - 1)); ///< 样本标准差公式
            }
            return x;
        }
        else if (dim == 1)
        {
            Eigen::VectorXf x(value.rows()); ///< 存储各行的标准差
            for (int i = 0; i < value.rows(); i++)
            {
                Eigen::VectorXf row = value.row(i);                                       ///< 当前行数据
                float mean = row.mean();                                                  ///< 当前行均值
                x[i] = std::sqrt((row.array() - mean).square().sum() / (row.size() - 1)); ///< 样本标准差公式
            }
            return x;
        }
        else
        {
            LogHelper::ErrorLog("Std方法的dim参数值不正确，必须为1或2。", "", "", 20, "Extensions::Std");
            return Eigen::VectorXf(); // 参数非法时返回空向量
        }
    }

    /**
     * @brief 将相对路径或任意路径字符串转换为绝对路径
     * @param str 输入路径字符串（可以是相对路径或绝对路径）
     * @return 对应的绝对路径字符串
     * @code
     * std::string absPath = Extensions::GetABSPath("./data/input.txt");
     * // 例如返回 "E:/Qahse/data/input.txt"
     * @endcode
     */
    std::string Extensions::GetABSPath(const std::string &str)
    {
        return std::filesystem::absolute(str).string(); ///< 调用标准库获取绝对路径
    }

    /**
     * @brief 获取路径中的目录部分（去掉文件名），例如 "c:/tmp/file.txt" -> "c:/tmp"
     * @param str 输入路径字符串
     * @return 目录路径字符串
     * @throws std::invalid_argument 如果无法获取目录（如输入仅为文件名）
     * @code
     * std::string dir = Extensions::GetDirectoryName("c:/tmp/file.txt");
     * // 返回 "c:/tmp"
     * @endcode
     */
    std::string Extensions::GetDirectoryName(const std::string &str)
    {
        auto path = std::filesystem::path(GetABSPath(str)).parent_path(); ///< 获取父目录路径
        if (path.empty())
        {
            throw std::invalid_argument("无法获取路径的目录名。");
        }
        return path.string();
    }

    /**
     * @brief 获取路径中的文件名，不包含扩展名
     * @param str 输入路径字符串
     * @return 文件名字符串（不含扩展名）
     * @code
     * std::string name = Extensions::GetFileNameWithoutExtension("c:/tmp/model.dat");
     * // 返回 "model"
     * @endcode
     */
    std::string Extensions::GetFileNameWithoutExtension(const std::string &str)
    {
        return std::filesystem::path(str).stem().string(); ///< 提取文件主名（stem）
    }

    /**
     * @brief 从字符串数组中移除所有空字符串元素
     * @param array 输入字符串数组
     * @return 仅保留非空字符串的新数组
     * @code
     * std::vector<std::string> arr = {"abc", "", "def", "", "xyz"};
     * std::vector<std::string> res = Extensions::RemoveNull(arr);
     * // res = {"abc", "def", "xyz"}
     * @endcode
     */
    std::vector<std::string> Extensions::RemoveNull(const std::vector<std::string> &array)
    {
        std::vector<std::string> result; ///< 存储过滤后的非空字符串
        std::copy_if(array.begin(), array.end(), std::back_inserter(result),
                     [](const std::string &s)
                     { return !s.empty(); }); ///< 仅复制非空字符串
        return result;
    }

    /**
     * @brief 去除字符串数组中每个元素的首尾空白字符（空格、制表符、换行等）
     * @param array 输入字符串数组
     * @return 首尾空白已被去除的新数组
     * @code
     * std::vector<std::string> arr = {"  abc  ", " def", "xyz "};
     * std::vector<std::string> res = Extensions::TrimNull(arr);
     * // res = {"abc", "def", "xyz"}
     * @endcode
     */
    std::vector<std::string> Extensions::TrimNull(const std::vector<std::string> &array)
    {
        std::vector<std::string> result; ///< 存储去除首尾空白后的字符串
        std::transform(array.begin(), array.end(), std::back_inserter(result),
                       [](std::string s)
                       {
                           s.erase(0, s.find_first_not_of(" \t\n\r\f\v")); ///< 去除前导空白
                           s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1); ///< 去除尾部空白
                           return s;
                       });
        return result;
    }

    /**
     * @brief 将去除首尾空白后的字符串转换为 int 类型
     * @param str 输入字符串
     * @return 整数值
     * @throws std::invalid_argument 字符串非整数格式时抛出
     * @throws std::out_of_range 数值超出 int 范围时抛出
     * @code
     * int val = Extensions::ToInt("  42  "); // val = 42
     * int val2 = Extensions::ToInt("-100");  // val2 = -100
     * @endcode
     */
    int Extensions::ToInt(const std::string &str)
    {
        std::string trimmed = str;                                  ///< 工作副本，用于去除首尾空白
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v")); ///< 去除前导空白
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1); ///< 去除尾部空白
        return std::stoi(trimmed);
    }

    /**
     * @brief 将字符串按指定分隔符分割，并将各子串转换为 int 数组
     * @param str 输入字符串
     * @param split 分隔符字符集合（可包含多个分隔符）
     * @return 转换后的整数数组，无法转换的子串将被忽略
     * @code
     * std::vector<int> arr = Extensions::ToIntArray("1, 2, 3, 4", {',',' '});
     * // arr = {1, 2, 3, 4}
     * std::vector<int> arr2 = Extensions::ToIntArray("10 20 30", {' '});
     * // arr2 = {10, 20, 30}
     * @endcode
     */
    std::vector<int> Extensions::ToIntArray(const std::string &str, const std::vector<char> &split)
    {
        std::vector<int> result = {}; ///< 存储转换后的整数结果
        std::string current = "";     ///< 当前正在积累的子字符串

        for (char c : str)
        {
            // 检查当前字符是否是分隔符
            bool isDelimiter = std::find(split.begin(), split.end(), c) != split.end();

            if (isDelimiter)
            {
                // 如果当前字符串不为空且不全是空格，转换为整数
                if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos)
                {
                    try
                    {
                        result.push_back(std::stoi(current)); ///< 转换并追加到结果
                    }
                    catch (const std::exception &)
                    {
                        // 忽略无法转换的字符串（如非数字内容）
                    }
                }
                current.clear(); ///< 清空当前子串，准备下一段
            }
            else
            {
                current += c; ///< 继续积累当前子串
            }
        }

        // 处理最后一个子字符串（末尾无分隔符时）
        if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos)
        {
            try
            {
                result.push_back(std::stoi(current));
            }
            catch (const std::exception &)
            {
                // 忽略无法转换的字符串
            }
        }

        return result;
    }

    /**
     * @brief 将去除首尾空白后的字符串转换为 double 类型
     * @param str 输入字符串
     * @return double 值
     * @throws std::invalid_argument 字符串非浮点格式时抛出
     * @throws std::out_of_range 数值超出 double 范围时抛出
     * @code
     * double val = Extensions::ToDouble("  3.14  "); // val = 3.14
     * double val2 = Extensions::ToDouble("-2.718");  // val2 = -2.718
     * @endcode
     */
    double Extensions::ToDouble(const std::string &str)
    {
        std::string trimmed = str;                                  ///< 工作副本，用于去除首尾空白
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v")); ///< 去除前导空白
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1); ///< 去除尾部空白
        double result = std::stod(trimmed);                         ///< 执行字符串到 double 的转换
        return result;
    }

    /**
     * @brief 将字符串按指定分隔符分割，并将各子串转换为 double 数组
     * @param str 输入字符串
     * @param split 分隔符字符集合（可包含多个分隔符）
     * @return 转换后的 double 数组，无法转换的子串将被忽略
     * @code
     * std::vector<double> arr = Extensions::ToDoubleArray("1.1, 2.2, 3.3", {',',' '});
     * // arr = {1.1, 2.2, 3.3}
     * std::vector<double> arr2 = Extensions::ToDoubleArray("0.5 1.5 2.5", {' '});
     * // arr2 = {0.5, 1.5, 2.5}
     * @endcode
     */
    std::vector<double> Extensions::ToDoubleArray(const std::string &str, const std::vector<char> &split)
    {
        std::vector<double> result = {}; ///< 存储转换后的 double 结果
        std::string current = "";        ///< 当前正在积累的子字符串

        for (char c : str)
        {
            // 检查当前字符是否是分隔符
            bool isDelimiter = std::find(split.begin(), split.end(), c) != split.end();

            if (isDelimiter)
            {
                // 如果当前字符串不为空且不全是空格，转换为 double
                if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos)
                {
                    try
                    {
                        result.push_back(std::stod(current)); ///< 转换并追加到结果
                    }
                    catch (const std::exception &)
                    {
                        // 忽略无法转换的字符串
                    }
                }
                current.clear(); ///< 清空当前子串，准备下一段
            }
            else
            {
                current += c; ///< 继续积累当前子串
            }
        }

        // 处理最后一个子字符串（末尾无分隔符时）
        if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos)
        {
            try
            {
                result.push_back(std::stod(current));
            }
            catch (const std::exception &)
            {
                // 忽略无法转换的字符串
            }
        }

        return result;
    }

    /**
     * @brief 将字符串转换为 bool 类型（忽略大小写和首尾空白）
     * @param str 输入字符串，合法值为 "true" 或 "false"（不区分大小写）
     * @return true 或 false
     * @throws std::invalid_argument 如果字符串不是有效的布尔值表示
     * @code
     * bool b1 = Extensions::ToBool(" True ");  // b1 = true
     * bool b2 = Extensions::ToBool("FALSE");   // b2 = false
     * @endcode
     */
    bool Extensions::ToBool(const std::string &str)
    {
        std::string trimmed = str;                                  ///< 工作副本，用于去除首尾空白
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v")); ///< 去除前导空白
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1); ///< 去除尾部空白

        std::transform(trimmed.begin(), trimmed.end(), trimmed.begin(), ::tolower); ///< 转为小写以不区分大小写
        if (trimmed == "true")
            return true;
        if (trimmed == "false")
            return false;
        throw std::invalid_argument("Invalid boolean string"); ///< 非法输入抛出异常
    }

    /**
     * @brief 将字符串按指定分隔符分割，并将各子串转换为 bool 数组
     * @param str 输入字符串
     * @param split 分隔符字符集合（可包含多个分隔符），以 split[0] 作为主分隔符流式读取
     * @return 转换后的 bool 数组
     * @throws std::invalid_argument 如果某个子串不是合法的布尔值字符串
     * @code
     * std::vector<bool> arr = Extensions::ToBoolArray("true,false,True", {','});
     * // arr = {true, false, true}
     * @endcode
     */
    std::vector<bool> Extensions::ToBoolArray(const std::string &str, const std::vector<char> &split)
    {
        std::vector<std::string> parts; ///< 存储分割后的各子串
        std::stringstream ss(str);      ///< 用于流式按字符分割
        std::string item;               ///< 当前读取的子串

        std::string splitChars(split.begin(), split.end()); ///< 所有分隔符组成的字符串（用于多字符过滤）

        while (std::getline(ss, item, split[0])) ///< 以第一个分隔符进行流式分割
        {
            if (!item.empty())
            {
                // 去除子串中其他分隔符字符
                item.erase(std::remove_if(item.begin(), item.end(),
                                          [&splitChars](char c)
                                          { return splitChars.find(c) != std::string::npos; }),
                           item.end());
                if (!item.empty())
                {
                    parts.push_back(item); ///< 加入有效子串列表
                }
            }
        }

        std::vector<bool> result; ///< 存储转换后的 bool 结果
        std::transform(parts.begin(), parts.end(), std::back_inserter(result),
                       [](const std::string &s)
                       { return ToBool(s); }); ///< 逐一转换为 bool
        return result;
    }

    /**
     * @brief 将字符串按指定分隔符分割为字符串数组，自动跳过空白子串
     * @param str 输入字符串
     * @param split 分隔符字符集合（可包含多个分隔符）
     * @return 分割后的非空白字符串数组
     * @code
     * std::vector<std::string> arr = Extensions::ToStringArray("a, b, c", {',',' '});
     * // arr = {"a", "b", "c"}
     * std::vector<std::string> arr2 = Extensions::ToStringArray("hello world", {' '});
     * // arr2 = {"hello", "world"}
     * @endcode
     */
    std::vector<std::string> Extensions::ToStringArray(const std::string &str, const std::vector<char> &split)
    {
        std::vector<string> result = {}; ///< 存储分割后的字符串结果
        std::string current = "";        ///< 当前正在积累的子字符串

        for (char c : str)
        {
            // 检查当前字符是否是分隔符
            bool isDelimiter = std::find(split.begin(), split.end(), c) != split.end();

            if (isDelimiter)
            {
                // 如果当前子串不为空且非纯空白，则加入结果
                if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos)
                {
                    try
                    {
                        result.push_back(current); ///< 追加当前子串到结果
                    }
                    catch (const std::exception &)
                    {
                        // 忽略异常（实际上 push_back 不会抛出，此处仅保留结构一致性）
                    }
                }
                current.clear(); ///< 清空当前子串，准备下一段
            }
            else
            {
                current += c; ///< 继续积累当前子串
            }
        }

        // 处理最后一个子字符串（末尾无分隔符时）
        if (!current.empty() && current.find_first_not_of(" \t\n\r") != std::string::npos)
        {
            try
            {
                result.push_back(current);
            }
            catch (const std::exception &)
            {
                // 忽略异常
            }
        }

        return result;
    }
}