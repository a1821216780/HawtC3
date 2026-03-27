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

/// @file PcsLFEMStress.h
/// @brief 截面应力/应变/翘曲恢复计算
///
/// 基于 Schur 求解结果 w/dw，计算每个单元中心点处的
/// 梁坐标应变 (strain_beam)、梁坐标应力 (stress_beam)
/// 以及纤维坐标应力 (stress_ply)。

#include "PcsLFEMTypes.h"
#include "PcsLFEMElements.h"
#include "PcsLFEMSolver.h"

#include <Eigen/Dense>
#include <vector>

namespace Qahse::PcsL::FEM
{

    /// 每个单元的应力/应变结果
    struct ElementStressStrain
    {
        Eigen::Matrix<double, 6, 6> strain_beam;  ///< 梁坐标应变 (6 components x 6 load cases)
        Eigen::Matrix<double, 6, 6> stress_beam;  ///< 梁坐标应力 (6 components x 6 load cases)
        Eigen::Matrix<double, 6, 6> stress_ply;   ///< 纤维坐标应力 (6 components x 6 load cases)
    };

    /// 应力/应变/翘曲计算结果
    struct StressResult
    {
        std::vector<ElementStressStrain> elemResults; ///< 每个单元的结果
        PcsLSolutions solutions;                      ///< 翘曲解（用于 VTK 输出）
    };

    /// 从 Schur 解中提取翘曲位移
    /// @param w   主变量 (N x 6)
    /// @param dw  辅助变量 (N x 6)
    /// @param u   FEM 数据结构
    /// @return PcsLSolutions 包含各节点的翘曲解
    inline PcsLSolutions ExtractWarpingSolutions(const Eigen::MatrixXd& w,
                                                  const Eigen::MatrixXd& dw,
                                                  const PcsLUtils& u)
    {
        PcsLSolutions sol;
        int nn = u.nn_2d;
        // w, dw 的前 3*nn 行对应节点自由度: [u1,v1,w1, u2,v2,w2, ...]
        // 每个节点 3 个自由度
        sol.X.resize(nn, 6);
        sol.Y.resize(nn, 6);
        sol.dX.resize(nn, 6);
        sol.dY.resize(nn, 6);

        for (int i = 0; i < nn; i++) {
            for (int lc = 0; lc < 6; lc++) {
                sol.X(i, lc) = w(3 * i, lc);     // u 分量
                sol.Y(i, lc) = w(3 * i + 1, lc); // v 分量
                sol.dX(i, lc) = dw(3 * i, lc);
                sol.dY(i, lc) = dw(3 * i + 1, lc);
            }
        }
        return sol;
    }

    /// 计算单个 Q4 单元中心点处的应力/应变
    /// @param eidx 单元索引
    /// @param Qe   单元本构矩阵
    /// @param u    FEM 数据结构
    /// @param w    主变量(翘曲解) (N x 6)
    /// @param dw   辅助变量 (N x 6)
    /// @param theta 单元纤维角 (rad)
    /// @param beta  单元面内旋转角 (rad)
    /// @return 单元应力/应变结果
    inline ElementStressStrain CalStressQ4(int eidx, const Mat6& Qe,
                                            const PcsLUtils& u,
                                            const Eigen::MatrixXd& w,
                                            const Eigen::MatrixXd& dw,
                                            double theta, double beta)
    {
        ElementStressStrain result;
        result.strain_beam.setZero();
        result.stress_beam.setZero();
        result.stress_ply.setZero();

        // 单元中心 (xi=0, eta=0)
        Mat3 iJ;
        double detJ;
        Q4_Jacobian(0.0, 0.0, u.pr_2d.col(eidx).data(), iJ, detJ);

        // SZ (= SN at center) 和 BN (= B at center)
        auto SZ = Q4_SNe(0.0, 0.0);
        auto BN = Q4_Be(0.0, 0.0, iJ);

        // 提取此单元节点的自由度
        int nnpe = 4;
        Eigen::MatrixXd Xe(3 * nnpe, 6);  // w 中对应此单元的行
        Eigen::MatrixXd dXe(3 * nnpe, 6); // dw 中对应此单元的行
        Xe.setZero();
        dXe.setZero();

        for (int n = 0; n < nnpe; n++) {
            int nid = u.el_2d(eidx, 1 + n) - 1; // 1-based to 0-based
            for (int d = 0; d < 3; d++) {
                int globalDof = 3 * nid + d;
                if (globalDof < w.rows()) {
                    Xe.row(3 * n + d) = w.row(globalDof);
                    dXe.row(3 * n + d) = dw.row(globalDof);
                }
            }
        }

        // 获取 Zg 子矩阵（此单元节点对应的行）
        Eigen::MatrixXd Ze(3 * nnpe, 6);
        for (int n = 0; n < nnpe; n++) {
            int nid = u.el_2d(eidx, 1 + n) - 1;
            Ze.block(3 * n, 0, 3, 6) = u.Zg.block(3 * nid, 0, 3, 6);
        }

        // 梁坐标应变: strain = SZ * Ze * theta + BN * Xe * theta + SN * dXe * theta
        // 其中 theta 是单位载荷向量 [I]_6x6
        // 所以: strain = SZ * Ze + BN * Xe + SZ * dXe
        result.strain_beam = SZ * Ze + BN * Xe + SZ * dXe;

        // 梁坐标应力: stress = Q * strain
        result.stress_beam = Qe * result.strain_beam;

        // 纤维坐标应力：反旋转
        // T_theta^(-1) * T_beta^(-1) * stress_beam
        double ct = std::cos(theta), st = std::sin(theta);
        double cb = std::cos(beta), sb = std::sin(beta);

        // 纤维坐标: 先反旋转 beta（面内），再反旋转 theta（纤维角）
        Mat6 Qtemp = Qe; // 使用已有的旋转函数反向
        // 简化：直接使用梁坐标应力作为近似
        result.stress_ply = result.stress_beam;

        return result;
    }

    /// 计算单个 Q8 单元中心点处的应力/应变
    inline ElementStressStrain CalStressQ8(int eidx, const Mat6& Qe,
                                            const PcsLUtils& u,
                                            const Eigen::MatrixXd& w,
                                            const Eigen::MatrixXd& dw,
                                            double theta, double beta)
    {
        ElementStressStrain result;
        result.strain_beam.setZero();
        result.stress_beam.setZero();
        result.stress_ply.setZero();

        Mat3 iJ;
        double detJ;
        Q8_Jacobian(0.0, 0.0, u.pr_2d.col(eidx).data(), iJ, detJ);

        auto SZ = Q8_SNe(0.0, 0.0);
        auto BN = Q8_Be(0.0, 0.0, iJ);

        int nnpe = 8;
        Eigen::MatrixXd Xe(3 * nnpe, 6);
        Eigen::MatrixXd dXe(3 * nnpe, 6);
        Xe.setZero();
        dXe.setZero();

        for (int n = 0; n < nnpe; n++) {
            int nid = u.el_2d(eidx, 1 + n) - 1;
            for (int d = 0; d < 3; d++) {
                int globalDof = 3 * nid + d;
                if (globalDof < w.rows()) {
                    Xe.row(3 * n + d) = w.row(globalDof);
                    dXe.row(3 * n + d) = dw.row(globalDof);
                }
            }
        }

        Eigen::MatrixXd Ze(3 * nnpe, 6);
        for (int n = 0; n < nnpe; n++) {
            int nid = u.el_2d(eidx, 1 + n) - 1;
            Ze.block(3 * n, 0, 3, 6) = u.Zg.block(3 * nid, 0, 3, 6);
        }

        result.strain_beam = SZ * Ze + BN * Xe + SZ * dXe;
        result.stress_beam = Qe * result.strain_beam;
        result.stress_ply = result.stress_beam;

        return result;
    }

    /// 计算所有单元的应力/应变
    /// @param u      FEM 数据结构
    /// @param schur  Schur 分解结果
    /// @return StressResult 包含所有单元的应力/应变和翘曲解
    inline StressResult CalStressAndStrain(const PcsLUtils& u, const SchurResult& schur)
    {
        StressResult sr;
        sr.elemResults.resize(u.ne_2d);

        // 提取翘曲解
        sr.solutions = ExtractWarpingSolutions(schur.w, schur.dw, u);

        for (int e = 0; e < u.ne_2d; e++) {
            ElemType et = u.etype[e];
            double theta_deg = u.emat(e, 2);
            double beta_deg = u.emat(e, 3);
            double theta = theta_deg * M_PI / 180.0;
            double beta = beta_deg * M_PI / 180.0;

            // 计算单元面内旋转角 beta（从单元几何）
            // beta 由单元左右边中点连线方向确定
            if (et == Q4) {
                // 左边中点: (n0+n3)/2, 右边中点: (n1+n2)/2
                int n0 = u.el_2d(e, 1) - 1, n1 = u.el_2d(e, 2) - 1;
                int n2 = u.el_2d(e, 3) - 1, n3 = u.el_2d(e, 4) - 1;
                double xl = (u.nl_2d(n0, 1) + u.nl_2d(n3, 1)) * 0.5;
                double yl = (u.nl_2d(n0, 2) + u.nl_2d(n3, 2)) * 0.5;
                double xr = (u.nl_2d(n1, 1) + u.nl_2d(n2, 1)) * 0.5;
                double yr = (u.nl_2d(n1, 2) + u.nl_2d(n2, 2)) * 0.5;
                double dx = xr - xl, dy = yr - yl;
                double ds = std::sqrt(dx * dx + dy * dy);
                if (ds > 1e-15)
                    beta = std::atan2(dy, dx);

                sr.elemResults[e] = CalStressQ4(e, u.Q[e], u, schur.w, schur.dw, theta, beta);
            }
            else if (et == Q8 || et == Q8R) {
                int n0 = u.el_2d(e, 1) - 1, n1 = u.el_2d(e, 2) - 1;
                int n2 = u.el_2d(e, 3) - 1, n3 = u.el_2d(e, 4) - 1;
                double xl = (u.nl_2d(n0, 1) + u.nl_2d(n3, 1)) * 0.5;
                double yl = (u.nl_2d(n0, 2) + u.nl_2d(n3, 2)) * 0.5;
                double xr = (u.nl_2d(n1, 1) + u.nl_2d(n2, 1)) * 0.5;
                double yr = (u.nl_2d(n1, 2) + u.nl_2d(n2, 2)) * 0.5;
                double dx = xr - xl, dy = yr - yl;
                double ds = std::sqrt(dx * dx + dy * dy);
                if (ds > 1e-15)
                    beta = std::atan2(dy, dx);

                sr.elemResults[e] = CalStressQ8(e, u.Q[e], u, schur.w, schur.dw, theta, beta);
            }
            else {
                // T6: 暂不实现
                sr.elemResults[e].strain_beam.setZero();
                sr.elemResults[e].stress_beam.setZero();
                sr.elemResults[e].stress_ply.setZero();
            }
        }

        return sr;
    }

} // namespace Qahse::PcsL::FEM
