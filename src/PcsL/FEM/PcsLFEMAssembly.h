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

#include "PcsLFEMTypes.h"
#include "PcsLFEMElements.h"
// Must include ChMatrix.h BEFORE Eigen/Sparse to activate EIGEN_SPARSEMATRIX_PLUGIN
// (Chrono adds virtual methods to SparseMatrix — without this, ODR violation with
//  other TUs that include Chrono causes crashes)
#include "chrono/core/ChMatrix.h"
#include <Eigen/Sparse>
#include <vector>

namespace Qahse::PcsL::FEM
{

    using SpMat = Eigen::SparseMatrix<double>; ///< 稀疏矩阵类型，用于全局刚度/质量等矩阵
    using Triplet = Eigen::Triplet<double>;    ///< 稀疏矩阵三元组，用于高效装配

    /// @brief 构建Zg矩阵 (3*nn x 6)：刚体约束矩阵
    /// @details Zg用于约束刚体运动自由度，防止刚体漂移。每个节点贡献3行，6列分别对应平移和转动。
    /// @param u 全局数据结构，Zg为其成员
    /// @note 对应MATLAB的PcsL_Zg.m
    inline void BuildZg(PcsLUtils &u)
    {
        int n3 = 3 * u.nn_2d; // 总自由度数（每节点3自由度）
        u.Zg.setZero(n3, 6);  // 初始化Zg矩阵为零，尺寸为(3*节点数, 6)
        for (int i = 0; i < u.nn_2d; i++)
        {
            int b = i * 3; // 当前节点在Zg中的起始行号
            // 刚体平移分量
            u.Zg(b, 0) = 1.0;     // x方向平移
            u.Zg(b + 1, 1) = 1.0; // y方向平移
            u.Zg(b + 2, 2) = 1.0; // z方向平移（2D问题z自由度可用于后续扩展）
            // 节点坐标
            double x = u.nl_2d(i, 1); // 节点x坐标
            double y = u.nl_2d(i, 2); // 节点y坐标
            // 刚体转动分量
            u.Zg(b, 5) = -y;     // 绕z轴转动对x分量的影响
            u.Zg(b + 1, 5) = x;  // 绕z轴转动对y分量的影响
            u.Zg(b + 2, 3) = y;  // 绕x轴转动对z分量的影响（2D问题可忽略）
            u.Zg(b + 2, 4) = -x; // 绕y轴转动对z分量的影响（2D问题可忽略）
        }
    }

    /// @brief 按单元重排节点坐标，生成pr_2d矩阵
    /// @details pr_2d(i, e)为第e个单元第i个自由度的节点坐标，便于单元矩阵计算
    /// @param u 全局数据结构，pr_2d为其成员
    /// @note 对应MATLAB的PcsL_ReorderNodalPositions.m
    inline void ReorderNodalPositions(PcsLUtils &u)
    {
        u.pr_2d.setZero(2 * u.max_nnpe, u.ne_2d); // 初始化pr_2d为零，行为2*最大单元节点数，列为单元数
        for (int e = 0; e < u.ne_2d; e++)
        {
            int nnpe = u.elemInfo[static_cast<int>(u.etype[e]) - 1].nnpe; // 当前单元节点数
            for (int i = 0; i < nnpe; i++)
            {
                int nid = u.el_2d(e, i + 1) - 1;         // 单元第i个节点的全局节点编号（0基）
                u.pr_2d(2 * i, e) = u.nl_2d(nid, 1);     // x坐标
                u.pr_2d(2 * i + 1, e) = u.nl_2d(nid, 2); // y坐标
            }
        }
    }

    /**
     * @brief 全局刚度块矩阵结构体
     * @details K11, H, G22分别为系统刚度、耦合、质量块（稀疏矩阵），用于广义特征值问题
     */
    struct StiffnessBlocks
    {
        SpMat K11; ///< 主刚度块（含刚体约束）
        SpMat H;   ///< 耦合块
        SpMat G22; ///< 质量块
    };

    /// @brief 装配全局刚度、耦合、质量块矩阵（稀疏）
    /// @details 逐单元装配Eg, Cg, Mg，构造K11/H/G22大系统矩阵，适用于广义特征值分析
    /// @param u 全局数据结构
    /// @return StiffnessBlocks结构体，含K11/H/G22
    /// @note 对应MATLAB的PcsL_Assemble_Ks.m
    inline StiffnessBlocks AssembleKs(const PcsLUtils &u)
    {
        int ndof = 3 * u.nn_2d; // 全局自由度数

        // --------- 阶段1：逐单元装配Eg, Cg, Mg ---------
        // setFromTriplets会自动累加重复(i,j)项，适合FEM装配
        std::vector<Triplet> tripE, tripC, tripM;
        {
            // 预估三元组容量：每个单元贡献mdim²个条目
            int maxMdim = 24; // Q8单元最大自由度数
            size_t est = (size_t)u.ne_2d * maxMdim * maxMdim;
            tripE.reserve(est);
            tripC.reserve(est);
            tripM.reserve(est);
        }

        for (int e = 0; e < u.ne_2d; e++)
        {
            const auto &ei = u.elemInfo[static_cast<int>(u.etype[e]) - 1]; // 当前单元类型信息
            int nnpe = ei.nnpe;                                            // 节点数
            int mdim = ei.mdim;                                            // 自由度数

            Eigen::MatrixXd Me, Ee, Ce;                       // 单元质量、刚度、耦合矩阵
            ComputeElementMatrices(e, u.Q[e], u, Me, Ee, Ce); // 计算单元矩阵

            // 构建单元自由度到全局自由度的映射
            std::vector<int> edof(mdim); // 单元自由度编号
            for (int i = 0; i < nnpe; i++)
            {
                int gnode = u.el_2d(e, i + 1) - 1; // 全局节点编号
                for (int j = 0; j < 3; j++)        // 每节点3自由度
                    edof[3 * i + j] = 3 * gnode + j;
            }

            // 将单元矩阵分量散装到全局三元组
            for (int i = 0; i < mdim; i++)
                for (int j = 0; j < mdim; j++)
                {
                    int gi = edof[i], gj = edof[j];       // 全局自由度编号
                    tripE.emplace_back(gi, gj, Ee(i, j)); // 刚度
                    tripC.emplace_back(gi, gj, Ce(i, j)); // 耦合
                    tripM.emplace_back(gi, gj, Me(i, j)); // 质量
                }
        }

        // 由三元组生成稀疏矩阵
        SpMat EgS(ndof, ndof), CgS(ndof, ndof), MgS(ndof, ndof);
        EgS.setFromTriplets(tripE.begin(), tripE.end()); // 全局刚度
        tripE.clear();
        tripE.shrink_to_fit();
        CgS.setFromTriplets(tripC.begin(), tripC.end()); // 全局耦合
        tripC.clear();
        tripC.shrink_to_fit();
        MgS.setFromTriplets(tripM.begin(), tripM.end()); // 全局质量
        tripM.clear();
        tripM.shrink_to_fit();

        // 矩阵对称化（与MATLAB一致，消除数值非对称）
        SpMat EgSym = (EgS + SpMat(EgS.transpose())) * 0.5;
        SpMat MgSym = (MgS + SpMat(MgS.transpose())) * 0.5;
        EgS = EgSym;
        MgS = MgSym;

        // 推导出的稠密子块（小矩阵/细矩阵）
        const Eigen::MatrixXd &ZgD = u.Zg;           // 3nn x 6，刚体约束矩阵
        Eigen::MatrixXd RgD = CgS * ZgD;             // 3nn x 6，耦合块
        Eigen::MatrixXd LgD = MgS * ZgD;             // 3nn x 6，质量-约束块
        Eigen::MatrixXd AgD = ZgD.transpose() * LgD; // 6x6，约束质量块
        AgD = (AgD + AgD.transpose()) * 0.5;         // 对称化
        Eigen::MatrixXd DgD = ZgD.transpose();       // 6 x 3nn，约束转置

        int N = ndof + 12; // 总自由度数+12（6刚体+6约束）

        // --------- 阶段2：组装大系统块矩阵 ---------
        // K11 = [Eg Rg Dg'; Rg' Ag 0; Dg 0 0]，H = [Cg^T Lg 0; 0 0 0; 0 0 0]，G22 = [Mg 0 0; 0 0 0; 0 0 0]
        std::vector<Triplet> tripK;
        tripK.reserve(EgS.nonZeros() + 4 * ndof * 6 + 36); // 预估容量

        // Eg块（左上ndof x ndof）
        for (int k = 0; k < EgS.outerSize(); ++k)
            for (SpMat::InnerIterator it(EgS, k); it; ++it)
                tripK.emplace_back((int)it.row(), (int)it.col(), it.value());

        // Rg块（刚体耦合，及其转置）
        for (int i = 0; i < ndof; i++)
            for (int j = 0; j < 6; j++)
            {
                double v = RgD(i, j);
                if (v != 0.0)
                {
                    tripK.emplace_back(i, ndof + j, v); // Rg
                    tripK.emplace_back(ndof + j, i, v); // Rg转置
                }
            }

        // Ag块（刚体质量块）
        for (int i = 0; i < 6; i++)
            for (int j = 0; j < 6; j++)
                if (AgD(i, j) != 0.0)
                    tripK.emplace_back(ndof + i, ndof + j, AgD(i, j));

        // Dg'块（右上）和Dg块（左下）
        for (int i = 0; i < 6; i++)
            for (int j = 0; j < ndof; j++)
            {
                double v = DgD(i, j);
                if (v != 0.0)
                {
                    tripK.emplace_back(j, ndof + 6 + i, v); // Dg'（右上）
                    tripK.emplace_back(ndof + 6 + i, j, v); // Dg（左下）
                }
            }

        SpMat K11(N, N); // 主刚度块
        K11.setFromTriplets(tripK.begin(), tripK.end());
        tripK.clear();
        tripK.shrink_to_fit();

        // Build H = [Cg^T Lg 0; 0 0 0; 0 0 0] as sparse
        SpMat CgT = CgS.transpose();
        std::vector<Triplet> tripH;
        tripH.reserve(CgT.nonZeros() + ndof * 6);

        for (int k = 0; k < CgT.outerSize(); ++k)
            for (SpMat::InnerIterator it(CgT, k); it; ++it)
                tripH.emplace_back((int)it.row(), (int)it.col(), it.value());

        for (int i = 0; i < ndof; i++)
            for (int j = 0; j < 6; j++)
                if (LgD(i, j) != 0.0)
                    tripH.emplace_back(i, ndof + j, LgD(i, j));

        SpMat H(N, N); // 耦合块
        H.setFromTriplets(tripH.begin(), tripH.end());
        tripH.clear();
        tripH.shrink_to_fit();

        // Build G22 = [Mg 0 0; 0 0 0; 0 0 0] as sparse
        std::vector<Triplet> tripG;
        tripG.reserve(MgS.nonZeros());

        for (int k = 0; k < MgS.outerSize(); ++k)
            for (SpMat::InnerIterator it(MgS, k); it; ++it)
                tripG.emplace_back((int)it.row(), (int)it.col(), it.value());

        SpMat G22(N, N); // 质量块
        G22.setFromTriplets(tripG.begin(), tripG.end());

        return {K11, H, G22};
    }

    /// @brief 由质量属性组装6x6截面质量矩阵
    /// @param Mass 总质量
    /// @param xm, ym 质心坐标
    /// @param Ixx, Iyy, Ixy 惯性矩
    /// @return 6x6 质量矩阵
    /// @note 对应MATLAB的PcsL_Assemble_Ms.m
    inline Eigen::Matrix<double, 6, 6> AssembleMs(double Mass, double xm, double ym,
                                                  double Ixx, double Iyy, double Ixy)
    {
        Eigen::Matrix<double, 6, 6> Ms;
        Ms.setZero(); // 初始化为零
        // 质量主对角块
        Ms(0, 0) = Mass; // x方向质量
        Ms(1, 1) = Mass; // y方向质量
        Ms(2, 2) = Mass; // z方向质量（2D问题可忽略）
        // 质量-转动耦合项
        Ms(0, 5) = -Mass * ym; // x-转动
        Ms(1, 5) = Mass * xm;  // y-转动
        Ms(2, 3) = Mass * ym;  // z-x
        Ms(2, 4) = -Mass * xm; // z-y
        // 下三角对称块
        Ms(5, 0) = -Mass * ym;
        Ms(5, 1) = Mass * xm;
        Ms(3, 2) = Mass * ym;
        Ms(4, 2) = -Mass * xm;
        // 转动惯量块
        Ms(3, 3) = Ixx;       // 绕x轴惯性矩
        Ms(3, 4) = -Ixy;      // 惯性积
        Ms(4, 3) = -Ixy;      // 对称
        Ms(4, 4) = Iyy;       // 绕y轴惯性矩
        Ms(5, 5) = Ixx + Iyy; // 绕z轴惯性矩
        return Ms;
    }

} // namespace PCSL
