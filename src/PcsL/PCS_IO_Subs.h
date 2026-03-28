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

#include "Pcs_Types.h"
#include "PCS_IO_Domain.h"
#include "../IO/IO/StringHelper.h"
#include "../IO/IO/CheckError.h"
#include "../IO/Log/LogHelper.h"
#include "../IO/IO/InputFileParser.h"
#include "FEM/Pcs_FEM_Types.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <tuple>
#include <cmath>
#include <iostream>
#include <initializer_list>

namespace Qahse::PcsL
{
    using Qahse::IO::IO::CheckError;
    using Qahse::IO::IO::StringHelper;
    using Qahse::IO::Log::LogHelper;
    using Qahse::IO::IO::InputFileParser;

    inline int FindFirstKeywordLineAny(const std::vector<std::string> &lines,
                                       const std::string &filepath,
                                       std::initializer_list<const char *> keys)
    {
        for (const char *key : keys)
        {
            const int idx = InputFileParser::FindKeywordLine(lines, key, filepath, false, false);
            if (idx >= 0)
                return idx;
        }
        return -1;
    }

    inline int ReadKeywordIntAny(const std::vector<std::string> &lines,
                                 const std::string &filepath,
                                 int defaultVal,
                                 std::initializer_list<const char *> keys,
                                 bool error = false,
                                 bool show = false)
    {
        for (const char *key : keys)
        {
            if (InputFileParser::FindKeywordLine(lines, key, filepath, false, false) >= 0)
                return InputFileParser::ReadKeywordInt(lines, key, filepath, defaultVal, error, show);
        }
        return defaultVal;
    }

    inline double ReadKeywordDoubleAny(const std::vector<std::string> &lines,
                                       const std::string &filepath,
                                       double defaultVal,
                                       std::initializer_list<const char *> keys,
                                       bool error = false,
                                       bool show = false)
    {
        for (const char *key : keys)
        {
            if (InputFileParser::FindKeywordLine(lines, key, filepath, false, false) >= 0)
                return InputFileParser::ReadKeywordDouble(lines, key, filepath, defaultVal, error, show);
        }
        return defaultVal;
    }

    inline bool ReadKeywordBoolAny(const std::vector<std::string> &lines,
                                   const std::string &filepath,
                                   bool defaultVal,
                                   std::initializer_list<const char *> keys,
                                   bool error = false,
                                   bool show = false)
    {
        for (const char *key : keys)
        {
            if (InputFileParser::FindKeywordLine(lines, key, filepath, false, false) >= 0)
                return InputFileParser::ReadKeywordBool(lines, key, filepath, defaultVal, error, show);
        }
        return defaultVal;
    }

    inline std::string ReadKeywordStringAny(const std::vector<std::string> &lines,
                                            const std::string &filepath,
                                            const std::string &defaultVal,
                                            std::initializer_list<const char *> keys,
                                            bool error = false,
                                            bool show = false)
    {
        for (const char *key : keys)
        {
            const int idx = InputFileParser::FindKeywordLine(lines, key, filepath, false, false);
            if (idx < 0)
                continue;

            const std::string parsed = InputFileParser::ParseQuotedString(lines[idx]);
            if (!parsed.empty())
                return parsed;

            return InputFileParser::ReadKeywordToken(lines, key, filepath, defaultVal, error, show);
        }
        return defaultVal;
    }

    // ===================================================================
    //  内部辅助：从文件行中按关键字查找行索引
    // ===================================================================

    /**
     * @brief 璇诲彇 PcsL 鎴潰鍒嗘瀽涓绘帶鏂囦欢锛?pcs锛夛紝瑙ｆ瀽鎵€鏈夎緭鍏ュ弬鏁?
     * @details 涓绘枃浠堕噰鐢ㄥ叧閿瓧-鏁板€兼牸寮忥紙绫?Fortran namelist 椋庢牸锛夛紝鏈嚱鏁伴€愭瑙ｆ瀽锛?
     *          1. 鍩烘湰淇℃伅锛堝彾鐗囬暱搴︺€佹埅闈㈡暟銆佹潗鏂欐暟绛夛級
     *          2. 璇诲彇鏉愭枡鏂囦欢锛圧eadMaterials锛?
     *          3. 搴斿姏鍒嗘瀽鍙傛暟锛圕alStress 妯″紡涓嬶級
     *          4. FEM 姹傝В鍙傛暟锛團EMSolve 妯″紡涓嬶級
     *          5. 鎴潰涓庤吂鏉垮畾涔夛紙闈?SelfMesh 妯″紡涓嬶紝鍚考鍨嬪拰閾哄眰鏂囦欢璇诲彇锛?
     *          6. 鑷畾涔夌綉鏍硷紙SelfMesh 妯″紡涓嬶紝璇诲彇鑺傜偣鍜屽崟鍏冪煩闃碉級
     * @param path .pcs 鏂囦欢鐨勭粷瀵硅矾寰?
     * @return 瑙ｆ瀽瀹屾垚鐨?PcsLInput 缁撴瀯浣?
     * @throws std::runtime_error 鏂囦欢涓嶅瓨鍦ㄦ垨鍏抽敭鍙傛暟缂哄け鏃舵姏鍑?
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
        pre.BladeLength = ReadKeywordDoubleAny(d, path, 0.0, {"Bl_length"}, true, true);                        // 叶片长度
        pre.SectionCount = ReadKeywordIntAny(d, path, 0, {"N_sections"}, false, false);                          // 截面数量（可选）
        pre.MaterialTypeCount = ReadKeywordIntAny(d, path, 0, {"N_materials"}, true, true);                      // 材料数量
        pre.OutputFormat = ReadKeywordIntAny(d, path, 3, {"Out_format"}, false, false);                          // 输出格式（默认3=双格式）
        pre.UseTabDelimiter = ReadKeywordBoolAny(d, path, true, {"TabDelim"}, false, false);                     // 是否使用Tab分隔符
        pre.FEMSolve = ReadKeywordBoolAny(d, path, false, {"FEMSolve", "FEMSove"}, false, false);               // 兼容旧拼写
        pre.SelfMesh = ReadKeywordBoolAny(d, path, false, {"SelfMesh"}, false, false);                            // 是否使用自定义网格
        pre.CalStress = ReadKeywordBoolAny(d, path, false, {"CalStress"}, false, false);                          // 是否计算应力
        pre.MaterialsPath = ReadKeywordStringAny(d, path, "", {"MaterialsPath"}, false, false);                  // 材料文件路径（相对或绝对）

        // 将材料路径解析为绝对路径（相对于主文件目录）
        if (!pre.MaterialsPath.empty())
        {
            pre.MaterialsPath = InputFileParser::ResolvePath(path, pre.MaterialsPath);
        }

        // ---- 读取材料属性文件 ----
        pre.Material = ReadMaterials(pre.MaterialsPath, pre.MaterialTypeCount, pre.FEMSolve);

        // ---- 输出目录路径 ----
        pre.SumPath = ReadKeywordStringAny(d, path, "", {"SumPath"}, false, false); // 汇总输出目录
        if (!pre.SumPath.empty())
        {
            pre.SumPath = InputFileParser::ResolvePath(path, pre.SumPath);     // 转为绝对路径
            std::filesystem::create_directories(pre.SumPath); // 确保目录存在
        }

        // ---- 应力/应变分析参数（CalStress=true 时才解析）----
        if (pre.CalStress)
        {
            pre.StrainMeshFile = ReadKeywordStringAny(d, path, "", {"StrainMeshFile"}, false, false); // 应变网格文件
            if (!pre.StrainMeshFile.empty())
                pre.StrainMeshFile = InputFileParser::ResolvePath(path, pre.StrainMeshFile);
            pre.StrainCache = ReadKeywordStringAny(d, path, "", {"StrainCache"}, false, false); // 应变缓存文件
            if (!pre.StrainCache.empty())
                pre.StrainCache = InputFileParser::ResolvePath(path, pre.StrainCache);
            pre.CalFailure = ReadKeywordBoolAny(d, path, false, {"CalFailure"}, false, false); // 是否进行失效分析
            if (pre.CalFailure)
            {
                // 读取失效准则编号（如 1=MaxStress, 2=Tsai-Wu）
                pre.FailureCriterion = {ReadKeywordIntAny(d, path, 0, {"FailureCriterion"}, false, false)};
            }
            pre.ExtForceNum = ReadKeywordIntAny(d, path, 0, {"ExtForceNum"}, false, false); // 外部截面力数量
            if (pre.ExtForceNum > 0)
            {
                // 读取外部截面力表（每行6个分量：Fx Fy Fz Mx My Mz）
                int idx = FindFirstKeywordLineAny(d, path, {"!BeginEXTFORCE"});
                if (idx >= 0)
                {
                    pre.ExtForce = ReadMatrixBlock(d, idx + 1, pre.ExtForceNum, 6);
                }
            }
        }

        // ---- FEM 求解参数（FEMSolve=true 时才解析）----
        if (pre.FEMSolve)
        {
            pre.Shear_Center = ReadKeywordBoolAny(d, path, true, {"Shear_Center"}, false, false); // 是否计算剪切中心
            int et = ReadKeywordIntAny(d, path, 0, {"ElementType", "ElememtType"}, false, false); // 兼容旧拼写
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
            pre.VTKShow = ReadKeywordBoolAny(d, path, false, {"VTKShow"}, false, false);       // 是否输出VTK可视化文件
            pre.SaveCatch = ReadKeywordBoolAny(d, path, false, {"SaveCatch"}, false, false);     // 是否保存中间计算缓存

            if (!pre.SelfMesh)
            {
                // 网格生成参数（程序自动生成网格时有效）
                pre.MeshWeb = ReadKeywordIntAny(d, path, 4, {"MeshWeb"}, false, false);                   // 腹板厚度方向网格层数
                pre.SurMeshExpT = ReadKeywordIntAny(d, path, -1, {"SurMeshExpT"}, false, false);          // 上翼面网格加密系数
                pre.SurMeshExpC = ReadKeywordIntAny(d, path, -1, {"SurMeshExpC"}, false, false);          // 下翼面网格加密系数
                {
                    // 加密位置的归一化弦向坐标列表
                    int idx = FindFirstKeywordLineAny(d, path, {"SurMeshExpCNorm"});
                    if (idx >= 0)
                        pre.SurMeshExpCNorm = InputFileParser::ParseDoubleArray(d[idx]);
                }
                pre.OutMeshfFile = ReadKeywordBoolAny(d, path, true, {"OutMeshfFile"}, false, false);      // 是否输出网格文件
                pre.ConsidertwistAndpitch = ReadKeywordBoolAny(d, path, true, {"ConsidertwistAndpitch"}, false, false); // 是否考虑扭角和预倾角
            }
        }

        // ---- 非 SelfMesh 模式：读取截面定义与腹板结构 ----
        if (!pre.SelfMesh)
        {
            pre.Nweb = ReadKeywordIntAny(d, path, 0, {"Nweb"}, false, false);                   // 腹板数量
            pre.Ib_sp_stn = ReadKeywordIntAny(d, path, 1, {"Ib_sp_stn"}, false, false);         // 腹板内侧起始截面编号（1基）
            pre.Ob_sp_stn = ReadKeywordIntAny(d, path, 1, {"Ob_sp_stn"}, false, false);         // 腹板外侧终止截面编号（1基）

            // 读取腹板结构定义（每条腹板：WebId 内侧弦向坐标 外侧弦向坐标）
            if (pre.Nweb > 0)
            {
                auto webIdxs = InputFileParser::FindAllLineIndexes(d, "Web_num", path, false, false); // 查找所有腹板定义行
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
                auto spanIdxs = InputFileParser::FindAllLineIndexes(d, "Span_loc", path, false, false);
                if (spanIdxs.empty())
                {
                    LogHelper::ErrorLog("未找到 Span_loc 定义: " + path, "", "", 20, "ReadPcsL_MainFile");
                    return pre;
                }
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
                        sec.AirfoilFile = InputFileParser::ResolvePath(path, paths[0]);
                    if (paths.size() >= 2)
                        sec.LayupFile = InputFileParser::ResolvePath(path, paths[1]);

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
            int meshPathIdx = FindFirstKeywordLineAny(d, path, {"FEMMeshPath"});
            std::vector<std::string> meshLines;
            if (meshPathIdx >= 0)
            {
                pre.FEMMeshPath = ReadKeywordStringAny(d, path, "", {"FEMMeshPath"}, false, false);
                pre.FEMMeshPath = InputFileParser::ResolvePath(path, pre.FEMMeshPath);
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

            pre.FEMNodeNum = ReadKeywordIntAny(meshLines, pre.FEMMeshPath, 0, {"FEMNodeNum"}, true, true);
            pre.FEMElementNum = ReadKeywordIntAny(meshLines, pre.FEMMeshPath, 0, {"FEMElementNum"}, true, true);

            // 读取节点
            int nodeMarker = FindFirstKeywordLineAny(meshLines, pre.FEMMeshPath, {"!BeginNode"});
            if (nodeMarker < 0)
            {
                LogHelper::ErrorLog("未找到 !BeginNode: " + pre.FEMMeshPath, "", "", 20, "ReadPcsL_MainFile");
                return pre;
            }
            pre.FEMNodeMatrix = ReadMatrixBlock(meshLines, nodeMarker + 1, pre.FEMNodeNum, 3);

            // 读取单元
            int elemMarker = FindFirstKeywordLineAny(meshLines, pre.FEMMeshPath, {"!BeginElement"});
            if (elemMarker < 0)
            {
                LogHelper::ErrorLog("未找到 !BeginElement: " + pre.FEMMeshPath, "", "", 20, "ReadPcsL_MainFile");
                return pre;
            }
            pre.FEMElementMatrix = ReadMatrixBlockAuto(meshLines, elemMarker + 1, pre.FEMElementNum);
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
