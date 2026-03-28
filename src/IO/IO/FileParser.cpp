#pragma once
//**********************************************************************************************************************************
// LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of Qahse.IO.Math
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

#include "FileParser.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <cerrno>

/**
 * @brief Qahse 输入文件解析命名空间
 * @details 该命名空间提供统一的输入文件解析能力，
 *          通过 FileParser 工具函数和模块化解析器类实现配置读取。
 */
namespace Qahse::IO::IO
{

        // =========================================================================
        // 辅助函数
        // =========================================================================

        // 去除字符串首尾空白

        /// @brief 去除字符串首尾空白
        /// @param s 输入字符串
        /// @return 去除首尾空白后的字符串
        static std::string TrimString(const std::string &s)
        {
            auto start = s.find_first_not_of(" \t\r\n");
            if (start == std::string::npos)
                return "";
            auto end = s.find_last_not_of(" \t\r\n");
            return s.substr(start, end - start + 1);
        }

        /// @brief 简化字符串，去除首尾空白并压缩内部空白
        /// @param s 输入字符串
        /// @return 简化后的字符串
        static std::string SimplifyString(const std::string &s)
        {
            std::string result;
            result.reserve(s.size());
            bool inSpace = true; // 跳过开头空白
            for (char c : s)
            {
                if (std::isspace(static_cast<unsigned char>(c)))
                {
                    if (!inSpace)
                    {
                        result += ' ';
                        inSpace = true;
                    }
                }
                else
                {
                    result += c;
                    inSpace = false;
                }
            }
            // 去除末尾空格
            if (!result.empty() && result.back() == ' ')
                result.pop_back();
            return result;
        }

        /// @brief 替换字符串中所有出现的子串
        /// @param str 输入字符串
        /// @param from 要被替换的子串
        /// @param to 替换后的子串
        static void ReplaceAll(std::string &str, const std::string &from, const std::string &to)
        {
            if (from.empty())
                return;
            size_t pos = 0;
            while ((pos = str.find(from, pos)) != std::string::npos)
            {
                str.replace(pos, from.length(), to);
                pos += to.length();
            }
        }

        // =========================================================================
        // UnifyString
        // =========================================================================
        /** @brief 看齐数字字符串的空格/符号，防止科学计数法被分割
         * 【功能】将数字中威胁符号/与空格的组合处理，配居分隔符为小数点
         * 【物理意义】将输入文件数据还原为OpenFAST/QBlade格式用double解析前的预处理
         * @param input 原始字符串
         * @return 看齐后的字符串
         * 【使用案例】 auto s = UnifyString("1 e-3"); // => "1e-3" */
        std::string FileParser::UnifyString(const std::string &input)
        {
            std::string s = input;
            ReplaceAll(s, " e", "_e");
            ReplaceAll(s, " E", "_E");
            ReplaceAll(s, "e ", "e_");
            ReplaceAll(s, "E ", "E_");
            // 注: QBlade 原始代码中 "\\-" 和 "\\+" 实际指 literal 反斜杠, 但在实际使用中
            // 不太可能出现, 以下保留原始模式的意图: 处理符号与空格的交互
            ReplaceAll(s, "- ", "-_");
            ReplaceAll(s, " -", "_-");
            ReplaceAll(s, "+ ", "+_");
            ReplaceAll(s, " +", "_+");
            ReplaceAll(s, "_", "");
            ReplaceAll(s, ",", ".");
            return s;
        }

        // =========================================================================
        // FileContentToStringList
        // =========================================================================
        /** @brief 将文本文件读入为行字符串列表
         * 【功能】逐行读取文件内容并返回字符串向量
         * 【物理意义】是所有文件解析的第一步，将磁盘文件内容载入内存俗序处理
         * @param filename 文件路径
         * @return 行字符串列表（文件不存在则返回空列表）
         * 【使用案例】 auto lines = FileContentToStringList("input.dat"); */
        std::vector<std::string> FileParser::FileContentToStringList(const std::string &filename)
        {
            std::vector<std::string> lines;
            std::ifstream file(filename);
            if (!file.is_open())
            {
                std::cerr << "Warning: Could not read the file: " << filename << std::endl;
                return lines;
            }
            std::string line;
            while (std::getline(file, line))
            {
                lines.push_back(line);
            }
            return lines;
        }

        // =========================================================================
        // TokenizeLine
        // =========================================================================
        /** @brief 将一行字符串按空格分割为多个令牌
         * 【功能】先压缩多余空格，可选技会逗号/分号为空格，再分割
         * 【物理意义】拆分行是关键词查找和数据提取的基础
         * @param line 输入行字符串
         * @param replaceSeparators 是否将逗号/分号也视为分隔符
         * @return 令牌字符串列表
         * 【使用案例】 auto toks = TokenizeLine("3.14   42 hello"); // {"3.14", "42", "hello"} */
        std::vector<std::string> FileParser::TokenizeLine(const std::string &line, bool replaceSeparators)
        {
            std::string simplified = SimplifyString(line);
            if (replaceSeparators)
            {
                ReplaceAll(simplified, ",", " ");
                ReplaceAll(simplified, ";", " ");
                simplified = SimplifyString(simplified);
            }
            std::vector<std::string> tokens;
            std::istringstream iss(simplified);
            std::string token;
            while (iss >> token)
            {
                tokens.push_back(token);
            }
            return tokens;
        }

        // =========================================================================
        // TokenizeFile
        // =========================================================================
        /** @brief 将多行字符串列表批量分割为令牌二维数组
         * 【功能】对fileLines每行调用TokenizeLine，返回二维令牌数组
         * 【物理意义】将文件行列表先分拆成结构化数据便于后续搜索
         * @param fileLines 文件行列表
         * @param replaceSeparators 是否技会逗号/分号
         * @return 令牌二维数组
         * 【使用案例】 auto tbl = TokenizeFile(lines, true); */
        std::vector<std::vector<std::string>> FileParser::TokenizeFile(
            const std::vector<std::string> &fileLines, bool replaceSeparators)
        {
            std::vector<std::vector<std::string>> result;
            result.reserve(fileLines.size());
            for (const auto &line : fileLines)
            {
                result.push_back(TokenizeLine(line, replaceSeparators));
            }
            return result;
        }

        // =========================================================================
        // FindKeywordInFile
        // =========================================================================
        /** @brief 在文件行列表中查找关键字是否存在
         * 【功能】对所有行进行分割并将每个令牌与keyword对比
         * @param keyword 要搜索的关键字
         * @param fileLines 文件行列表
         * @return true=发现关键字
         * 【使用案例】 if (FindKeywordInFile("WIND", lines)) { ... } */
        bool FileParser::FindKeywordInFile(const std::string &keyword, const std::vector<std::string> &fileLines)
        {
            for (const auto &line : fileLines)
            {
                auto tokens = TokenizeLine(line);
                for (const auto &tok : tokens)
                {
                    if (tok == keyword)
                        return true;
                }
            }
            return false;
        }

        // =========================================================================
        // FindValueInFile
        // =========================================================================
        /** @brief 在文件行列表中查找关键字并返回同行的第一个令牌
         * 【功能】搜索包含 keyword的行，返回该行的tokens[0]（即关键字前面的数据）
         * 【物理意义】常用于从 "VALUE  KEYWORD" 格式的文件中提取标量
         * @param keyword 关键字
         * @param fileLines 文件行列表
         * @param error_msg 如果自设置错误消息
         * @param setmsg 不找到时是否更改错误消息
         * @param found 输出：是否找到
         * @return 找到的标量字符串，未找到则返回空字符串
         * 【使用案例】 auto val = FindValueInFile("WindSpeed", lines); */
        std::string FileParser::FindValueInFile(const std::string &keyword,
                                    const std::vector<std::string> &fileLines,
                                    std::string *error_msg,
                                    bool setmsg,
                                    bool *found)
        {
            for (const auto &line : fileLines)
            {
                // Skip comment lines to avoid matching keywords inside comments.
                auto first = line.find_first_not_of(" \t\r\n");
                if (first == std::string::npos)
                    continue;
                if (line[first] == '#')
                    continue;

                auto tokens = TokenizeLine(line);
                // 搜索 tokens[j>=1] 中的 keyword, 找到后返回 tokens[0]
                for (size_t j = 1; j < tokens.size(); ++j)
                {
                    if (tokens[j] == keyword)
                    {
                        if (found)
                            *found = true;
                        return tokens[0];
                    }
                }
            }
            if (found)
                *found = false;
            if (setmsg && error_msg)
                error_msg->append("\nKeyword: " + keyword + " not found!");
            return "";
        }

        // =========================================================================
        // FindLineWithKeyword
        // =========================================================================
        /** @brief 在文件行列表中查找包含关键字的行并返回全行令牌
         * 【功能】搜索包含keyword的行，返回该行全部令牌列表
         * 【物理意义】常用于提取单行多个参数，如环境条件
         * @param keyword 关键字
         * @param fileLines 文件行列表
         * @param error_msg 错误信息输出
         * @param setmsg 是否设置错误消息
         * @param found 输出：是否找到
         * @param replaceSeparators 是否替换分隔符
         * @return 行令牌列表
         * 【使用案例】 auto toks = FindLineWithKeyword("BLADE_SECTIONS", lines); */
        std::vector<std::string> FileParser::FindLineWithKeyword(const std::string &keyword,
                                                     const std::vector<std::string> &fileLines,
                                                     std::string *error_msg,
                                                     bool setmsg,
                                                     bool *found,
                                                     bool replaceSeparators)
        {
            for (const auto &line : fileLines)
            {
                auto tokens = TokenizeLine(line, replaceSeparators);
                for (const auto &tok : tokens)
                {
                    if (tok == keyword)
                    {
                        if (found)
                            *found = true;
                        return tokens;
                    }
                }
            }
            if (found)
                *found = false;
            if (setmsg && error_msg)
                error_msg->append("\n Variable " + keyword + " not found in file");
            return {};
        }

        // =========================================================================
        // FindMatrixInFile
        // =========================================================================
        /** @brief 在文件中查找关键字并读取rows*cols矩阵
         * 【功能】定位keyword后的数值数据块，读取rows行cols列并封装为Eigen矩阵
         * 【物理意义】用于导入控制/结构的矩阵参数（如刚度矩阵、质量阵）
         * @param keyword 关键字
         * @param fileLines 文件行列表
         * @param rows 矩阵行数
         * @param cols 矩阵列数
         * @param error_msg 错误信息输出
         * @param setmsg 是否设置错误消息
         * @param found 输出：是否找到
         * @return 不找到返回零矩阵
         * 【使用案例】 auto M = FindMatrixInFile("MASS_MATRIX", lines, 6, 6); */
        Eigen::MatrixXd FileParser::FindMatrixInFile(const std::string &keyword,
                                         const std::vector<std::string> &fileLines,
                                         int rows, int cols,
                                         std::string *error_msg,
                                         bool setmsg,
                                         bool *found)
        {
            Eigen::MatrixXd mat = Eigen::MatrixXd::Zero(rows, cols);

            auto fileContents = TokenizeFile(fileLines);

            for (size_t i = 0; i < fileContents.size(); ++i)
            {
                for (size_t j = 0; j < fileContents[i].size(); ++j)
                {
                    if (fileContents[i][j] == keyword && j == 0)
                    {
                        // 找到关键字, 从下一行开始寻找数据
                        if (i + 1 < fileContents.size())
                        {
                            for (size_t skip = i + 1; skip < fileContents.size(); ++skip)
                            {
                                if (!fileContents[skip].empty())
                                {
                                    double testVal;
                                    if (TryParseDouble(fileContents[skip][0], testVal))
                                    {
                                        // 找到数据起始行, 读取 rows 行
                                        for (int k = 0; k < rows; ++k)
                                        {
                                            size_t lineIdx = skip + k;
                                            if (lineIdx < fileContents.size() &&
                                                static_cast<int>(fileContents[lineIdx].size()) >= cols)
                                            {
                                                for (int c = 0; c < cols; ++c)
                                                {
                                                    double num;
                                                    if (!TryParseDouble(fileContents[lineIdx][c], num))
                                                    {
                                                        if (found)
                                                            *found = false;
                                                        if (setmsg && error_msg)
                                                            error_msg->append("\n Matrix " + keyword +
                                                                              ": Must be " + std::to_string(rows) +
                                                                              "x" + std::to_string(cols));
                                                        return mat;
                                                    }
                                                    mat(k, c) = num;
                                                }
                                            }
                                        }
                                        if (found)
                                            *found = true;
                                        return mat;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (found)
                *found = false;
            if (setmsg && error_msg)
                error_msg->append("\n Matrix " + keyword + ": Must be " +
                                  std::to_string(rows) + "x" + std::to_string(cols));
            return mat;
        }

        // =========================================================================
        // FindStringDataTable
        // =========================================================================
        /** @brief 在文件中查找关键字后的字符串数据表
         * 【功能】将keyword后的数据行（每行至少cols令牌）读入二维字符串数组
         * 【物理意义】用于导入可能包含非纯数字的数据表（如叶片BEM数据表）
         * @param keyword 关键字
         * @param fileLines 文件行列表
         * @param cols 每行最少必须有的列数
         * @param error_msg 错误信息输出
         * @param setmsg 是否设置错误消息
         * @param found 输出：是否找到
         * @return 字符串数据表
         * 【使用案例】 auto t = FindStringDataTable("BladeSections", lines, 5); */
        std::vector<std::vector<std::string>> FileParser::FindStringDataTable(
            const std::string &keyword,
            const std::vector<std::string> &fileLines,
            int cols,
            std::string *error_msg,
            bool setmsg,
            bool *found)
        {

            std::vector<std::vector<std::string>> table;
            auto fileContents = TokenizeFile(fileLines);

            for (size_t i = 0; i < fileContents.size(); ++i)
            {
                for (size_t j = 0; j < fileContents[i].size(); ++j)
                {
                    if (fileContents[i][j] == keyword)
                    {
                        if (i + 1 < fileContents.size())
                        {
                            for (size_t skip = i + 1; skip < fileContents.size(); ++skip)
                            {
                                if (!fileContents[skip].empty())
                                {
                                    double testVal;
                                    if (TryParseDouble(fileContents[skip][0], testVal))
                                    {
                                        // 开始提取数据
                                        size_t line = skip;
                                        bool ok = true;
                                        while (ok)
                                        {
                                            std::vector<std::string> row;
                                            if (line < fileContents.size() &&
                                                static_cast<int>(fileContents[line].size()) >= cols)
                                            {
                                                for (const auto &tok : fileContents[line])
                                                {
                                                    row.push_back(tok);
                                                }
                                            }
                                            if (static_cast<int>(row.size()) >= cols)
                                            {
                                                table.push_back(row);
                                                ok = true;
                                            }
                                            else
                                            {
                                                ok = false;
                                            }
                                            line++;
                                        }
                                        if (!table.empty())
                                        {
                                            if (found)
                                                *found = true;
                                            return table;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (found)
                *found = false;
            if (setmsg && error_msg)
                error_msg->append("\n Data Table " + keyword +
                                  ": no data found! Must be " + std::to_string(cols) +
                                  " per data row!");
            return table;
        }

        // =========================================================================
        // FindNumericDataTable
        // =========================================================================
        /** @brief 在文件中查找关键字后的数字数据表
         * 【功能】将keyword后的数字行（每行至少cols个double）读入二维double数组
         * 【物理意义】用于导入数据表（如面层BEM浏览表、风場时间序列）
         * @param keyword 关键字
         * @param fileLines 文件行列表
         * @param cols 每行列数
         * @param error_msg 错误信息输出
         * @param setmsg 是否设置错误消息
         * @param found 输出：是否找到
         * @return double数据表
         * 【使用案例】 auto t = FindNumericDataTable("PolCurve", lines, 3); */
        std::vector<std::vector<double>> FileParser::FindNumericDataTable(
            const std::string &keyword,
            const std::vector<std::string> &fileLines,
            int cols,
            std::string *error_msg,
            bool setmsg,
            bool *found)
        {

            std::vector<std::vector<double>> table;
            auto fileContents = TokenizeFile(fileLines);

            for (size_t i = 0; i < fileContents.size(); ++i)
            {
                for (size_t j = 0; j < fileContents[i].size(); ++j)
                {
                    if (fileContents[i][j] == keyword)
                    {
                        if (i + 1 < fileContents.size())
                        {
                            for (size_t skip = i + 1; skip < fileContents.size(); ++skip)
                            {
                                if (!fileContents[skip].empty())
                                {
                                    double testVal;
                                    if (TryParseDouble(fileContents[skip][0], testVal))
                                    {
                                        size_t line = skip;
                                        bool ok = true;
                                        while (ok)
                                        {
                                            std::vector<double> row;
                                            if (line < fileContents.size() &&
                                                static_cast<int>(fileContents[line].size()) >= cols)
                                            {
                                                for (int c = 0; c < cols; ++c)
                                                {
                                                    double num;
                                                    if (TryParseDouble(fileContents[line][c], num))
                                                    {
                                                        row.push_back(num);
                                                    }
                                                }
                                            }
                                            if (static_cast<int>(row.size()) == cols)
                                            {
                                                table.push_back(row);
                                                ok = true;
                                            }
                                            else
                                            {
                                                ok = false;
                                            }
                                            line++;
                                        }
                                        if (!table.empty())
                                        {
                                            if (found)
                                                *found = true;
                                            return table;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (found)
                *found = false;
            if (setmsg && error_msg)
                error_msg->append("\n Data Table " + keyword +
                                  ": no data found! Must be " + std::to_string(cols) +
                                  " per data row!");
            return table;
        }

        // =========================================================================
        // FindNumericValuesInFile
        // =========================================================================
        /** @brief 在文件中提取所有纯数字行
         * 【功能】遍历文件行，对全行均为数字且列数不小于minColCount的行将其读入
         * 【物理意义】用于导入无关键字标识的纯数字表（如FAST风场文件）
         * @param minColCount 最少列数
         * @param fileLines 文件行列表
         * @param error_msg 错误信息输出
         * @param fileName 文件名（用于错误信息）
         * @return double数据二维数组
         * 【使用案例】 auto v = FindNumericValuesInFile(3, lines); */
        std::vector<std::vector<double>> FileParser::FindNumericValuesInFile(
            int minColCount,
            const std::vector<std::string> &fileLines,
            std::string *error_msg,
            const std::string &fileName)
        {

            auto fileContents = TokenizeFile(fileLines);

            std::vector<std::vector<double>> values;
            for (size_t i = 0; i < fileContents.size(); ++i)
            {
                bool valid = true;
                for (const auto &tok : fileContents[i])
                {
                    if (!IsNumeric(tok))
                    {
                        valid = false;
                        break;
                    }
                }
                if (valid && static_cast<int>(fileContents[i].size()) >= minColCount)
                {
                    std::vector<double> row;
                    for (const auto &tok : fileContents[i])
                    {
                        double val;
                        TryParseDouble(tok, val);
                        row.push_back(val);
                    }
                    values.push_back(row);
                }
            }

            if (values.empty() && error_msg)
                error_msg->append("\nNo values found in " + fileName + "\n");

            return values;
        }

        // =========================================================================
        // FindStreamSegmentByKeyword
        // =========================================================================
        /** @brief 提取文件中KEYWORD与END_KEYWORD之间的行内容
         * 【功能】在fileLines中搜索 keyword和 END_keyword，返回其间的原始行
         * 【物理意义】用于将多目的地来读取版块内容，如OpenFAST BLADE标记内容
         * @param keyword 平块开始关键字
         * @param fileLines 文件行列表
         * @return KEYWORD到END_KEYWORD之间的行列表（不含KEYWORD行本身）
         * 【使用案例】 auto seg = FindStreamSegmentByKeyword("TOWER", lines); */
        std::vector<std::string> FileParser::FindStreamSegmentByKeyword(
            const std::string &keyword,
            const std::vector<std::string> &fileLines)
        {

            std::vector<std::string> segment;
            std::string endKeyword = "END_" + keyword;
            bool readIn = false;

            for (size_t i = 0; i < fileLines.size(); ++i)
            {
                auto tokens = TokenizeLine(fileLines[i]);

                // 检查是否遇到 END_KEYWORD
                for (const auto &tok : tokens)
                {
                    if (tok == endKeyword)
                    {
                        readIn = false;
                        break;
                    }
                }

                // 如果在读取段内, 添加原始行
                if (readIn)
                {
                    segment.push_back(fileLines[i]);
                }

                // 检查是否遇到 KEYWORD (在添加行之后检查, 这样 keyword 行本身不包含在内)
                for (const auto &tok : tokens)
                {
                    if (tok == keyword)
                    {
                        readIn = true;
                        break;
                    }
                }
            }

            return segment;
        }

        // =========================================================================
        // TryParseDouble
        // =========================================================================
        /** @brief 尝试将字符串解析为double
         * 【功能】使用strtod解析，完全匹配才称功
         * @param str 要解析的字符串
         * @param outVal 输出解析结果
         * @return true=解析成功
         * 【使用案例】 double v; if (TryParseDouble("3.14", v)) { ... } */
        bool FileParser::TryParseDouble(const std::string &str, double &outVal)
        {
            if (str.empty())
                return false;
            char *end = nullptr;
            errno = 0;
            outVal = std::strtod(str.c_str(), &end);
            if (errno == ERANGE || end == str.c_str() || *end != '\0')
                return false;
            return true;
        }

        // =========================================================================
        // TryParseInt
        // =========================================================================
        /** @brief 尝试将字符串解析为int
         * 【功能】使用strtol解析，完全匹配才称功
         * @param str 要解析的字符串
         * @param outVal 输出解析结果
         * @return true=解析成功
         * 【使用案例】 int n; if (TryParseInt("42", n)) { ... } */
        bool FileParser::TryParseInt(const std::string &str, int &outVal)
        {
            if (str.empty())
                return false;
            char *end = nullptr;
            errno = 0;
            long val = std::strtol(str.c_str(), &end, 10);
            if (errno == ERANGE || end == str.c_str() || *end != '\0')
                return false;
            outVal = static_cast<int>(val);
            return true;
        }

        // =========================================================================
        // ParseBool
        // =========================================================================
        /** @brief 将字符串解析为bool：支持true/TRUE/True/1
         * @param str 要解析的字符串 */
        bool FileParser::ParseBool(const std::string &str)
        {
            return (str == "true" || str == "TRUE" || str == "True" || str == "1");
        }

        // =========================================================================
        // IsNumeric
        // =========================================================================
        /** @brief 检验字符串是否为合法数字（整数或浮点）
         * 【功能】用正则表达式匹配数字字符串，支持科学计数法
         * @param str 要检验的字符串
         * @return true=是合法数字
         * 【使用案例】 if (IsNumeric(tok)) { ... } */
        bool FileParser::IsNumeric(const std::string &str)
        {
            if (str.empty())
                return false;
            // 使用正则匹配: 可选符号, 数字(含小数点), 可选科学计数法
            static const std::regex numPattern(
                R"(^[+-]?(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?$)");
            return std::regex_match(str, numPattern);
        }
    
}