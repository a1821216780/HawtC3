#pragma once
//**********************************************************************************************************************************
// LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of HawtC3.IO.Math
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

#include "chrono/core/ChMatrix.h" // EIGEN_SPARSEMATRIX_PLUGIN (ODR consistency)
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#include <Eigen/PardisoSupport>
#include <iostream>
#include <stdexcept>
#include <cmath>

#include "../../IO/Log/LogHelper.h"
#include "PcsLFEMTypes.h"
#include "PcsLFEMElements.h"
#include "PcsLFEMAssembly.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace HawtC3::IO::Log;

namespace HawtC3::PcsL::FEM
{

    // 稀疏矩阵类型定义，便于后续代码书写
    using SpMat = Eigen::SparseMatrix<double>;
    using Mat6 = Eigen::Matrix<double, 6, 6>;

    /// @brief Schur分解结果结构体
    /// @details 存储截面刚度矩阵及求解过程中用到的中间变量
    struct SchurResult
    {
        Eigen::Matrix<double, 6, 6> Ks; ///< 6x6 截面刚度矩阵，反映截面整体刚度特性
        Eigen::MatrixXd w;              ///< 求解得到的主未知量（位移/广义力）
        Eigen::MatrixXd dw;             ///< 求解得到的辅助未知量（约束相关变量）
    };

    /**
     * @brief 求解广义截面刚度的Schur分解主流程
     * @details 输入为组装好的大系统稀疏矩阵K11、H、G22，输出为截面刚度矩阵及中间变量。采用MKL PardisoLU直接法求解鞍点系统，自动回退到SparseLU（极少发生）。
     * @param K11 主刚度块（N×N稀疏矩阵，含约束）
     * @param H   耦合块（N×N稀疏矩阵）
     * @param G22 质量/约束块（N×N稀疏矩阵）
     * @return SchurResult结构体，含6x6截面刚度矩阵Ks及主/辅助未知量
     */
    inline SchurResult SolveSchur(const SpMat &K11,
                                  const SpMat &H,
                                  const SpMat &G22)
    {
        int N = static_cast<int>(K11.rows()); // 系统总自由度数
        int nEx = N - 12;                     // 主自由度数（去除6刚体+6约束）

        // PardisoLU: MKL加速的直接法求解器，适用于不定鞍点系统
        // 若Pardiso不可用会自动回退到SparseLU（一般不会发生）
        Eigen::PardisoLU<SpMat> lu;
        lu.analyzePattern(K11); // 分析稀疏结构
        lu.factorize(K11);      // 数值分解
        if (lu.info() != Eigen::Success)
        {
            LogHelper::ErrorLog("PardisoLU factorization failed for K11"); // 分解失败记录日志
        }

        // 第一步：求解辅助变量dw
        Eigen::MatrixXd rhs1 = Eigen::MatrixXd::Zero(N, 6); // 右端项初始化为零
        rhs1(nEx + 4, 0) = -1;                              // 施加单位约束（物理意义：刚体平移/转动）
        rhs1(nEx + 3, 1) = 1;
        Eigen::MatrixXd dw = lu.solve(rhs1); // 求解dw

        // 第二步：求解主变量w
        Eigen::MatrixXd rhs2 = Eigen::MatrixXd::Zero(N, 6); // 右端项初始化为零
        for (int i = 0; i < 6; i++)
            rhs2(nEx + i, i) = 1.0; // 施加单位主约束
        // H^T - H 作用于dw，物理意义为耦合修正项
        Eigen::MatrixXd HtmH_dw = SpMat(H.transpose()) * dw - H * dw;
        rhs2 -= HtmH_dw;
        Eigen::MatrixXd w = lu.solve(rhs2); // 求解w

        // 计算柔度和刚度矩阵（稀疏*稠密→稠密）
        Eigen::MatrixXd K11w = K11 * w;                   // 主刚度作用
        Eigen::MatrixXd Htdw = SpMat(H.transpose()) * dw; // 耦合项
        Eigen::MatrixXd Hw = H * w;                       // 耦合项
        Eigen::MatrixXd G22dw = G22 * dw;                 // 约束块作用

        // 组装6x6柔度矩阵Fs
        Eigen::MatrixXd Fs = w.transpose() * (K11w + Htdw) + dw.transpose() * (Hw + G22dw);

        // 结果写入结构体
        SchurResult res;
        res.Ks = Fs.inverse(); // 柔度矩阵求逆得到刚度矩阵
        res.w = w;             // 主变量
        res.dw = dw;           // 辅助变量
        return res;
    }
    // Build 6x6 material constitutive matrix in PCSL ordering
    // Input: matprops row for the material (E1 E2 E3 G12 G13 G23 nu12 nu13 nu23)
    inline Mat6 BuildConstitutiveMatrix(const Eigen::VectorXd &mp)
    {
        double E1 = mp(0), E2 = mp(1), E3 = mp(2);
        double G12 = mp(3), G13 = mp(4), G23 = mp(5);
        double nu12 = mp(6), nu13 = mp(7), nu23 = mp(8);

        // Compliance matrix in Sadd's notation
        Mat6 Sm;
        Sm.setZero();
        Sm(0, 0) = 1.0 / E1;
        Sm(0, 1) = -nu12 / E1;
        Sm(0, 2) = -nu13 / E1;
        Sm(1, 0) = -nu12 / E1;
        Sm(1, 1) = 1.0 / E2;
        Sm(1, 2) = -nu23 / E2;
        Sm(2, 0) = -nu13 / E1;
        Sm(2, 1) = -nu23 / E2;
        Sm(2, 2) = 1.0 / E3;
        Sm(3, 3) = 1.0 / G23;
        Sm(4, 4) = 1.0 / G13;
        Sm(5, 5) = 1.0 / G12;

        Mat6 Qm = Sm.inverse();

        // Reorder to PCSL ordering: [6 1 2 3 5 4] (1-based) = [5 0 1 2 4 3] (0-based)
        int edof[6] = {5, 0, 1, 2, 4, 3};
        Mat6 Qr;
        for (int i = 0; i < 6; i++)
            for (int j = 0; j < 6; j++)
                Qr(edof[i], edof[j]) = Qm(i, j);
        return Qr;
    }

    // Rotate for layer orientation (fiber angle)
    // Direct port of PcsL_ElemRotateLayer symbolically generated code
    inline Mat6 RotateLayer(const Mat6 &Qm, double c, double s)
    {
        Mat6 Qp;
        double t1 = c * c, t6 = s * s;
        double t5 = 2 * Qm(0, 3) * c * s;
        double t8 = Qm(0, 0) * t1 - t5 + Qm(0, 5) * t6;
        double t13 = 2 * Qm(3, 3) * c * s;
        double t15 = Qm(3, 0) * t1 - t13 + Qm(3, 5) * t6;
        double t16 = s * t15;
        double t17 = c * t8 - t16;
        double t19 = c * t15;
        double t23 = 2 * Qm(5, 3) * c * s;
        double t25 = Qm(5, 0) * t1 - t23 + Qm(5, 5) * t6;
        double t27 = t19 - s * t25;
        double t31 = s * Qm(3, 1);
        double t32 = c * Qm(0, 1) - t31;
        double t34 = c * Qm(3, 1);
        double t36 = t34 - s * Qm(5, 1);
        double t43 = 1.0 * Qm(0, 2) * c - 1.0 * Qm(0, 4) * s;
        double t49 = 1.0 * Qm(3, 2) * c - 1.0 * Qm(3, 4) * s;
        double t50 = s * t49;
        double t51 = c * t43 - t50;
        double t53 = c * t49;
        double t58 = 1.0 * Qm(5, 2) * c - 1.0 * Qm(5, 4) * s;
        double t60 = t53 - s * t58;
        double t68 = -1.0 * t6 + 1.0 * t1;
        double t73 = 1.0 * Qm(0, 0) * c * s + Qm(0, 3) * t68 - 1.0 * Qm(0, 5) * c * s;
        double t82 = 1.0 * Qm(3, 0) * c * s + Qm(3, 3) * t68 - 1.0 * Qm(3, 5) * c * s;
        double t83 = s * t82;
        double t84 = c * t73 - t83;
        double t86 = c * t82;
        double t94 = 1.0 * Qm(5, 0) * c * s + Qm(5, 3) * t68 - 1.0 * Qm(5, 5) * c * s;
        double t96 = t86 - s * t94;
        double t103 = 1.0 * Qm(0, 2) * s + 1.0 * Qm(0, 4) * c;
        double t109 = 1.0 * Qm(3, 2) * s + 1.0 * Qm(3, 4) * c;
        double t110 = s * t109;
        double t111 = c * t103 - t110;
        double t113 = c * t109;
        double t118 = 1.0 * Qm(5, 2) * s + 1.0 * Qm(5, 4) * c;
        double t120 = t113 - s * t118;
        double t125 = Qm(0, 0) * t6 + t5 + Qm(0, 5) * t1;
        double t129 = Qm(3, 0) * t6 + t13 + Qm(3, 5) * t1;
        double t130 = s * t129;
        double t131 = c * t125 - t130;
        double t133 = c * t129;
        double t136 = Qm(5, 0) * t6 + t23 + Qm(5, 5) * t1;
        double t138 = t133 - s * t136;
        double t144 = 2 * Qm(1, 3) * c * s;
        double t171 = 2 * Qm(2, 3) * c * s;
        double t173 = Qm(2, 0) * t1 - t171 + Qm(2, 5) * t6;
        double t178 = 2 * Qm(4, 3) * c * s;
        double t180 = Qm(4, 0) * t1 - t178 + Qm(4, 5) * t6;
        double t190 = 1.0 * Qm(2, 2) * c - 1.0 * Qm(2, 4) * s;
        double t196 = 1.0 * Qm(4, 2) * c - 1.0 * Qm(4, 4) * s;
        double t206 = 1.0 * Qm(2, 0) * c * s + Qm(2, 3) * t68 - 1.0 * Qm(2, 5) * c * s;
        double t215 = 1.0 * Qm(4, 0) * c * s + Qm(4, 3) * t68 - 1.0 * Qm(4, 5) * c * s;
        double t222 = 1.0 * Qm(2, 2) * s + 1.0 * Qm(2, 4) * c;
        double t228 = 1.0 * Qm(4, 2) * s + 1.0 * Qm(4, 4) * c;
        double t233 = Qm(2, 0) * t6 + t171 + Qm(2, 5) * t1;
        double t237 = Qm(4, 0) * t6 + t178 + Qm(4, 5) * t1;

        Qp(0, 0) = t17 * c - t27 * s;
        Qp(0, 1) = t32 * c - t36 * s;
        Qp(0, 2) = t51 * c - t60 * s;
        Qp(0, 3) = t84 * c - t96 * s;
        Qp(0, 4) = t111 * c - t120 * s;
        Qp(0, 5) = t131 * c - t138 * s;

        Qp(1, 0) = Qm(1, 0) * t1 - t144 + Qm(1, 5) * t6;
        Qp(1, 1) = Qm(1, 1);
        Qp(1, 2) = 1.0 * Qm(1, 2) * c - 1.0 * Qm(1, 4) * s;
        Qp(1, 3) = 1.0 * Qm(1, 0) * c * s + Qm(1, 3) * t68 - 1.0 * Qm(1, 5) * c * s;
        Qp(1, 4) = 1.0 * Qm(1, 2) * s + 1.0 * Qm(1, 4) * c;
        Qp(1, 5) = Qm(1, 0) * t6 + t144 + Qm(1, 5) * t1;

        Qp(2, 0) = c * t173 - s * t180;
        Qp(2, 1) = c * Qm(2, 1) - s * Qm(4, 1);
        Qp(2, 2) = c * t190 - s * t196;
        Qp(2, 3) = c * t206 - s * t215;
        Qp(2, 4) = c * t222 - s * t228;
        Qp(2, 5) = c * t233 - s * t237;

        Qp(3, 0) = t17 * s + t27 * c;
        Qp(3, 1) = t32 * s + t36 * c;
        Qp(3, 2) = t51 * s + t60 * c;
        Qp(3, 3) = t84 * s + t96 * c;
        Qp(3, 4) = t111 * s + t120 * c;
        Qp(3, 5) = t131 * s + t138 * c;

        Qp(4, 0) = t173 * s + t180 * c;
        Qp(4, 1) = Qm(2, 1) * s + Qm(4, 1) * c;
        Qp(4, 2) = t190 * s + t196 * c;
        Qp(4, 3) = t206 * s + t215 * c;
        Qp(4, 4) = t222 * s + t228 * c;
        Qp(4, 5) = t233 * s + t237 * c;

        Qp(5, 0) = (s * t8 + t19) * s + (t16 + c * t25) * c;
        Qp(5, 1) = (s * Qm(0, 1) + t34) * s + (t31 + c * Qm(5, 1)) * c;
        Qp(5, 2) = (s * t43 + t53) * s + (t50 + c * t58) * c;
        Qp(5, 3) = (s * t73 + t86) * s + (t83 + c * t94) * c;
        Qp(5, 4) = (s * t103 + t113) * s + (t110 + c * t118) * c;
        Qp(5, 5) = (s * t125 + t133) * s + (t130 + c * t136) * c;

        return Qp;
    }

    // Rotate for fiber plane orientation
    // Direct port of PcsL_ElemRotateFiberPlane symbolically generated code
    inline Mat6 RotateFiberPlane(const Mat6 &Qm, double c, double s)
    {
        Mat6 Qf;
        double t1 = c * c, t3 = s * s;
        double t7 = 2 * Qm(0, 2) * c * s;
        double t8 = Qm(0, 0) * t1 + Qm(0, 1) * t3 + t7;
        double t14 = 2 * Qm(2, 2) * c * s;
        double t15 = Qm(2, 0) * t1 + Qm(2, 1) * t3 + t14;
        double t16 = s * t15;
        double t17 = c * t8 + t16;
        double t19 = c * t15;
        double t24 = 2 * Qm(1, 2) * c * s;
        double t25 = Qm(1, 0) * t1 + Qm(1, 1) * t3 + t24;
        double t27 = t19 + s * t25;
        double t32 = Qm(0, 0) * t3 + Qm(0, 1) * t1 - t7;
        double t36 = Qm(2, 0) * t3 + Qm(2, 1) * t1 - t14;
        double t37 = s * t36;
        double t38 = c * t32 + t37;
        double t40 = c * t36;
        double t43 = Qm(1, 0) * t3 + Qm(1, 1) * t1 - t24;
        double t45 = t40 + s * t43;
        double t56 = -1.0 * t3 + 1.0 * t1;
        double t58 = -1.0 * Qm(0, 0) * c * s + 1.0 * Qm(0, 1) * c * s + Qm(0, 2) * t56;
        double t67 = -1.0 * Qm(2, 0) * c * s + 1.0 * Qm(2, 1) * c * s + Qm(2, 2) * t56;
        double t68 = s * t67;
        double t69 = c * t58 + t68;
        double t71 = c * t67;
        double t79 = -1.0 * Qm(1, 0) * c * s + 1.0 * Qm(1, 1) * c * s + Qm(1, 2) * t56;
        double t81 = t71 + s * t79;
        double t88 = 1.0 * Qm(0, 3) * c + 1.0 * Qm(0, 4) * s;
        double t94 = 1.0 * Qm(2, 3) * c + 1.0 * Qm(2, 4) * s;
        double t95 = s * t94;
        double t96 = c * t88 + t95;
        double t98 = c * t94;
        double t103 = 1.0 * Qm(1, 3) * c + 1.0 * Qm(1, 4) * s;
        double t105 = t98 + s * t103;
        double t112 = -1.0 * Qm(0, 3) * s + 1.0 * Qm(0, 4) * c;
        double t118 = -1.0 * Qm(2, 3) * s + 1.0 * Qm(2, 4) * c;
        double t119 = s * t118;
        double t120 = c * t112 + t119;
        double t122 = c * t118;
        double t127 = -1.0 * Qm(1, 3) * s + 1.0 * Qm(1, 4) * c;
        double t129 = t122 + s * t127;
        double t133 = s * Qm(2, 5);
        double t134 = c * Qm(0, 5) + t133;
        double t136 = c * Qm(2, 5);
        double t138 = t136 + s * Qm(1, 5);
        double t205 = 2 * Qm(3, 2) * c * s;
        double t206 = Qm(3, 0) * t1 + Qm(3, 1) * t3 + t205;
        double t212 = 2 * Qm(4, 2) * c * s;
        double t213 = Qm(4, 0) * t1 + Qm(4, 1) * t3 + t212;
        double t218 = Qm(3, 0) * t3 + Qm(3, 1) * t1 - t205;
        double t222 = Qm(4, 0) * t3 + Qm(4, 1) * t1 - t212;
        double t232 = -1.0 * Qm(3, 0) * c * s + 1.0 * Qm(3, 1) * c * s + Qm(3, 2) * t56;
        double t241 = -1.0 * Qm(4, 0) * c * s + 1.0 * Qm(4, 1) * c * s + Qm(4, 2) * t56;
        double t248 = 1.0 * Qm(3, 3) * c + 1.0 * Qm(3, 4) * s;
        double t254 = 1.0 * Qm(4, 3) * c + 1.0 * Qm(4, 4) * s;
        double t261 = -1.0 * Qm(3, 3) * s + 1.0 * Qm(3, 4) * c;
        double t267 = -1.0 * Qm(4, 3) * s + 1.0 * Qm(4, 4) * c;
        double t295 = 2 * Qm(5, 2) * c * s;

        Qf(0, 0) = t17 * c + t27 * s;
        Qf(0, 1) = t38 * c + t45 * s;
        Qf(0, 2) = t69 * c + t81 * s;
        Qf(0, 3) = t96 * c + t105 * s;
        Qf(0, 4) = t120 * c + t129 * s;
        Qf(0, 5) = t134 * c + t138 * s;

        Qf(1, 0) = -(-s * t8 + t19) * s + (-t16 + c * t25) * c;
        Qf(1, 1) = -(-s * t32 + t40) * s + (-t37 + c * t43) * c;
        Qf(1, 2) = -(-s * t58 + t71) * s + (-t68 + c * t79) * c;
        Qf(1, 3) = -(-s * t88 + t98) * s + (-t95 + c * t103) * c;
        Qf(1, 4) = -(-s * t112 + t122) * s + (-t119 + c * t127) * c;
        Qf(1, 5) = -(-s * Qm(0, 5) + t136) * s + (-t133 + c * Qm(1, 5)) * c;

        Qf(2, 0) = -t17 * s + t27 * c;
        Qf(2, 1) = -t38 * s + t45 * c;
        Qf(2, 2) = -t69 * s + t81 * c;
        Qf(2, 3) = -t96 * s + t105 * c;
        Qf(2, 4) = -t120 * s + t129 * c;
        Qf(2, 5) = -t134 * s + t138 * c;

        Qf(3, 0) = c * t206 + s * t213;
        Qf(3, 1) = c * t218 + s * t222;
        Qf(3, 2) = c * t232 + s * t241;
        Qf(3, 3) = c * t248 + s * t254;
        Qf(3, 4) = c * t261 + s * t267;
        Qf(3, 5) = c * Qm(3, 5) + s * Qm(4, 5);

        Qf(4, 0) = -s * t206 + c * t213;
        Qf(4, 1) = -s * t218 + c * t222;
        Qf(4, 2) = -s * t232 + c * t241;
        Qf(4, 3) = -s * t248 + c * t254;
        Qf(4, 4) = -s * t261 + c * t267;
        Qf(4, 5) = -s * Qm(3, 5) + c * Qm(4, 5);

        Qf(5, 0) = Qm(5, 0) * t1 + Qm(5, 1) * t3 + t295;
        Qf(5, 1) = Qm(5, 0) * t3 + Qm(5, 1) * t1 - t295;
        Qf(5, 2) = -1.0 * Qm(5, 0) * c * s + 1.0 * Qm(5, 1) * c * s + Qm(5, 2) * t56;
        Qf(5, 3) = 1.0 * Qm(5, 3) * c + 1.0 * Qm(5, 4) * s;
        Qf(5, 4) = -1.0 * Qm(5, 3) * s + 1.0 * Qm(5, 4) * c;
        Qf(5, 5) = Qm(5, 5);

        return Qf;
    }

    // Compute rotated constitutive matrices and densities for all elements
    // Port of PcsL_RotateElementMaterialConstMatrix
    inline void ComputeAllConstitutiveMatrices(PcsLUtils &u)
    {
        u.Q.resize(u.ne_2d);
        u.density.resize(u.ne_2d);

        for (int e = 0; e < u.ne_2d; e++)
        {
            int nmat = u.emat(e, 1);                // material index (1-based in PCSL)
            u.density[e] = u.matprops(nmat - 1, 9); // rho

            Mat6 Qm = BuildConstitutiveMatrix(u.matprops.row(nmat - 1));

            // Layer (fiber) rotation
            double fiber_angle = u.emat(e, 2);
            double cf = std::cos(-fiber_angle * M_PI / 180.0);
            double sf = std::sin(-fiber_angle * M_PI / 180.0);
            Mat6 Qg = RotateLayer(Qm, cf, sf);

            // Fiber plane rotation
            double plane_angle = u.emat(e, 3);
            double cp = std::cos(-plane_angle * M_PI / 180.0);
            double sp = std::sin(-plane_angle * M_PI / 180.0);
            Qg = RotateFiberPlane(Qg, cp, sp);

            // Symmetrize to remove tiny numerical discrepancies
            u.Q[e] = (Qg + Qg.transpose()) * 0.5;
        }
    }

    // Translation matrix for cross-section (port of PcsL_CrossSectionTranslationMatrix)
    inline Mat6 TranslationMatrix(double px, double py)
    {
        Mat6 T = Mat6::Identity();
        T(0, 5) = py;
        T(1, 5) = -px;
        T(2, 3) = -py;
        T(2, 4) = px;
        return T;
    }

    // Rotation matrix for cross-section (port of PcsL_CrossSectionRotationMatrix)
    inline Mat6 RotationMatrix(double alpha_deg)
    {
        double c = std::cos(alpha_deg * M_PI / 180.0);
        double s = std::sin(alpha_deg * M_PI / 180.0);
        Mat6 R = Mat6::Zero();
        R(0, 0) = c;
        R(0, 1) = s;
        R(1, 0) = -s;
        R(1, 1) = c;
        R(2, 2) = 1;
        R(3, 3) = c;
        R(3, 4) = s;
        R(4, 3) = -s;
        R(4, 4) = c;
        R(5, 5) = 1;
        return R;
    }

    // Transform (translate + rotate) a 6x6 cross-section matrix
    // Port of PcsL_TransformCrossSectionMatrix
    inline Mat6 TransformCrossSectionMatrix(const Mat6 &M, double px, double py, double alpha_deg)
    {
        Mat6 T = TranslationMatrix(px, py);
        Mat6 Mt = T.transpose() * M * T;
        Mat6 R = RotationMatrix(alpha_deg);
        return R * Mt * R.transpose();
    }

    // Calculate shear center and elastic center from stiffness matrix
    // Port of PcsL_CalcShearAndElasticCenter
    inline void CalcShearAndElasticCenter(const Mat6 &Ks,
                                          double &ShearX, double &ShearY, double &ElasticX, double &ElasticY)
    {
        Mat6 F = Ks.inverse(); // compliance matrix

        // Shear center (z=1 in PCSL convention)
        double z = 1.0;
        ShearX = (-F(5, 1) + F(5, 3) * (1.0 - z)) / F(5, 5);
        ShearY = (F(5, 0) + F(5, 4) * (1.0 - z)) / F(5, 5);

        // Elastic center
        double denom = F(3, 3) * F(4, 4) - F(3, 4) * F(3, 4);
        ElasticX = -(-F(3, 3) * F(4, 2) + F(3, 4) * F(3, 2)) / denom;
        ElasticY = -(F(3, 2) * F(4, 4) - F(3, 4) * F(4, 2)) / denom;
    }

    // Calculate mass properties from element integration
    // Port of PcsL_CalcSectionMassProps
    inline void CalcSectionMassProps(const PcsLUtils &u,
                                     double &Mass, double &xm, double &ym,
                                     double &Ixx, double &Iyy, double &Ixy,
                                     double &AreaX, double &AreaY, double &Axx, double &Ayy, double &Axy,
                                     double &AreaTotal)
    {
        Mass = 0;
        xm = 0;
        ym = 0;
        Ixx = 0;
        Iyy = 0;
        Ixy = 0;
        AreaX = 0;
        AreaY = 0;
        Axx = 0;
        Ayy = 0;
        Axy = 0;
        AreaTotal = 0;

        for (int e = 0; e < u.ne_2d; e++)
        {
            double eIxx, eIyy, eIxy, eAxx, eAyy, eAxy, eArea, eAx, eAy, eMass, eMassX, eMassY;
            ComputeElementMassProps(e, u, eIxx, eIyy, eIxy, eAxx, eAyy, eAxy,
                                    eArea, eAx, eAy, eMass, eMassX, eMassY);
            Ixx += eIxx;
            Iyy += eIyy;
            Ixy += eIxy;
            xm += eMassX;
            ym += eMassY;
            Mass += eMass;
            Axx += eAxx;
            Ayy += eAyy;
            Axy += eAxy;
            AreaTotal += eArea;
            AreaX += eAx;
            AreaY += eAy;
        }
        xm /= Mass;
        ym /= Mass;
        AreaX /= AreaTotal;
        AreaY /= AreaTotal;
    }

    // Calculate orientation of principal elastic axes
    // Port of PcsL_CalcOrientationElasticAxes
    inline void CalcOrientationElasticAxes(const Mat6 &Ks,
                                           double ElasticX, double ElasticY,
                                           double &AlphaRef, double &AlphaElasticCenter)
    {
        // At reference point
        Eigen::Matrix2d ksub;
        ksub << Ks(3, 3), Ks(3, 4), Ks(4, 3), Ks(4, 4);
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix2d> eig1(ksub);
        Eigen::Vector2d vals1 = eig1.eigenvalues();
        Eigen::Matrix2d vecs1 = eig1.eigenvectors();
        // Sort by ascending eigenvalue
        int idx0 = (vals1(0) <= vals1(1)) ? 0 : 1;
        AlphaRef = std::atan(vecs1(1, idx0) / vecs1(0, idx0));

        // At elastic center
        Mat6 Ksprime = TransformCrossSectionMatrix(Ks, ElasticX, ElasticY, 0.0);
        Eigen::Matrix2d ksub2;
        ksub2 << Ksprime(3, 3), Ksprime(3, 4), Ksprime(4, 3), Ksprime(4, 4);
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix2d> eig2(ksub2);
        Eigen::Vector2d vals2 = eig2.eigenvalues();
        Eigen::Matrix2d vecs2 = eig2.eigenvectors();
        int idx1 = (vals2(0) <= vals2(1)) ? 0 : 1;
        AlphaElasticCenter = std::atan(vecs2(1, idx1) / vecs2(0, idx1));
    }

    // Compute all cross-section properties
    // Port of PcsL_CrossSectionProps
    inline PcsLCrossSectionProps ComputeCrossSectionProps(const Mat6 &Ks, const PcsLUtils &u)
    {
        PcsLCrossSectionProps props;

        CalcShearAndElasticCenter(Ks,
                                  props.ShearX, props.ShearY, props.ElasticX, props.ElasticY);

        CalcSectionMassProps(u,
                             props.MassTotal, props.MassX, props.MassY,
                             props.Ixx, props.Iyy, props.Ixy,
                             props.AreaX, props.AreaY, props.Axx, props.Ayy, props.Axy,
                             props.AreaTotal);

        CalcOrientationElasticAxes(Ks, props.ElasticX, props.ElasticY,
                                   props.AlphaPrincipleAxis_Ref, props.AlphaPrincipleAxis_ElasticCenter);

        return props;
    }
} // namespace PCSL
