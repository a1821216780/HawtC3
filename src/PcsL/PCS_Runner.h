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

#include "chrono/core/ChMatrix.h" // MUST come before any Eigen include (EIGEN_SPARSEMATRIX_PLUGIN)
#include "Pcs_Types.h"
#include "Pcs_IO_Subs.h"
#include "FEM/Pcs_FEM_Types.h"
#include "FEM/Pcs_FEM_Solver.h"
#include "FEM/Pcs_FEM_Assembly.h"
#include "FEM/Pcs_FEM_Mesh.h"
#include "FEM/Pcs_FEM_Stress.h"
#include "FEM/Pcs_FEM_VTK.h"
#include "CLT/PCS_CLT_Run.h"
#include "IO/Log/LogHelper.h"
#include "IO/IO/OtherHelper.h"

#include <filesystem>
#include <iostream>
#include <iomanip>
#include <fstream>

namespace PCSL
{
    using namespace Qahse::PcsL;
    using namespace Qahse::PcsL::FEM;
    using namespace Qahse::PcsL::CLT;
    using namespace Qahse::IO::Log;
    using namespace Qahse::IO::IO;

    /**
     * @brief 在指定目录下查找并返回第一个 .pcs 文件的完整路径
     * @details 遍历目录下所有文件，找到第一个扩展名为 .pcs 的普通文件并返回其路径。
     *          没有找到时返回空字符串。
     * @param folder 要搜索的目录路径
     * @return 第一个 .pcs 文件的绝对路径；未找到返回空字符串
     *
     * @par 使用示例
     * @code
     * std::string pcs = FindPcsFile("Demo/NREL_5MW_OC3_Monopile");
     * if (!pcs.empty()) RunPcslFromFile(pcs);
     * @endcode
     */
    inline std::string FindPcsFile(const std::string& folder)
    {
        namespace fs = std::filesystem;
        for (auto& entry : fs::directory_iterator(folder)) { // 遍历目录下所有条目
            if (entry.is_regular_file() && entry.path().extension() == ".pcs") {
                return entry.path().string(); // 找到第一个 .pcs 文件即返回
            }
        }
        return ""; // 未找到

    }


    /// @brief 在指定目录下查找所有 .pcs 文件的完整路径列表
    /// @details 遍历目录下所有文件，收集扩展名为 .pcs 的普通文件的路径并返回列表。
    ///          没有找到时返回空列表，并记录错误日志。
    /// @param folder 要搜索的目录路径
    /// @return 包含所有匹配文件路径的字符串向量；未找到返回空向量
    ///
    /// @par 使用示例
    /// @code
    /// auto pcsFiles = FindPcsFiles("Demo/NREL_5MW_OC3_Monopile");
    /// if (pcsFiles.empty()) { /* 处理未找到情况 */ }
    /// @endcode
    /// @note 适用于需要批量处理多个 .pcs 文件的场景，如一个目录下有多个截面分析文件。
    inline std::vector<std::string> FindPcsFiles(const std::string& folder)
    {
        namespace fs = std::filesystem;
        auto result = OtherHelper::FindFilesWithExtension(folder, ".pcs");
        if (result.empty()) 
        {
            LogHelper::ErrorLog("在目录 " + folder + " 中未找到任何 .pcs 文件！", "", "", 20, "FindPcsFiles");
        }
        return result;

    }

    /**
     * @brief 将 6×6 矩阵以科学记数法格式化输出到流
     * @details 先输出标题，再按行输出矩阵元素，每个元素占 16 个字符宽。
     * @param os    输出流（文件流或控制台流）
     * @param title 矩阵标题（输出在矩阵前）
     * @param M     要输出的6×6矩阵
     *
     * @par 使用示例
     * @code
     * WriteMatrix6x6(std::cout, "Ks", result.constitutive.Ks);
     * @endcode
     */
    inline void WriteMatrix6x6(std::ostream& os, const std::string& title, const Eigen::Matrix<double, 6, 6>& M)
    {
        os << "\n" << title << ":\n";               // 输出标题
        os << std::scientific << std::setprecision(6); // 科学记数法，6位小数
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6; j++) {
                os << std::setw(16) << M(i, j); // 每列对齁16字符宽
            }
            os << "\n"; // 换行
        }
    }

    /**
     * @brief 将车截面分析结果写入输出目录的 .out 结果文件
     * @details 输出内容包括：
     *          - 6×6截面刚度矩阵（原点和剪切中心）
     *          - 6×6质量矩阵
     *          - 弹性中心、剪切中心、质量中心、惯性矩、面积等截面属性
     *          输出文件名为 "SectionResult_{title}.out"
     * @param outDir     输出目录（自动创建）
     * @param title      截面标题（通常为文件名不含扩展名）
     * @param result     截面分析结果（PcsLResult 结构体）
     * @param shearCenter 是否将刚度矩阵变换到了剪切中心
     *
     * @par 使用示例
     * @code
     * WriteResults("Result", "section_r10", result, true);
     * // 生成 Result/SectionResult_section_r10.out
     * @endcode
     */
    inline void WriteResults(const std::string& outDir, const std::string& title,
                             const PcsLResult& result, bool shearCenter)
    {
        namespace fs = std::filesystem;
        fs::create_directories(outDir); // 确保输出目录存在

        std::string filename = outDir + "/SectionResult_" + title + ".out"; // 输出文件路径
        std::ofstream ofs(filename);
        if (!ofs.is_open()) {
            LogHelper::ErrorLog("无法创建输出文件: " + filename, "", "", 20, "WriteResults");
            return;
        }

        // 文件头部装饰
        ofs << "============================================================\n";
        ofs << "  PcsL Cross-Section Analysis Results\n";
        ofs << "  Title: " << title << "\n";
        ofs << "============================================================\n";

        // 输出三个6×6矩阵
        WriteMatrix6x6(ofs, "Cross-Section Stiffness Matrix (Ks)", result.constitutive.Ks);
        WriteMatrix6x6(ofs, "Cross-Section Stiffness Matrix at Origin (Ks_origin)", result.constitutive.Ks_origin);
        WriteMatrix6x6(ofs, "Cross-Section Mass Matrix (Ms)", result.constitutive.Ms);

        const auto& p = result.props; // 截面属性引用
        ofs << "\n--- Cross-Section Properties ---\n";
        ofs << std::scientific << std::setprecision(8);
        ofs << "  Elastic Center X    : " << p.ElasticX << "\n"; // 弹性中心 x 坐标
        ofs << "  Elastic Center Y    : " << p.ElasticY << "\n"; // 弹性中心 y 坐标
        ofs << "  Shear Center X      : " << p.ShearX << "\n";   // 剪切中心 x 坐标
        ofs << "  Shear Center Y      : " << p.ShearY << "\n";   // 剪切中心 y 坐标
        ofs << "  Mass per unit length: " << p.MassTotal << "\n"; // 单位长度质量
        ofs << "  Mass Center X       : " << p.MassX << "\n";    // 质量中心 x 坐标
        ofs << "  Mass Center Y       : " << p.MassY << "\n";    // 质量中心 y 坐标
        ofs << "  Ixx                 : " << p.Ixx << "\n";       // 绕 x 轴质量惯性矩
        ofs << "  Iyy                 : " << p.Iyy << "\n";       // 绕 y 轴质量惯性矩
        ofs << "  Ixy                 : " << p.Ixy << "\n";       // 惯性积
        ofs << "  Area                : " << p.AreaTotal << "\n"; // 截面总面积
        ofs << "  Area Center X       : " << p.AreaX << "\n";    // 面积中心 x 坐标
        ofs << "  Area Center Y       : " << p.AreaY << "\n";    // 面积中心 y 坐标
        // 主轴方向（相对参考点和弹性中心）转换为度输出
        ofs << "  Principal Axis (ref): " << p.AlphaPrincipleAxis_Ref * 180.0 / 3.14159265358979 << " deg\n";
        ofs << "  Principal Axis (EC) : " << p.AlphaPrincipleAxis_ElasticCenter * 180.0 / 3.14159265358979 << " deg\n";

        if (shearCenter) {
            ofs << "\n  (Stiffness matrix transformed to shear center)\n"; // 已变换到剪切中心
        }

        ofs.close();
        std::cout << "  Result written to: " << filename << std::endl;
    }

    /**
     * @brief PcsL 截面分析主流程函数（自动选择 CLT 或 FEM 方法）
     * @details 根据 PcsLInput::FEMSolve 标志选择分析方法：
     *          - FEMSolve=false → 调用 CLT（经典层合板理论）分析，适用于叶片多截面梁特性计算
     *          - FEMSolve=true  → 调用 FEM 截面分析流程，包括：
     *            1. 将输入数据转换为 FEM 平台格式（SelfMesh或自动网格生成）
     *            2. 计算各单元本构矩阵（含纤维旋转）
     *            3. 构建刚体运动约束矩阵 Zg
     *            4. 重排节点坐标
     *            5. 装配全局刚度/耗合块矩阵
     *            6. Schur 分解求 6×6 截面刚度矩阵
     *            7. 计算 6×6 质量矩阵
     *            8. 计算截面属性（弹性中心、剪切中心等）
     *            9. （可选）变换刚度矩阵到剪切中心
     *            10. （可选）应力/应变/翘曲恢复
     *            11. （可选）生成 VTK 可视化文件
     *            12. 输出结果到文件和控制台
     * @param pre 已解析的 PcsLInput 结构体（非const引用，CLT模式下会修改翼型节点数组）
     * @return 0 表示成功；非0 表示失败
     *
     * @par 使用示例
     * @code
     * PcsLInput pre = ReadPcsL_MainFile("section.pcs");
     * int ret = RunPcslAnalysis(pre);
     * if (ret != 0) std::cerr << "分析失败" << std::endl;
     * @endcode
     */
    inline int RunPcslAnalysis(PcsLInput& pre)
    {
        if (!pre.FEMSolve) {
            // CLT（经典层合板理论）分析模式
            std::cout << "  Using CLT (Classical Laminate Theory) analysis mode." << std::endl;
            auto cltResults = RunCLT(pre);
            if (cltResults.empty()) {
                LogHelper::ErrorLog("CLT 分析返回空结果", "", "", 20, "RunPcslAnalysis");
                return 1;
            }
            return 0;
        }

        std::cout << "  Converting input to FEM data structures..." << std::endl;

        PcsLUtils u; // FEM 求解器全局数据结构体
        if (pre.SelfMesh) {
            // SelfMesh=true: 用户提供了自定义节点/单元表，直接转换
            u = ConvertToFEMUtils(pre);
        } else {
            // SelfMesh=false: 从翼型+铺层自动生成网格
            std::cout << "  Generating mesh from airfoil + layup..." << std::endl;
            if (pre.Sections.empty()) {
                LogHelper::ErrorLog("SelfMesh=false 但无截面数据", "", "", 20, "RunPcslAnalysis");
                return 1;
            }
            BladeSection sec = pre.Sections[0]; // 取第一个截面（复制以便修改）
            MeshResult meshResult = CalAirfoilMesh(
                sec, 0, pre.Webs, pre.Nweb,
                pre.Ib_sp_stn, pre.Ob_sp_stn, pre.Material);
            std::cout << "  Mesh generated: " << meshResult.nodes.size() << " nodes, "
                      << meshResult.elements.size() << " elements" << std::endl;
            u = ConvertMeshToFEMUtils(meshResult, pre.Material, pre.ElememtType);
        }

        // ── 步骤1：计算各单元正交异性本构矩阵（D矩阵），含纤维方向旋转变换
        std::cout << "  Computing constitutive matrices..." << std::endl;
        ComputeAllConstitutiveMatrices(u);

        // ── 步骤2：构建刚体运动约束矩阵 Zg（3*nn × 6），用于 Schur 分解中消除刚体模态
        std::cout << "  Building constraint matrix Zg..." << std::endl;
        BuildZg(u);

        // ── 步骤3：重排节点坐标，生成 pr_2d（单元-节点坐标存取表）
        std::cout << "  Reordering nodal positions..." << std::endl;
        ReorderNodalPositions(u);

        // ── 步骤4：装配全局刚度/耗合块矩阵（K11/H/G22），为 Schur 分解准备
        std::cout << "  Assembling global stiffness/mass matrices..." << std::endl;
        StiffnessBlocks blocks = AssembleKs(u);

        // ── 步骤5：用 Schur 分解求解 6×6 截面刚度矩阵 Ks
        //          Schur 补矩阵：Ks = H^T * K11^{-1} * H - G22
        std::cout << "  Solving Schur decomposition for 6x6 stiffness..." << std::endl;
        SchurResult schur = SolveSchur(blocks.K11, blocks.H, blocks.G22);

        // ── 步骤6：计算截面质量特性（单位长度质量、质心、惯性矩、面积）
        std::cout << "  Assembling 6x6 mass matrix..." << std::endl;
        double Mass, xm, ym, Ixx, Iyy, Ixy, AreaX, AreaY, Axx, Ayy, Axy, AreaTotal;
        CalcSectionMassProps(u, Mass, xm, ym, Ixx, Iyy, Ixy, AreaX, AreaY, Axx, Ayy, Axy, AreaTotal);
        Eigen::Matrix<double, 6, 6> Ms = AssembleMs(Mass, xm, ym, Ixx, Iyy, Ixy); // 组装 6×6 截面质量矩阵

        // ── 步骤7：汇总分析结果到 PcsLResult 结构体
        PcsLResult result;
        result.constitutive.Ks_origin = schur.Ks; // 原点处刚度矩阵（保留备用）
        result.constitutive.Ks        = schur.Ks; // 待变换刚度矩阵
        result.constitutive.Ms        = Ms;        // 质量矩阵

        // ── 步骤8：由 Ks 计算弹性中心、剪切中心、质量中心等截面属性（与 BECAS 基准一致）
        std::cout << "  Computing cross-section properties..." << std::endl;
        result.props = ComputeCrossSectionProps(schur.Ks, u);

        // ── 步骤9：若需要，将刚度矩阵由原点变换到剪切中心（平行轴定理）
        if (pre.Shear_Center) {
            std::cout << "  Transforming to shear center..." << std::endl;
            result.constitutive.Ks = TransformCrossSectionMatrix(
                schur.Ks, result.props.ShearX, result.props.ShearY, 0.0);
        }

        // ── 步骤10：（可选）应力/应变/翘曲恢复 —— 需 CalStress=true 或 VTKShow=true
        StressResult stressResult;
        if (pre.CalStress || pre.VTKShow) {
            std::cout << "  Computing stress/strain/warping recovery..." << std::endl;
            stressResult = CalStressAndStrain(u, schur); // 利用 Schur 模态解恢复场量
            result.solutions = stressResult.solutions;   // 存储应力/翘曲模态解
        }

        // ── 步骤11：（可选）生成 VTK 可视化文件，供 ParaView 等后处理工具使用
        if (pre.VTKShow) {
            std::string title = std::filesystem::path(pre.path).stem().string(); // .pcs 文件名（无扩展名）
            std::string outDir = pre.SumPath.empty()
                ? (std::filesystem::path(pre.path).parent_path() / "Result1").string()
                : pre.SumPath; // 确定输出目录
            std::string vtkBase = outDir + "/" + title; // VTK 文件基名
            if (pre.CalStress) {
                // 包含应力场时输出所有载荷工况（6个单位荷载工况）
                WritePcsLVTK_AllLoadCases(vtkBase, u, stressResult);
            } else {
                // 仅输出网格 + 材料ID + 纤维角（不含应力场）
                WritePcsLVTK(vtkBase + ".vtu", u);
            }
        }

        // ── 步骤12：将结果写入 .out 文件并打印摘要到控制台
        std::string title = std::filesystem::path(pre.path).stem().string(); // 以 .pcs 文件名为标题
        std::string outDir = pre.SumPath.empty()
            ? (std::filesystem::path(pre.path).parent_path() / "Result1").string()
            : pre.SumPath;
        WriteResults(outDir, title, result, pre.Shear_Center);

        // 将关键结果打印到控制台（6×6 Ks + 特征点坐标）
        std::cout << "\n  ============ Cross-Section Stiffness Matrix (Ks) ============\n";
        std::cout << std::scientific << std::setprecision(6);
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6; j++) {
                std::cout << std::setw(16) << result.constitutive.Ks(i, j); // 逐行打印矩阵元素
            }
            std::cout << "\n";
        }
        std::cout << "\n  Elastic Center: (" << result.props.ElasticX << ", " << result.props.ElasticY << ")\n";
        std::cout << "  Shear Center:   (" << result.props.ShearX   << ", " << result.props.ShearY   << ")\n";
        std::cout << "  Mass/length:    " << result.props.MassTotal << "\n";

        return 0; // 分析成功
    }

    /**
     * @brief CLI 入口函数：从 .pcs 文件路径启动 PcsL 截面分析
     * @details 检查文件存在，读取主文件，打印配置信息，然后调用 RunPcslAnalysis 执行分析。
     *          适用于命令行直接输入文件路径的场景。
     * @param filepath .pcs 文件的完整路径
     * @return 0 表示成功；非0 表示失败（1=文件不存在或分析失败）
     *
     * @par 使用示例
     * @code
     * int ret = PCSL::RunPcslFromFile("Demo/NREL_5MW_OC3_Monopile/section.pcs");
     * @endcode
     */
    inline int RunPcslFromFile(const std::string& filepath)
    {
        std::cout << "=== Qahse PcsL Cross-Section Analysis ===" << std::endl;
        std::cout << "  Input file: " << filepath << std::endl;

        // 检查输入文件是否存在
        if (!std::filesystem::exists(filepath)) {
            LogHelper::ErrorLog("输入文件不存在: " + filepath, "", "", 20, "RunPcslFromFile");
            return 1;
        }

        // 解析主文件，读入所有参数
        std::cout << "  Reading input file..." << std::endl;
        PcsLInput pre = ReadPcsL_MainFile(filepath);

        // 打印关键输入参数到控制台
        std::cout << "  Blade length: " << pre.BladeLength << " m" << std::endl;
        std::cout << "  Materials: " << pre.MaterialTypeCount << std::endl;
        std::cout << "  Analysis Mode: " << (pre.FEMSolve ? "FEM" : "CLT") << std::endl;
        if (pre.FEMSolve) {
            std::cout << "  Self Mesh: " << (pre.SelfMesh ? "Yes" : "No") << std::endl;
        }
        // 单元类型名称映射函数
        auto etypeName = [](PcsLElementType t) {
            switch (t) {
                case PcsLElementType::Q8:  return "Q8";
                case PcsLElementType::Q8R: return "Q8R";
                case PcsLElementType::T6:  return "T6";
                default:                   return "Q4";
            }
        };
        std::cout << "  Element Type: " << etypeName(pre.ElememtType) << std::endl;
        if (pre.SelfMesh) {
            std::cout << "  Nodes: " << pre.FEMNodeNum << std::endl;       // 节点数
            std::cout << "  Elements: " << pre.FEMElementNum << std::endl; // 单元数
        }

        return RunPcslAnalysis(pre); // 执行分析主流程
    }
} // namespace PCSL
