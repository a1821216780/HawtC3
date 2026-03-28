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

#include <string>
#include <vector>
#include <Eigen/Core>

namespace Qahse::IO::IO
{

    class FileParser
    {
        
        public:
    // -------------------------------------------------------------------------
    // UnifyString: 规范化科学计数法字符串
    // 处理: "1.0 e5" → "1.0e5", 逗号→句点
    // -------------------------------------------------------------------------
    static std::string UnifyString(const std::string &input);

    // -------------------------------------------------------------------------
    // FileContentToStringList: 读取文件内容到字符串列表 (每行一个元素)
    // -------------------------------------------------------------------------
    static std::vector<std::string> FileContentToStringList(const std::string &filename);

    // -------------------------------------------------------------------------
    // TokenizeLine: 将一行按空白符分割为 token 列表
    // 如果 replaceSeparators=true, 还会将逗号和分号替换为空格
    // -------------------------------------------------------------------------
    static std::vector<std::string> TokenizeLine(const std::string &line, bool replaceSeparators = false);

    // -------------------------------------------------------------------------
    // TokenizeFile: 将整个文件按行 tokenize
    // -------------------------------------------------------------------------
    static std::vector<std::vector<std::string>> TokenizeFile(const std::vector<std::string> &fileLines,
                                                       bool replaceSeparators = false);

    // -------------------------------------------------------------------------
    // FindKeywordInFile: 检查关键字是否存在于文件中 (精确 token 匹配)
    // -------------------------------------------------------------------------
    static bool FindKeywordInFile(const std::string &keyword, const std::vector<std::string> &fileLines);

    // -------------------------------------------------------------------------
    // FindValueInFile: 查找 "VALUE KEYWORD" 格式的值
    // 在 tokens[j>=1] 中查找 keyword，返回 tokens[0]
    // @param keyword 要搜索的关键字
    // @param fileLines 文件内容行列表
    // @param error_msg 错误信息附加指针 (可选)
    // @param setmsg 是否写入错误信息
    // @param found 是否找到 (输出参数)
    // @return 关键字所在行的第一个 token (即 VALUE)
    // -------------------------------------------------------------------------
    static std::string FindValueInFile(const std::string &keyword,
                                const std::vector<std::string> &fileLines,
                                std::string *error_msg = nullptr,
                                bool setmsg = true,
                                bool *found = nullptr);

    // -------------------------------------------------------------------------
    // FindLineWithKeyword: 查找包含关键字的行，返回该行的所有 token
    // -------------------------------------------------------------------------
    static  std::vector<std::string> FindLineWithKeyword(const std::string &keyword,
                                                 const std::vector<std::string> &fileLines,
                                                 std::string *error_msg = nullptr,
                                                 bool setmsg = true,
                                                 bool *found = nullptr,
                                                 bool replaceSeparators = false);

    // -------------------------------------------------------------------------
    // FindMatrixInFile: 查找关键字后的 rows×cols 矩阵数据
    // 关键字必须是该行的第一个 token, 后续行为数据行
    // -------------------------------------------------------------------------
    static Eigen::MatrixXd FindMatrixInFile(const std::string &keyword,
                                     const std::vector<std::string> &fileLines,
                                     int rows, int cols,
                                     std::string *error_msg = nullptr,
                                     bool setmsg = true,
                                     bool *found = nullptr);

    // -------------------------------------------------------------------------
    // FindStringDataTable: 查找关键字后的字符串数据表
    // 从关键字下一行开始, 读取所有至少有 cols 个 token 的行
    // 第一个 token 必须可转换为数字
    // -------------------------------------------------------------------------
    static std::vector<std::vector<std::string>> FindStringDataTable(
        const std::string &keyword,
        const std::vector<std::string> &fileLines,
        int cols,
        std::string *error_msg = nullptr,
        bool setmsg = true,
        bool *found = nullptr);

    // -------------------------------------------------------------------------
    // FindNumericDataTable: 查找关键字后的数值数据表
    // 从关键字下一行开始, 读取恰好有 cols 个 double 值的行
    // -------------------------------------------------------------------------
    static std::vector<std::vector<double>> FindNumericDataTable(
        const std::string &keyword,
        const std::vector<std::string> &fileLines,
        int cols,
        std::string *error_msg = nullptr,
        bool setmsg = true,
        bool *found = nullptr);

    // -------------------------------------------------------------------------
    // FindNumericValuesInFile: 提取文件中所有纯数值行 (无须关键字)
    // 每行的所有 token 必须为数字, 且列数 >= minColCount
    // -------------------------------------------------------------------------
    static std::vector<std::vector<double>> FindNumericValuesInFile(
        int minColCount,
        const std::vector<std::string> &fileLines,
        std::string *error_msg = nullptr,
        const std::string &fileName = "");

    // -------------------------------------------------------------------------
    // FindStreamSegmentByKeyword: 提取 KEYWORD 与 END_KEYWORD 之间的行
    // 返回原始行 (非 tokenized)
    // -------------------------------------------------------------------------
    static std::vector<std::string> FindStreamSegmentByKeyword(
        const std::string &keyword,
        const std::vector<std::string> &fileLines);

    // -------------------------------------------------------------------------
    // TryParseDouble: 安全地尝试将字符串转换为 double
    // @return true 如果转换成功
    // -------------------------------------------------------------------------
    static bool TryParseDouble(const std::string &str, double &outVal);

    // -------------------------------------------------------------------------
    // TryParseInt: 安全地尝试将字符串转换为 int
    // -------------------------------------------------------------------------
    static bool TryParseInt(const std::string &str, int &outVal);

    // -------------------------------------------------------------------------
    // ParseBool: 将字符串解析为布尔值 ("true"/"TRUE"/"True"/"1" → true)
    // -------------------------------------------------------------------------
    static bool ParseBool(const std::string &str);

    // -------------------------------------------------------------------------
    // IsNumeric: 判断字符串是否为有效数字
    // -------------------------------------------------------------------------
    static bool IsNumeric(const std::string &str);
    };

} // namespace FileParser
