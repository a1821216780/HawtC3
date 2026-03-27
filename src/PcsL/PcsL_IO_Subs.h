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

#include "PcsL_Types.h"
#include "../IO/IO/StringHelper.h"
#include "../IO/IO/OtherHelper.h"
#include "../IO/IO/CheckError.h"
#include "../IO/Log/LogHelper.h"
#include "FEM/PcsLFEMTypes.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <tuple>
#include <cmath>
#include <iostream>

namespace Qahse::PcsL
{
    using Qahse::IO::IO::CheckError;
    using Qahse::IO::IO::OtherHelper;
    using Qahse::IO::IO::StringHelper;
    using Qahse::IO::Log::LogHelper;

    // ===================================================================
    //  内部辅助：从文件行中按关键字查找行索引
    // ===================================================================

    /**
     * @brief 按关键字在文件行集合中查找第一个匹配行的索引
     * @details 遍历文件行列表，查找包含指定关键字的第一行，返回其零基行索引。
     *          找不到时根据 error 参数决定是否报错，找不到则返回 -1。
     * @param lines    文件的所有行（由 StringHelper::ReadAllLines 读入）
     * @param keyword  要搜索的关键字字符串
     * @param filepath 文件路径（仅用于错误提示）
     * @param error    找不到时是否抛出错误（默认 true）
     * @return 第一个匹配行的零基行索引；找不到且 error=false 时返回 -1
     *
     * @par 使用示例
     * @code
     * auto lines = StringHelper::ReadAllLines("input.pcs");
     * int idx = FindLineIndex(lines, " Bl_length ", "input.pcs");
     * // lines[idx] 即包含叶片长度的行
     * @endcode
     */
    inline int FindLineIndex(const std::vector<std::string> &lines, const std::string &keyword,
                             const std::string &filepath, bool error = true)
    {
        auto idxs = OtherHelper::GetMatchingLineIndexes(lines, keyword, filepath, error); // 获取所有匹配行索引
        if (idxs.empty())
            return -1;  // 未找到返回 -1
        return idxs[0]; // 返回第一个匹配行索引
    }

    /**
     * @brief 按关键字在文件行集合中查找所有匹配行的索引列表
     * @details 返回所有包含指定关键字的行的零基索引列表，适合有多个同名关键字的场景（如多扇区）。
     * @param lines    文件的所有行
     * @param keyword  要搜索的关键字字符串
     * @param filepath 文件路径（仅用于错误提示）
     * @param error    找不到时是否报错（默认 true）
     * @return 所有匹配行的零基索引向量；未找到则为空向量
     *
     * @par 使用示例
     * @code
     * auto idxs = FindAllLineIndexes(lines, "Sect_num", path, false);
     * for (int i : idxs) { /* 处理每个扇区行 *\/ }
     * @endcode
     */
    inline std::vector<int> FindAllLineIndexes(const std::vector<std::string> &lines, const std::string &keyword,
                                               const std::string &filepath, bool error = true)
    {
        return OtherHelper::GetMatchingLineIndexes(lines, keyword, filepath, error); // 代理到 OtherHelper
    }

    // ===================================================================
    //  行解析辅助
    // ===================================================================

    /**
     * @brief 解析一行文本中的第一个有效数值为 double
     * @details 使用 istringstream 跳过前导空白，提取第一个浮点数。
     * @param line 待解析的文本行
     * @return 解析到的 double 值；解析失败则返回 0.0
     *
     * @par 使用示例
     * @code
     * double v = ParseDouble(" 63.0  ! 叶片长度");
     * // v == 63.0
     * @endcode
     */
    inline double ParseDouble(const std::string &line)
    {
        std::istringstream iss(line); // 构建字符串流
        double v = 0;
        iss >> v; // 提取第一个数值
        return v;
    }

    /**
     * @brief 解析一行文本中的第一个有效数值为 int
     * @details 使用 istringstream 提取第一个整数。
     * @param line 待解析的文本行
     * @return 解析到的 int 值；解析失败则返回 0
     *
     * @par 使用示例
     * @code
     * int n = ParseInt(" 20  ! 截面数量");
     * // n == 20
     * @endcode
     */
    inline int ParseInt(const std::string &line)
    {
        std::istringstream iss(line); // 构建字符串流
        int v = 0;
        iss >> v; // 提取第一个整数
        return v;
    }

    /**
     * @brief 解析一行文本中的第一个有效 token 为布尔值
     * @details 忽略大小写，支持 "true"/"false" 和 "1"/"0" 两种格式。
     * @param line 待解析的文本行
     * @return true 当 token 为 "true" 或 "1"；否则返回 false
     *
     * @par 使用示例
     * @code
     * bool b = ParseBool(" True");
     * // b == true
     * @endcode
     */
    inline bool ParseBool(const std::string &line)
    {
        std::string token;
        std::istringstream iss(line);
        iss >> token; // 提取第一个 token
        // 统一转为小写便于比较
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        return (token == "true" || token == "1"); // 支持字符串和数字两种布尔格式
    }

    /**
     * @brief 解析一行文本中的第一个带引号的字符串（去掉双引号/单引号）
     * @details 查找行中第一对相同的引号（'或"），提取其中内容。
     *          若无引号，则提取第一个空白分隔的 token。
     * @param line 待解析的文本行
     * @return 去引号后的字符串内容
     *
     * @par 使用示例
     * @code
     * std::string s = ParseQuotedString(" MaterialsPath  'data/materials.inp'");
     * // s == "data/materials.inp"
     * @endcode
     */
    inline std::string ParseQuotedString(const std::string &line)
    {
        // 查找第一个引号
        auto p1 = line.find_first_of("\"'");
        if (p1 == std::string::npos)
        {
            // 没有引号，取第一个 token
            std::istringstream iss(line);
            std::string t;
            iss >> t;
            return t;
        }
        char quote = line[p1];              // 记录引号字符类型（'或"）
        auto p2 = line.find(quote, p1 + 1); // 查找配对的结束引号
        if (p2 == std::string::npos)
            p2 = line.size();                    // 无结束引号则取到行末
        return line.substr(p1 + 1, p2 - p1 - 1); // 返回引号内内容
    }

    /**
     * @brief 解析一行中以空格或逗号分隔的 double 数组
     * @details 支持 "0.2,0.9" 或 "0.2 0.9" 两种格式，也支持混合格式。
     *          解析遇到非数字内容时提前终止并返回已解析结果。
     * @param line 待解析的文本行
     * @return 解析到的 double 数组；若无有效数值则返回空向量
     *
     * @par 使用示例
     * @code
     * auto v = ParseDoubleArray(" 0.2,0.5,0.8,1.0");
     * // v == {0.2, 0.5, 0.8, 1.0}
     * @endcode
     */
    inline std::vector<double> ParseDoubleArray(const std::string &line)
    {
        std::vector<double> result;
        std::istringstream iss(line);
        std::string token;
        while (iss >> token)
        { // 按空格分割
            std::istringstream ts(token);
            std::string sub;
            bool anyParsed = false;
            while (std::getline(ts, sub, ','))
            { // 按逗号再分割
                if (!sub.empty())
                {
                    try
                    {
                        result.push_back(std::stod(sub)); // 解析为 double
                        anyParsed = true;
                    }
                    catch (...)
                    {
                        return result; // 遇非数字内容提前终止
                    }
                }
            }
            if (!anyParsed)
                break; // 整个 token 无法解析，终止
        }
        return result;
    }

    // ===================================================================
    //  通过关键字查找并解析
    // ===================================================================

    /**
     * @brief 在文件行中按关键字找到对应行并解析为 double
     * @param lines      文件所有行
     * @param keyword    关键字字符串
     * @param filepath   文件路径（用于错误提示）
     * @param defaultVal 找不到时的默认返回值（默认 0.0）
     * @param error      找不到时是否报错（默认 true）
     * @return 找到并解析的 double 值，找不到时返回 defaultVal
     *
     * @par 使用示例
     * @code
     * double len = FindAndParseDouble(d, " Bl_length ", path);
     * @endcode
     */
    inline double FindAndParseDouble(const std::vector<std::string> &lines, const std::string &keyword,
                                     const std::string &filepath, double defaultVal = 0.0, bool error = true)
    {
        int idx = FindLineIndex(lines, keyword, filepath, error); // 查找关键字所在行
        if (idx < 0)
            return defaultVal;          // 未找到返回默认值
        return ParseDouble(lines[idx]); // 解析该行第一个数值
    }

    /**
     * @brief 在文件行中按关键字找到对应行并解析为 int
     * @param lines      文件所有行
     * @param keyword    关键字字符串
     * @param filepath   文件路径（用于错误提示）
     * @param defaultVal 找不到时的默认返回值（默认 0）
     * @param error      找不到时是否报错（默认 true）
     * @return 找到并解析的 int 值，找不到时返回 defaultVal
     *
     * @par 使用示例
     * @code
     * int n = FindAndParseInt(d, " N_materials ", path);
     * @endcode
     */
    inline int FindAndParseInt(const std::vector<std::string> &lines, const std::string &keyword,
                               const std::string &filepath, int defaultVal = 0, bool error = true)
    {
        int idx = FindLineIndex(lines, keyword, filepath, error); // 查找关键字所在行
        if (idx < 0)
            return defaultVal;       // 未找到返回默认值
        return ParseInt(lines[idx]); // 解析该行第一个整数
    }

    /**
     * @brief 在文件行中按关键字找到对应行并解析为 bool
     * @param lines      文件所有行
     * @param keyword    关键字字符串
     * @param filepath   文件路径（用于错误提示）
     * @param defaultVal 找不到时的默认返回值（默认 false）
     * @param error      找不到时是否报错（默认 true）
     * @return 找到并解析的 bool 值，找不到时返回 defaultVal
     *
     * @par 使用示例
     * @code
     * bool fem = FindAndParseBool(d, " FEMSove ", path, false, false);
     * @endcode
     */
    inline bool FindAndParseBool(const std::vector<std::string> &lines, const std::string &keyword,
                                 const std::string &filepath, bool defaultVal = false, bool error = true)
    {
        int idx = FindLineIndex(lines, keyword, filepath, error); // 查找关键字所在行
        if (idx < 0)
            return defaultVal;        // 未找到返回默认值
        return ParseBool(lines[idx]); // 解析该行布尔值
    }

    /**
     * @brief 在文件行中按关键字找到对应行并解析为带引号的字符串
     * @param lines    文件所有行
     * @param keyword  关键字字符串
     * @param filepath 文件路径（用于错误提示）
     * @param error    找不到时是否报错（默认 true）
     * @return 解析到的字符串内容（去掉引号）；找不到时返回空字符串
     *
     * @par 使用示例
     * @code
     * std::string matPath = FindAndParseString(d, " MaterialsPath ", path, false);
     * @endcode
     */
    inline std::string FindAndParseString(const std::vector<std::string> &lines, const std::string &keyword,
                                          const std::string &filepath, bool error = true)
    {
        int idx = FindLineIndex(lines, keyword, filepath, error); // 查找关键字所在行
        if (idx < 0)
            return "";                        // 未找到返回空串
        return ParseQuotedString(lines[idx]); // 提取引号内字符串
    }

    // ===================================================================
    //  材料文件读取
    // ===================================================================

    /**
     * @brief 读取复合材料属性文件（materials.inp），返回材料列表
     * @details 文件格式：前 3 行为标题行，第 4 行起每行一个材料。
     *          FEM模式：Id E1 E2 E3 G12 G13 G23 Nu12 Nu13 Nu23 Density Name [S1t S2t S3t S1c S2c S3c S12 S23 S13]
     *          CLT模式：Id E1 E2 G12 Nu12 Density Name
     *          读取完成后按 Id 升序排序。
     * @param matPath 材料文件路径
     * @param count   要读取的材料数量
     * @param isFEM   是否为 FEM 模式（true=读取全部9个弹性常数，false=CLT模式仅读5个）
     * @return 排序后的材料属性列表
     *
     * @par 使用示例
     * @code
     * auto mats = ReadMaterials("data/materials.inp", 3, true);
     * // mats[0].E1 为第1号材料的纤维方向弹性模量
     * @endcode
     */
    inline std::vector<CompositeMaterial> ReadMaterials(const std::string &matPath, int count, bool isFEM)
    {
        CheckError::Filexists(matPath);                   // 检查文件存在
        auto lines = StringHelper::ReadAllLines(matPath); // 读取文件所有行

        std::vector<CompositeMaterial> mats(count); // 预分配材料列表
        // 前 3 行为标题/注释行，第 4 行（索引3）起为数据
        for (int i = 0; i < count; i++)
        {
            int row = 3 + i; // 第 i 个材料的行号（0-based）
            if (row >= (int)lines.size())
            {
                LogHelper::ErrorLog("材料文件中材料数量不足: " + matPath, "", "", 20, "ReadMaterials");
                break;
            }
            std::istringstream iss(lines[row]);
            CompositeMaterial &m = mats[i];

            if (isFEM)
            {
                // FEM 完整弹性常数格式：Id E1 E2 E3 G12 G13 G23 Nu12 Nu13 Nu23 Density Name [强度参数...]
                iss >> m.Id >> m.E1 >> m.E2 >> m.E3 >> m.G12 >> m.G13 >> m.G23 >> m.Nu12 >> m.Nu13 >> m.Nu23 >> m.Density;
                std::string name;
                if (iss >> name)
                    m.Name = name; // 材料名称（可选）
                // 可选的失效强度参数（台 Tsai-Wu 等准则使用）
                iss >> m.S1t >> m.S2t >> m.S3t >> m.S1c >> m.S2c >> m.S3c >> m.S12s >> m.S23s >> m.S13s;
            }
            else
            {
                // CLT 模式（经典层合板理论），仅需5个弹性常数
                iss >> m.Id >> m.E1 >> m.E2 >> m.G12 >> m.Nu12 >> m.Density;
                std::string name;
                if (iss >> name)
                    m.Name = name;
            }
        }
        // 按材料 ID 升序排序，确保后续以 Id 索引时一致
        std::sort(mats.begin(), mats.end(), [](const CompositeMaterial &a, const CompositeMaterial &b)
                  { return a.Id < b.Id; });
        return mats;
    }

    // ===================================================================
    //  解析数据矩阵块（节点或单元）
    // ===================================================================

    /**
     * @brief 从文件行中读取固定尺寸的数值矩阵块
     * @details 从 startLine 行起，读取 numRows 行，每行读取 numCols 个 double 数值，
     *          填入 Eigen::MatrixXd 并返回。空值默认为 0。
     * @param lines     文件的所有行
     * @param startLine 数据起始行号（0-based）
     * @param numRows   要读取的行数（矩阵行数）
     * @param numCols   每行要读取的列数（矩阵列数）
     * @return 尺寸为 (numRows × numCols) 的矩阵
     *
     * @par 使用示例
     * @code
     * // 读取 100 个节点，每节点 [id, x, y]，共 3 列
     * auto nodes = ReadMatrixBlock(lines, nodeStart, 100, 3);
     * double x = nodes(0, 1); // 第1个节点 x 坐标
     * @endcode
     */
    inline Eigen::MatrixXd ReadMatrixBlock(const std::vector<std::string> &lines, int startLine, int numRows, int numCols)
    {
        Eigen::MatrixXd mat(numRows, numCols); // 初始化矩阵
        for (int i = 0; i < numRows; i++)
        {
            std::istringstream iss(lines[startLine + i]); // 逐行解析
            for (int j = 0; j < numCols; j++)
            {
                double v = 0;
                iss >> v; // 读取当前列数值
                mat(i, j) = v;
            }
        }
        return mat;
    }

    /**
     * @brief 从文件行中自动检测列数，读取指定行数的数值矩阵块
     * @details 以第一行的 token 数量自动推断列数，无需预先指定。
     *          适用于单元连通性矩阵（Q4/Q8/T6列数不同）等动态格式。
     * @param lines     文件的所有行
     * @param startLine 数据起始行号（0-based）
     * @param numRows   要读取的行数
     * @return 自动推断列数的矩阵，尺寸为 (numRows × 检测到的列数)
     *
     * @par 使用示例
     * @code
     * // 单元矩阵列数由单元类型决定，无需预先指定
     * auto elems = ReadMatrixBlockAuto(lines, elemStart, 200);
     * int ncol = (int)elems.cols(); // Q4=7列, Q8=11列
     * @endcode
     */
    inline Eigen::MatrixXd ReadMatrixBlockAuto(const std::vector<std::string> &lines, int startLine, int numRows)
    {
        // 探测第一行以确定列数
        std::istringstream probe(lines[startLine]);
        std::vector<double> vals;
        double v;
        while (probe >> v)
            vals.push_back(v);       // 读取第一行所有数值
        int ncol = (int)vals.size(); // 列数由第一行决定
        Eigen::MatrixXd mat(numRows, ncol);
        for (int j = 0; j < ncol; j++)
            mat(0, j) = vals[j]; // 填入第一行数据
        for (int i = 1; i < numRows; i++)
        { // 读取剩余行
            std::istringstream iss(lines[startLine + i]);
            for (int j = 0; j < ncol; j++)
            {
                double val = 0;
                iss >> val;
                mat(i, j) = val;
            }
        }
        return mat;
    }

    // ===================================================================
    //  翼型文件读取
    // ===================================================================

    /**
     * @brief 读取翼型外形数据文件，返回 AirfoilShape 结构体
     * @details 文件格式：第1行为节点数；第2-4行为标题；第5行起为 (x, y) 坐标对，
     *          坐标以弦长归一化（0=前缘，1=后缘）。
     *          读取后自动找到 x 最大值处（后缘）将轮廓分为上/下翼面，
     *          下翼面坐标会被反转以使前缘在起始端。
     * @param path 翼型文件路径（.dat 格式）
     * @return 包含全轮廓和上/下翼面坐标的 AirfoilShape 结构体
     *
     * @par 使用示例
     * @code
     * AirfoilShape af = ReadAirfoilFile("airfoils/NACA64.dat");
     * // af.X, af.Y: 全轮廓；af.Xu/Yu: 上翼面；af.Xl/Yl: 下翼面
     * @endcode
     */
    inline AirfoilShape ReadAirfoilFile(const std::string &path)
    {
        CheckError::Filexists(path);                   // 检查文件存在
        auto lines = StringHelper::ReadAllLines(path); // 读取所有行
        AirfoilShape af;
        af.N_af_nodes = ParseInt(lines[0]); // 第1行为节点总数
        // 跳过前4行（节点数+3行标题），从第5行起读取坐标
        auto mat = ReadMatrixBlock(lines, 4, af.N_af_nodes, 2);
        af.X = mat.col(0); // 所有节点 x 坐标（归一化弦向）
        af.Y = mat.col(1); // 所有节点 y 坐标（归一化厚度）

        // 找到后缘（x 最大）作为上/下翼面分界点
        int splitIdx = 0;              // 分界点索引
        double xmax = af.X.maxCoeff(); // 后缘 x 坐标（通常=1.0）
        for (int i = 0; i < af.N_af_nodes; i++)
        {
            if (af.X(i) == xmax)
            {
                splitIdx = i;
                break;
            } // 找到后缘索引
        }

        // 上翼面：从前缘（索引0）到后缘（splitIdx）
        af.Xu = af.X.head(splitIdx + 1);
        af.Yu = af.Y.head(splitIdx + 1);
        // 下翼面：从后缘到文件末尾，反转使前缘在起始端
        int nLow = af.N_af_nodes - splitIdx;
        af.Xl = af.X.tail(nLow).reverse();
        af.Yl = af.Y.tail(nLow).reverse();
        // 注：确保下翼面以 (0,0) 开头并以 (1,0) 结尾（前缘至后缘顺序）
        return af;
    }

    // ===================================================================
    //  铺层文件读取
    // ===================================================================

    /**
     * @brief 读取截面铺层定义文件，解析上翼面、下翼面和腹板的铺层配置
     * @details 铺层文件结构：
     *          - 上翼面：N_scts(1) 给出扇区数，xsec_node 给出分区点坐标，
     *            各 Sect_num 块给出每个扇区的铺层列表
     *          - 下翼面：同上，关键字为 N_scts(2)
     *          - 腹板：关键字 web_num/wlam_num，数量由 Nweb 决定
     *          每条铺层行格式：LayerId PlyCount Thickness FiberOrientation MaterialId [MatDisp]
     * @param path 铺层文件路径
     * @param Nweb 腹板数量（0表示无腹板）
     * @return (top, low, web) 三个面配置元组
     *
     * @par 使用示例
     * @code
     * auto [top, low, web] = ReadLayupFile("layup/r10.inp", 2);
     * // top.Sector[0].Laminae[0].Thickness: 第1扇区第1层厚度
     * @endcode
     */
    inline std::tuple<SurfaceConfiguration, SurfaceConfiguration, SurfaceConfiguration>
    ReadLayupFile(const std::string &path, int Nweb)
    {
        CheckError::Filexists(path);                   // 检查文件存在
        auto lines = StringHelper::ReadAllLines(path); // 读取文件所有行
        // 局部 lambda：快速查找所有含关键字的行索引
        auto findAll = [&](const std::string &kw)
        {
            return FindAllLineIndexes(lines, kw, path, false);
        };

        // ---- 上翼面铺层配置 ----
        SurfaceConfiguration top;
        {
            int idx = FindLineIndex(lines, "N_scts(1)", path); // 查找上翼面扇区数关键字
            top.N_scts = ParseInt(lines[idx]);                 // 解析上翼面扇区数
        }
        {
            auto xnodes = FindAllLineIndexes(lines, "xsec_node", path); // 查找所有分区点行
            if (!xnodes.empty())
            {
                // 第一个 xsec_node 的下一行为上翼面各扇区分界坐标
                top.xsec_node = ParseDoubleArray(lines[xnodes[0] + 1]);
            }
        }
        auto sectIdxs = findAll("Sect_num"); // 所有扇区定义块的行索引
        top.Sector.resize(top.N_scts);       // 预分配上翼面扇区列表
        for (int i = 0; i < top.N_scts; i++)
        {
            auto &sec = top.Sector[i];
            // Sect_num 行的下一行包含该扇区编号和铺层数
            std::istringstream iss(lines[sectIdxs[i] + 1]);
            iss >> sec.Sect_num >> sec.N_laminas; // 扇区编号和铺层数
            sec.Laminae.resize(sec.N_laminas);    // 预分配铺层列表
            for (int j = 0; j < sec.N_laminas; j++)
            {
                auto &lam = sec.Laminae[j];
                // 铺层数据偏移6行（头部标题格式固定）
                std::istringstream lss(lines[sectIdxs[i] + 6 + j]);
                lss >> lam.LayerId >> lam.PlyCount >> lam.Thickness >> lam.FiberOrientation >> lam.MaterialId;
                std::string disp;
                if (lss >> disp)
                    lam.MatDisp = disp; // 可选材料显示名
            }
        }

        // ---- 下翼面铺层配置 ----
        SurfaceConfiguration low;
        {
            int idx = FindLineIndex(lines, "N_scts(2)", path); // 下翼面扇区数
            low.N_scts = ParseInt(lines[idx]);
        }
        {
            auto xnodes = FindAllLineIndexes(lines, "xsec_node", path);
            if (xnodes.size() > 1)
            {
                // 第二个 xsec_node 的下一行为下翼面分界坐标
                low.xsec_node = ParseDoubleArray(lines[xnodes[1] + 1]);
            }
        }
        low.Sector.resize(low.N_scts);
        for (int i = 0; i < low.N_scts; i++)
        {
            auto &sec = low.Sector[i];
            int si = top.N_scts + i; // 下翼面扇区在全局 sectIdxs 中的偏移
            std::istringstream iss(lines[sectIdxs[si] + 1]);
            iss >> sec.Sect_num >> sec.N_laminas;
            sec.Laminae.resize(sec.N_laminas);
            for (int j = 0; j < sec.N_laminas; j++)
            {
                auto &lam = sec.Laminae[j];
                std::istringstream lss(lines[sectIdxs[si] + 6 + j]);
                lss >> lam.LayerId >> lam.PlyCount >> lam.Thickness >> lam.FiberOrientation >> lam.MaterialId;
                std::string disp;
                if (lss >> disp)
                    lam.MatDisp = disp;
            }
        }

        // ---- 腹板铺层配置 ----
        SurfaceConfiguration web;
        web.N_scts = Nweb;       // 腹板数量即为扇区数
        web.Sector.resize(Nweb); // 预分配腹板扇区列表
        if (Nweb > 0)
        {
            auto webIdxs = findAll("web_num");   // 腹板定义行索引
            auto wlamIdxs = findAll("wlam_num"); // 腹板铺层数据行索引
            for (int i = 0; i < Nweb; i++)
            {
                auto &sec = web.Sector[i];
                std::istringstream iss(lines[webIdxs[i] + 1]);
                iss >> sec.Sect_num >> sec.N_laminas; // 腹板编号和铺层数
                sec.Laminae.resize(sec.N_laminas);
                for (int j = 0; j < sec.N_laminas; j++)
                {
                    auto &lam = sec.Laminae[j];
                    std::istringstream lss(lines[wlamIdxs[i] + j + 1]);
                    lss >> lam.LayerId >> lam.PlyCount >> lam.Thickness >> lam.FiberOrientation >> lam.MaterialId;
                    std::string disp;
                    if (lss >> disp)
                        lam.MatDisp = disp;
                }
            }
        }

        return {top, low, web};
    }

    // ===================================================================
    //  路径解析辅助
    // ===================================================================

    /**
     * @brief 将相对路径解析为绝对路径（相对于基文件所在目录）
     * @details 去除 relPath 中的引号和首尾空白后，拼接到 basePath 的父目录下，
     *          使用 std::filesystem::absolute 规范化为绝对路径。
     *          适用于主文件中引用相对路径的翼型、铺层、材料等子文件。
     * @param basePath 基准文件的路径（取其所在目录）
     * @param relPath  相对路径字符串（可带引号和空白）
     * @return 规范化后的绝对路径字符串
     *
     * @par 使用示例
     * @code
     * std::string abs = ResolvePath("cases/blade.pcs", "'airfoils/NACA64.dat'");
     * // abs == "C:\\cases\\airfoils\\NACA64.dat"（Windows 下）
     * @endcode
     */
    inline std::string ResolvePath(const std::string &basePath, const std::string &relPath)
    {
        namespace fs = std::filesystem;
        fs::path base = fs::path(basePath).parent_path(); // 取主文件所在目录
        // 去掉路径字符串中的单引号和双引号
        std::string cleaned = relPath;
        cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), '\''), cleaned.end());
        cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), '"'), cleaned.end());
        // 去掉首尾空白（空格和制表符）
        while (!cleaned.empty() && (cleaned.front() == ' ' || cleaned.front() == '\t'))
            cleaned.erase(cleaned.begin());
        while (!cleaned.empty() && (cleaned.back() == ' ' || cleaned.back() == '\t'))
            cleaned.pop_back();

        fs::path resolved = fs::absolute(base / cleaned); // 拼接并规范化为绝对路径
        return resolved.string();
    }

    // ===================================================================
    //  主文件读取 ReadPcsL_MainFile
    // ===================================================================

    /**
     * @brief 读取 PcsL 截面分析主控文件（.pcs），解析所有输入参数
     * @details 主文件采用关键字-数值格式（类 Fortran namelist 风格），本函数逐步解析：
     *          1. 基本信息（叶片长度、截面数、材料数等）
     *          2. 读取材料文件（ReadMaterials）
     *          3. 应力分析参数（CalStress 模式下）
     *          4. FEM 求解参数（FEMSolve 模式下）
     *          5. 截面与腹板定义（非 SelfMesh 模式下，含翼型和铺层文件读取）
     *          6. 自定义网格（SelfMesh 模式下，读取节点和单元矩阵）
     * @param path .pcs 文件的绝对路径
     * @return 解析完成的 PcsLInput 结构体
     * @throws std::runtime_error 文件不存在或关键参数缺失时抛出
     *
     * @par 使用示例
     * @code
     * PcsLInput pre = ReadPcsL_MainFile("Demo/NREL_5MW/section.pcs");
     * std::cout << pre.BladeLength << std::endl;  // 输出叶片长度
     * @endcode
     */
    inline PcsLInput ReadPcsL_MainFile(const std::string &path)
    {
        CheckError::Filexists(path); // 检查主文件存在
        PcsLInput pre;
        pre.path = path;                             // 记录文件路径
        pre.data = StringHelper::ReadAllLines(path); // 读入所有行（缓存，便于多次查找）
        const auto &d = pre.data;

        // ---- 解析基本信息 ----
        pre.BladeLength = FindAndParseDouble(d, " Bl_length ", path);              // 叶片长度
        pre.SectionCount = FindAndParseInt(d, " N_sections ", path, 0, false);     // 截面数量（可选）
        pre.MaterialTypeCount = FindAndParseInt(d, " N_materials ", path);         // 材料数量
        pre.OutputFormat = FindAndParseInt(d, " Out_format ", path, 3);            // 输出格式（默认3=双格式）
        pre.UseTabDelimiter = FindAndParseBool(d, " TabDelim ", path, true);       // 是否使用Tab分隔符
        pre.FEMSolve = FindAndParseBool(d, " FEMSove ", path, false, false);       // 是否启用FEM求解
        pre.SelfMesh = FindAndParseBool(d, " SelfMesh ", path, false, false);      // 是否使用自定义网格
        pre.CalStress = FindAndParseBool(d, " CalStress ", path, false, false);    // 是否计算应力
        pre.MaterialsPath = FindAndParseString(d, " MaterialsPath ", path, false); // 材料文件路径（相对或绝对）

        // 将材料路径解析为绝对路径（相对于主文件目录）
        if (!pre.MaterialsPath.empty())
        {
            pre.MaterialsPath = ResolvePath(path, pre.MaterialsPath);
        }

        // ---- 读取材料属性文件 ----
        pre.Material = ReadMaterials(pre.MaterialsPath, pre.MaterialTypeCount, pre.FEMSolve);

        // ---- 输出目录路径 ----
        pre.SumPath = FindAndParseString(d, " SumPath ", path, false); // 汇总输出目录
        if (!pre.SumPath.empty())
        {
            pre.SumPath = ResolvePath(path, pre.SumPath);     // 转为绝对路径
            std::filesystem::create_directories(pre.SumPath); // 确保目录存在
        }

        // ---- 应力/应变分析参数（CalStress=true 时才解析）----
        if (pre.CalStress)
        {
            pre.StrainMeshFile = FindAndParseString(d, " StrainMeshFile ", path, false); // 应变网格文件
            if (!pre.StrainMeshFile.empty())
                pre.StrainMeshFile = ResolvePath(path, pre.StrainMeshFile);
            pre.StrainCache = FindAndParseString(d, " StrainCache ", path, false); // 应变缓存文件
            if (!pre.StrainCache.empty())
                pre.StrainCache = ResolvePath(path, pre.StrainCache);
            pre.CalFailure = FindAndParseBool(d, " CalFailure ", path, false, false); // 是否进行失效分析
            if (pre.CalFailure)
            {
                // 读取失效准则编号（如 1=MaxStress, 2=Tsai-Wu）
                pre.FailureCriterion = {FindAndParseInt(d, " FailureCriterion ", path, 0, false)};
            }
            pre.ExtForceNum = FindAndParseInt(d, " ExtForceNum ", path, 0, false); // 外部截面力数量
            if (pre.ExtForceNum > 0)
            {
                // 读取外部截面力表（每行6个分量：Fx Fy Fz Mx My Mz）
                int idx = FindLineIndex(d, "!BeginEXTFORCE", path, false);
                if (idx >= 0)
                {
                    pre.ExtForce = ReadMatrixBlock(d, idx + 1, pre.ExtForceNum, 6);
                }
            }
        }

        // ---- FEM 求解参数（FEMSolve=true 时才解析）----
        if (pre.FEMSolve)
        {
            pre.Shear_Center = FindAndParseBool(d, " Shear_Center ", path, true, false); // 是否计算剪切中心
            int et = FindAndParseInt(d, " ElememtType ", path, 0, false);                // 单元类型编号（0=Q4,1=Q8,2=Q8R,3=T6）
            switch (et)
            {
            case 1:
                pre.ElememtType = PcsLElementType::Q8;
                break; // 8节点完全积分四边形
            case 2:
                pre.ElememtType = PcsLElementType::Q8R;
                break; // 8节点减缩积分四边形
            case 3:
                pre.ElememtType = PcsLElementType::T6;
                break; // 6节点二次三角形
            default:
                pre.ElememtType = PcsLElementType::Q4;
                break; // 默认4节点四边形
            }
            pre.VTKShow = FindAndParseBool(d, " VTKShow ", path, false, false);     // 是否输出VTK可视化文件
            pre.SaveCatch = FindAndParseBool(d, " SaveCatch ", path, false, false); // 是否保存中间计算缓存

            if (!pre.SelfMesh)
            {
                // 网格生成参数（程序自动生成网格时有效）
                pre.MeshWeb = FindAndParseInt(d, " MeshWeb ", path, 4, false);          // 腹板厚度方向网格层数
                pre.SurMeshExpT = FindAndParseInt(d, " SurMeshExpT ", path, -1, false); // 上翼面网格加密系数
                pre.SurMeshExpC = FindAndParseInt(d, " SurMeshExpC ", path, -1, false); // 下翼面网格加密系数
                {
                    // 加密位置的归一化弦向坐标列表
                    int idx = FindLineIndex(d, " SurMeshExpCNorm ", path, false);
                    if (idx >= 0)
                        pre.SurMeshExpCNorm = ParseDoubleArray(d[idx]);
                }
                pre.OutMeshfFile = FindAndParseBool(d, " OutMeshfFile ", path, true, false);                   // 是否输出网格文件
                pre.ConsidertwistAndpitch = FindAndParseBool(d, " ConsidertwistAndpitch ", path, true, false); // 是否考虑扭角和预倾角
            }
        }

        // ---- 非 SelfMesh 模式：读取截面定义与腹板结构 ----
        if (!pre.SelfMesh)
        {
            pre.Nweb = FindAndParseInt(d, " Nweb ", path, 0, false);           // 腹板数量
            pre.Ib_sp_stn = FindAndParseInt(d, " Ib_sp_stn ", path, 1, false); // 腹板内侧起始截面编号（1基）
            pre.Ob_sp_stn = FindAndParseInt(d, " Ob_sp_stn ", path, 1, false); // 腹板外侧终止截面编号（1基）

            // 读取腹板结构定义（每条腹板：WebId 内侧弦向坐标 外侧弦向坐标）
            if (pre.Nweb > 0)
            {
                auto webIdxs = FindAllLineIndexes(d, "Web_num", path, false); // 查找所有腹板定义行
                pre.Webs.resize(pre.Nweb);
                for (int i = 0; i < pre.Nweb && i < (int)webIdxs.size(); i++)
                {
                    std::istringstream iss(d[webIdxs[i] + 1]);
                    iss >> pre.Webs[i].WebId >> pre.Webs[i].InnerChordPosition >> pre.Webs[i].OuterChordPosition;
                }
            }

            // 截面
            if (pre.SectionCount > 0)
            {
                auto spanIdxs = FindAllLineIndexes(d, "Span_loc", path, false);
                pre.Sections.resize(pre.SectionCount);
                for (int i = 0; i < pre.SectionCount; i++)
                {
                    auto &sec = pre.Sections[i];
                    int lineIdx = spanIdxs[0] + 3 + i; // 跳过标题行
                    // 跳过空行
                    while (lineIdx < (int)d.size() && StringHelper::Trim(d[lineIdx]).empty())
                        lineIdx++;
                    if (lineIdx >= (int)d.size())
                        break;

                    std::istringstream iss(d[lineIdx]);
                    std::string afFile, layFile;
                    iss >> sec.SpanLocation >> sec.AerodynamicCenter >> sec.ChordLength >> sec.TwistAngle;

                    // 读取翼型和铺层文件路径
                    std::string rest;
                    std::getline(iss, rest);
                    // 提取两个被引号包裹的路径
                    std::vector<std::string> paths;
                    size_t pos = 0;
                    while (pos < rest.size() && paths.size() < 2)
                    {
                        auto p1 = rest.find_first_of("'\"", pos);
                        if (p1 == std::string::npos)
                            break;
                        char q = rest[p1];
                        auto p2 = rest.find(q, p1 + 1);
                        if (p2 == std::string::npos)
                            break;
                        paths.push_back(rest.substr(p1 + 1, p2 - p1 - 1));
                        pos = p2 + 1;
                    }
                    if (paths.size() >= 1)
                        sec.AirfoilFile = ResolvePath(path, paths[0]);
                    if (paths.size() >= 2)
                        sec.LayupFile = ResolvePath(path, paths[1]);

                    sec.SpanLocation *= pre.BladeLength;

                    // 读取翼型
                    if (!sec.AirfoilFile.empty() && std::filesystem::exists(sec.AirfoilFile))
                    {
                        sec.Airfoil = ReadAirfoilFile(sec.AirfoilFile);
                    }
                    // 读取铺层
                    if (!sec.LayupFile.empty() && std::filesystem::exists(sec.LayupFile))
                    {
                        int nwebForSection = 0;
                        if (i >= pre.Ib_sp_stn - 1 && i <= pre.Ob_sp_stn - 1)
                        {
                            nwebForSection = pre.Nweb;
                        }
                        std::tie(sec.TopConfiguration, sec.LowConfiguration, sec.WebConfiguration) =
                            ReadLayupFile(sec.LayupFile, nwebForSection);
                        if (nwebForSection > 0)
                        {
                            sec.WebConfiguration.xsec_node.resize(pre.Nweb);
                            for (int w = 0; w < pre.Nweb; w++)
                            {
                                sec.WebConfiguration.xsec_node[w] = pre.Webs[w].InnerChordPosition;
                            }
                        }
                    }
                }
            }
        }

        // ---- SelfMesh 自定义网格 ----
        if (pre.SelfMesh)
        {
            // 检查是否有外部网格文件
            int meshPathIdx = FindLineIndex(d, " FEMMeshPath ", path, false);
            std::vector<std::string> meshLines;
            if (meshPathIdx >= 0)
            {
                pre.FEMMeshPath = ResolvePath(path, ParseQuotedString(d[meshPathIdx]));
                if (std::filesystem::exists(pre.FEMMeshPath))
                {
                    meshLines = StringHelper::ReadAllLines(pre.FEMMeshPath);
                }
                else
                {
                    LogHelper::ErrorLog("网格文件不存在: " + pre.FEMMeshPath, "", "", 20, "ReadPcsL_MainFile");
                }
            }
            else
            {
                pre.FEMMeshPath = path;
                meshLines = d; // 使用主文件
            }

            pre.FEMNodeNum = FindAndParseInt(meshLines, " FEMNodeNum ", pre.FEMMeshPath);
            pre.FEMElementNum = FindAndParseInt(meshLines, " FEMElementNum ", pre.FEMMeshPath);

            // 读取节点
            int nodeStart = FindLineIndex(meshLines, "!BeginNode", pre.FEMMeshPath) + 1;
            pre.FEMNodeMatrix = ReadMatrixBlock(meshLines, nodeStart, pre.FEMNodeNum, 3);

            // 读取单元
            int elemStart = FindLineIndex(meshLines, "!BeginElement", pre.FEMMeshPath) + 1;
            pre.FEMElementMatrix = ReadMatrixBlockAuto(meshLines, elemStart, pre.FEMElementNum);
        }

        return pre;
    }

    // ===================================================================
    //  PcsLInput -> FEM::PcsLUtils 转换
    // ===================================================================

    /**
     * @brief 将 PcsLInput（自定义网格模式）转换为 FEM 求解器所需的 PcsLUtils 数据结构
     * @details 将用户输入的节点矩阵、单元矩阵和材料列表，转换为 FEM 求解器内部格式：
     *          - 节点坐标表 nl_2d (N×3: [id, x, y])
     *          - 单元连通性表 el_2d (1-based 节点编号)
     *          - 材料属性矩阵 matprops (nmat×10)
     *          - 单元材料/纤维角属性 emat
     *          - 单元类型枚举数组 etype
     *          - 各类型单元信息表 elemInfo
     *          当电矩阵列数与声明的单元类型不一致时（如11列但声明Q4），以列数为准自动纠正。
     * @param pre 已解析的 PcsLInput 结构体（SelfMesh=true 模式）
     * @return 填充好的 FEM::PcsLUtils 结构体
     *
     * @par 使用示例
     * @code
     * PcsLInput pre = ReadPcsL_MainFile("section.pcs");
     * FEM::PcsLUtils u = ConvertToFEMUtils(pre);
     * // u.nn_2d == pre.FEMNodeNum
     * @endcode
     */
    inline FEM::PcsLUtils ConvertToFEMUtils(const PcsLInput &pre)
    {
        FEM::PcsLUtils u;

        u.nn_2d = pre.FEMNodeNum;       // 节点总数
        u.ne_2d = pre.FEMElementNum;    // 单元总数
        u.nmat = pre.MaterialTypeCount; // 材料总数

        // 节点坐标表：列为 [id, x, y]，行为节点（0-based）
        u.nl_2d = pre.FEMNodeMatrix;

        // 材料属性矩阵：每行10个弹性常数，顺序为 E1 E2 E3 G12 G13 G23 nu12 nu13 nu23 rho
        u.matprops.resize(u.nmat, 10);
        for (int i = 0; i < u.nmat; i++)
        {
            const auto &m = pre.Material[i];
            u.matprops(i, 0) = m.E1;      // 纤维方向弹性模量
            u.matprops(i, 1) = m.E2;      // 横向弹性模量
            u.matprops(i, 2) = m.E3;      // 厚度方向弹性模量
            u.matprops(i, 3) = m.G12;     // 1-2面剪切模量
            u.matprops(i, 4) = m.G13;     // 1-3面剪切模量
            u.matprops(i, 5) = m.G23;     // 2-3面剪切模量
            u.matprops(i, 6) = m.Nu12;    // 泊松比 nu12
            u.matprops(i, 7) = m.Nu13;    // 泊松比 nu13
            u.matprops(i, 8) = m.Nu23;    // 泊松比 nu23
            u.matprops(i, 9) = m.Density; // 密度
        }

        // 确定单元类型（根据矩阵列数与声明类型推断）
        const auto &em = pre.FEMElementMatrix;
        int ncol = (int)em.cols(); // 单元矩阵实际列数

        // 列数与单元类型对应：
        //   Q4:  elemId n1 n2 n3 n4 matId theta        => 7 列
        //   Q8:  elemId n1..n8 matId theta              => 11 列
        //   T6:  elemId n1..n6 matId theta              => 9 列
        FEM::ElemType etype = static_cast<FEM::ElemType>(static_cast<int>(pre.ElememtType));
        // 若列数为11但声明的是Q4，以列数为准自动修正为Q8
        if (ncol == 11 && etype == FEM::Q4)
            etype = FEM::Q8;
        int nnpe = (etype == FEM::Q4) ? 4 : (etype == FEM::T6 ? 6 : 8); // 单元节点数

        u.max_nnpe = nnpe; // 记录最大单元节点数

        // el_2d: 每行为一个单元，列为 [内部id, n1, n2, ...]（节点编号使用1-based）
        u.el_2d.resize(u.ne_2d, 1 + nnpe);
        // emat: 每行为一个单元的材料信息 [标签, 材料索引(1-based), 纤维角度, 纤维平面角度]
        u.emat.resize(u.ne_2d, 4);
        u.etype.resize(u.ne_2d, etype); // 所有单元同一类型

        for (int e = 0; e < u.ne_2d; e++)
        {
            u.el_2d(e, 0) = e; // 单元内部编号（0-based）
            for (int n = 0; n < nnpe; n++)
            {
                // 将输入的0-based节点编号转换为FEM内部使用的1-based编号
                u.el_2d(e, 1 + n) = (int)em(e, 1 + n) + 1;
            }
            int matCol = 1 + nnpe;   // 材料ID列索引（节点列之后）
            int thetaCol = 2 + nnpe; // 纤维角度列索引

            int matId = (int)em(e, matCol);                           // 材料ID（与CompositeMaterial.Id对应）
            double theta = (thetaCol < ncol) ? em(e, thetaCol) : 0.0; // 纤维角度（无则默认0°）

            // 从材料列表中按ID查找材料的顺序索引（FEM代码内部使用1-based索引）
            int matIdx = 1; // 默认指向第1个材料
            for (int mi = 0; mi < u.nmat; mi++)
            {
                if (pre.Material[mi].Id == matId)
                {
                    matIdx = mi + 1;
                    break;
                } // 找到后转为1-based
            }
            u.emat(e, 0) = e;      // 单元标签（0-based）
            u.emat(e, 1) = matIdx; // 材料索引（1-based）
            u.emat(e, 2) = theta;  // 纤维主方向角度 [°]
            u.emat(e, 3) = 0;      // 纤维平面角度（暂留，默认0）
        }

        // 初始化单元信息表（含各类型节点数、高斯点数、自由度数等元数据）
        u.elemInfo.resize(4);
        u.elemInfo[0] = FEM::MakeQ4Info();  // Q4单元信息
        u.elemInfo[1] = FEM::MakeQ8Info();  // Q8单元信息
        u.elemInfo[2] = FEM::MakeQ8RInfo(); // Q8R单元信息
        u.elemInfo[3] = FEM::MakeT6Info();  // T6单元信息

        return u;
    }

} // namespace Qahse::PcsL
