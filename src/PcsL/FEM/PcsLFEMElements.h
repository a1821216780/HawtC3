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
#include <cmath>
#include <stdexcept>

namespace Qahse::PcsL::FEM
{

    using Mat6x12 = Eigen::Matrix<double, 6, 12>; ///< Q4单元的6x12矩阵类型
    using Mat6x24 = Eigen::Matrix<double, 6, 24>; ///< Q8单元的6x24矩阵类型
    using Mat6x18 = Eigen::Matrix<double, 6, 18>; ///< T6单元的6x18矩阵类型
    using Mat6 = Eigen::Matrix<double, 6, 6>;     ///< 6x6刚度/质量矩阵
    using Mat3 = Eigen::Matrix3d;                 ///< 3x3矩阵，常用于雅可比矩阵
    using Vec = Eigen::VectorXd;                  ///< 动态长度向量

    // ============================================================================
    //  Q4 Element (4-node bilinear quadrilateral, 3 DOF/node)
    // ============================================================================

    /// @brief 计算Q4单元的雅可比矩阵逆和行列式
    /// @param xi, eta 参考单元坐标
    /// @param pr 节点坐标数组 [x1,y1,x2,y2,...,x4,y4]
    /// @param[out] iJ 雅可比逆矩阵
    /// @param[out] detJ 雅可比行列式
    inline void Q4_Jacobian(double xi, double eta, const double *pr,
                            Mat3 &iJ, double &detJ)
    {
        double t1 = 1 - eta, t4 = 1 + eta;
        double t13 = 1 - xi, t15 = 1 + xi;
        double J11 = (-t1 * pr[0] + t1 * pr[2] + t4 * pr[4] - t4 * pr[6]) / 4.0;
        double J12 = (-t1 * pr[1] + t1 * pr[3] + t4 * pr[5] - t4 * pr[7]) / 4.0;
        double J21 = (-t13 * pr[0] - t15 * pr[2] + t15 * pr[4] + t13 * pr[6]) / 4.0;
        double J22 = (-t13 * pr[1] - t15 * pr[3] + t15 * pr[5] + t13 * pr[7]) / 4.0;
        detJ = J11 * J22 - J12 * J21;
        iJ.setIdentity();
        double inv = 1.0 / detJ;
        iJ(0, 0) = inv * J22;
        iJ(0, 1) = -inv * J12;
        iJ(1, 0) = -inv * J21;
        iJ(1, 1) = inv * J11;
    }

    /// @brief Q4单元的SN矩阵（6×12），用于质量矩阵等
    /// @param xi, eta 参考单元坐标
    /// @return 6x12矩阵
    inline Mat6x12 Q4_SNe(double xi, double eta)
    {
        Mat6x12 SN = Mat6x12::Zero();
        double t4 = (1 - xi) * (1 - eta) / 4.0;
        double t7 = (1 + xi) * (1 - eta) / 4.0;
        double t10 = (1 + xi) * (1 + eta) / 4.0;
        double t12 = (1 - xi) * (1 + eta) / 4.0;
        SN(3, 0) = t4;
        SN(3, 3) = t7;
        SN(3, 6) = t10;
        SN(3, 9) = t12;
        SN(4, 1) = t4;
        SN(4, 4) = t7;
        SN(4, 7) = t10;
        SN(4, 10) = t12;
        SN(5, 2) = t4;
        SN(5, 5) = t7;
        SN(5, 8) = t10;
        SN(5, 11) = t12;
        return SN;
    }

    /// @brief Q4单元的B矩阵（应变-位移，6×12）
    /// @param xi, eta 参考单元坐标
    /// @param iJ 雅可比逆矩阵
    /// @return 6x12矩阵
    inline Mat6x12 Q4_Be(double xi, double eta, const Mat3 &iJ)
    {
        Mat6x12 B = Mat6x12::Zero();
        double t1 = -1 + eta, t3 = -1 + xi;
        double t5 = (iJ(0, 0) * t1 + iJ(0, 1) * t3) / 4.0;
        double t7 = -1 - xi;
        double t9 = (-iJ(0, 0) * t1 + iJ(0, 1) * t7) / 4.0;
        double t10 = 1 + eta;
        double t13 = (iJ(0, 0) * t10 - iJ(0, 1) * t7) / 4.0;
        double t16 = (-iJ(0, 0) * t10 - iJ(0, 1) * t3) / 4.0;
        double t19 = (iJ(1, 0) * t1 + iJ(1, 1) * t3) / 4.0;
        double t22 = (-iJ(1, 0) * t1 + iJ(1, 1) * t7) / 4.0;
        double t25 = (iJ(1, 0) * t10 - iJ(1, 1) * t7) / 4.0;
        double t28 = (-iJ(1, 0) * t10 - iJ(1, 1) * t3) / 4.0;
        B(0, 0) = t5;
        B(0, 3) = t9;
        B(0, 6) = t13;
        B(0, 9) = t16;
        B(1, 1) = t19;
        B(1, 4) = t22;
        B(1, 7) = t25;
        B(1, 10) = t28;
        B(2, 0) = t19;
        B(2, 1) = t5;
        B(2, 3) = t22;
        B(2, 4) = t9;
        B(2, 6) = t25;
        B(2, 7) = t13;
        B(2, 9) = t28;
        B(2, 10) = t16;
        B(3, 2) = t5;
        B(3, 5) = t9;
        B(3, 8) = t13;
        B(3, 11) = t16;
        B(4, 2) = t19;
        B(4, 5) = t22;
        B(4, 8) = t25;
        B(4, 11) = t28;
        return B;
    }

    /// @brief Q4单元插值物理坐标（由参考坐标）
    /// @param pr 节点坐标数组
    /// @param xi, eta 参考单元坐标
    /// @param[out] x, y 物理坐标
    inline void Q4_InterpPos(const double *pr, double xi, double eta,
                             double &x, double &y)
    {
        double N1 = (1 - xi) * (1 - eta) / 4.0, N2 = (1 + xi) * (1 - eta) / 4.0;
        double N3 = (1 + xi) * (1 + eta) / 4.0, N4 = (1 - xi) * (1 + eta) / 4.0;
        x = N1 * pr[0] + N2 * pr[2] + N3 * pr[4] + N4 * pr[6];
        y = N1 * pr[1] + N2 * pr[3] + N3 * pr[5] + N4 * pr[7];
    }

    /// @brief 计算Q4单元的质量、刚度、耦合矩阵
    /// @param eidx 单元索引
    /// @param Qe 单元本构矩阵
    /// @param u 全局数据结构
    /// @param[out] Me 质量矩阵
    /// @param[out] Ee 刚度矩阵
    /// @param[out] Ce 耦合矩阵
    inline void Q4_Element(int eidx, const Mat6 &Qe, const PcsLUtils &u,
                           Eigen::MatrixXd &Me, Eigen::MatrixXd &Ee, Eigen::MatrixXd &Ce)
    {
        const auto &ei = u.elemInfo[0]; // Q4
        int gp = ei.gpoints;
        int mdim = ei.mdim; // 12
        Eigen::MatrixXd SNa(6 * gp, mdim);
        SNa.setZero();
        Eigen::MatrixXd Ba(6 * gp, mdim);
        Ba.setZero();
        Eigen::VectorXd gw(gp);

        for (int n = 0; n < gp; n++)
        {
            double xi = ei.xxg[n], eta = ei.yyg[n];
            Mat3 iJ;
            double detJ;
            Q4_Jacobian(xi, eta, u.pr_2d.col(eidx).data(), iJ, detJ);
            auto SN = Q4_SNe(xi, eta);
            auto B = Q4_Be(xi, eta, iJ);
            SNa.block(n * 6, 0, 6, mdim) = SN;
            Ba.block(n * 6, 0, 6, mdim) = B;
            gw(n) = ei.wg[n] * detJ;
        }
        // Qa = blkdiag(Qe*gw(1), ..., Qe*gw(gp))
        Eigen::MatrixXd Qa(6 * gp, 6 * gp);
        Qa.setZero();
        for (int n = 0; n < gp; n++)
            Qa.block(n * 6, n * 6, 6, 6) = Qe * gw(n);

        Me = (Qa * SNa).transpose() * SNa;
        Ee = Ba.transpose() * Qa * Ba;
        Ce = Ba.transpose() * Qa * SNa;
    }

    /// @brief 计算Q4单元的质量积分属性（惯性矩、面积、质心等）
    /// @param eidx 单元索引
    /// @param u 全局数据结构
    /// @param[out] Ixx, Iyy, Ixy 惯性矩
    /// @param[out] Axx, Ayy, Axy 面积二次矩
    /// @param[out] Area, Ax, Ay 面积及一阶矩
    /// @param[out] Mass, MassX, MassY 质量及质量一阶矩
    inline void Q4_ElementMassProps(int eidx, const PcsLUtils &u,
                                    double &Ixx, double &Iyy, double &Ixy,
                                    double &Axx, double &Ayy, double &Axy, double &Area,
                                    double &Ax, double &Ay, double &Mass, double &MassX, double &MassY)
    {
        Ixx = Iyy = Ixy = Axx = Ayy = Axy = Area = Ax = Ay = Mass = MassX = MassY = 0;
        const auto &ei = u.elemInfo[0];
        double rho = u.density[eidx];
        for (int n = 0; n < ei.gpoints; n++)
        {
            double xi = ei.xxg[n], eta = ei.yyg[n];
            Mat3 iJ;
            double detJ;
            Q4_Jacobian(xi, eta, u.pr_2d.col(eidx).data(), iJ, detJ);
            double x, y;
            Q4_InterpPos(u.pr_2d.col(eidx).data(), xi, eta, x, y);
            double w = ei.wg[n] * detJ;
            Iyy += x * x * rho * w;
            Ixx += y * y * rho * w;
            Ixy += x * y * rho * w;
            Ayy += x * x * w;
            Axx += y * y * w;
            Axy += x * y * w;
            Area += w;
            Ax += x * w;
            Ay += y * w;
            Mass += rho * w;
            MassX += x * rho * w;
            MassY += y * rho * w;
        }
    }

    // ============================================================================
    //  Q8 Element (8-node serendipity quadrilateral, 3 DOF/node)
    // ============================================================================

    /// @brief 计算Q8单元的雅可比矩阵逆和行列式
    /// @param xi, eta 参考单元坐标
    /// @param pr 节点坐标数组 [x1,y1,...,x8,y8]
    /// @param[out] iJ 雅可比逆矩阵
    /// @param[out] detJ 雅可比行列式
    inline void Q8_Jacobian(double xi, double eta, const double *pr,
                            Mat3 &iJ, double &detJ)
    {
        double t1 = eta / 4.0, t2 = eta * eta, t3 = t2 / 4.0;
        double t5 = xi * (1 - eta);
        double t6 = t5 / 2.0;
        double t7 = t1 - t3 + t6;
        double t9 = -t1 + t6 + t3;
        double t12 = xi * (1 + eta);
        double t13 = t12 / 2.0;
        double t14 = t1 + t13 + t3;
        double t16 = -t1 + t13 - t3;
        double t19 = 1 - t2;
        double t37 = xi / 4.0;
        double t39 = (1 - xi) * eta;
        double t40 = t39 / 2.0;
        double t41 = xi * xi;
        double t42 = t41 / 4.0;
        double t43 = t37 + t40 - t42;
        double t46 = (1 + xi) * eta;
        double t47 = t46 / 2.0;
        double t48 = -t37 - t42 + t47;
        double t50 = t37 + t42 + t47;
        double t52 = -t37 + t42 + t40;
        double t54 = 1 - t41;

        double J11 = t7 * pr[0] + t9 * pr[2] + t14 * pr[4] + t16 * pr[6] - t5 * pr[8] + t19 * pr[10] / 2.0 - t12 * pr[12] - t19 * pr[14] / 2.0;
        double J12 = t7 * pr[1] + t9 * pr[3] + t14 * pr[5] + t16 * pr[7] - t5 * pr[9] + t19 * pr[11] / 2.0 - t12 * pr[13] - t19 * pr[15] / 2.0;
        double J21 = t43 * pr[0] + t48 * pr[2] + t50 * pr[4] + t52 * pr[6] - t54 * pr[8] / 2.0 - t46 * pr[10] + t54 * pr[12] / 2.0 - t39 * pr[14];
        double J22 = t43 * pr[1] + t48 * pr[3] + t50 * pr[5] + t52 * pr[7] - t54 * pr[9] / 2.0 - t46 * pr[11] + t54 * pr[13] / 2.0 - t39 * pr[15];

        detJ = J11 * J22 - J12 * J21;
        iJ.setIdentity();
        double inv = 1.0 / detJ;
        iJ(0, 0) = inv * J22;
        iJ(0, 1) = -inv * J12;
        iJ(1, 0) = -inv * J21;
        iJ(1, 1) = inv * J11;
    }

    /// @brief Q8单元的SN矩阵（6×24）
    /// @param xi, eta 参考单元坐标
    /// @return 6x24矩阵
    inline Mat6x24 Q8_SNe(double xi, double eta)
    {
        Mat6x24 SN = Mat6x24::Zero();
        double t1 = 1 - xi, t2 = 1 - eta, t4 = eta * eta, t5 = 1 - t4, t6 = t1 * t5;
        double t7 = xi * xi, t8 = 1 - t7, t9 = t8 * t2, t10 = t1 * t2 - t6 - t9;
        double t11 = 1 + xi, t13 = t11 * t5, t14 = t11 * t2 - t9 - t13;
        double t15 = 1 + eta, t17 = t8 * t15, t18 = t11 * t15 - t17 - t13;
        double t20 = t1 * t15 - t17 - t6;
        double t21 = t9 / 2.0, t22 = t13 / 2.0, t23 = t17 / 2.0, t24 = t6 / 2.0;
        double N[8] = {t10 / 4.0, t14 / 4.0, t18 / 4.0, t20 / 4.0, t21, t22, t23, t24};
        for (int i = 0; i < 8; i++)
        {
            SN(3, i * 3) = N[i];
            SN(4, i * 3 + 1) = N[i];
            SN(5, i * 3 + 2) = N[i];
        }
        return SN;
    }

    /// @brief Q8单元的B矩阵（应变-位移，6×24）
    /// @param xi, eta 参考单元坐标
    /// @param iJ 雅可比逆矩阵
    /// @return 6x24矩阵
    inline Mat6x24 Q8_Be(double xi, double eta, const Mat3 &iJ)
    {
        Mat6x24 B = Mat6x24::Zero();
        double t1 = eta / 4.0, t2 = eta * eta, t3 = t2 / 4.0;
        double t4 = 1 - eta;
        double t6 = xi * t4 / 2.0;
        double t7 = t1 - t3 + t6;
        double t9 = xi / 4.0;
        double t10 = 1 - xi;
        double t12 = t10 * eta / 2.0;
        double t13 = xi * xi;
        double t14 = t13 / 4.0;
        double t15 = t9 + t12 - t14;

        double t18 = -t1 + t6 + t3;
        double t20 = 1 + xi;
        double t22 = t20 * eta / 2.0;
        double t23 = -t9 - t14 + t22;

        double t26 = 1 + eta;
        double t28 = xi * t26 / 2.0;
        double t29 = t1 + t28 + t3;
        double t31 = t9 + t14 + t22;

        double t34 = -t1 + t28 - t3;
        double t36 = -t9 + t14 + t12;

        double ixx = iJ(0, 0) * xi;
        double t41 = 1 - t13;
        double t43 = -ixx * t4 + iJ(0, 1) * t41 / 2.0;
        double t44 = 1 - t2;
        double t48 = iJ(0, 0) * t44 / 2.0 - iJ(0, 1) * t20 * eta;
        double t51 = -ixx * t26 - iJ(0, 1) * t41 / 2.0;
        double t55 = -iJ(0, 0) * t44 / 2.0 - iJ(0, 1) * t10 * eta;

        double iyx = iJ(1, 0) * xi;
        double t71 = -iyx * t4 + iJ(1, 1) * t41 / 2.0;
        double t75 = iJ(1, 0) * t44 / 2.0 - iJ(1, 1) * t20 * eta;
        double t78 = -iyx * t26 - iJ(1, 1) * t41 / 2.0;
        double t82 = -iJ(1, 0) * t44 / 2.0 - iJ(1, 1) * t10 * eta;

        double dNdx[8], dNdy[8];
        dNdx[0] = iJ(0, 0) * t7 + iJ(0, 1) * t15;
        dNdx[1] = iJ(0, 0) * t18 + iJ(0, 1) * t23;
        dNdx[2] = iJ(0, 0) * t29 + iJ(0, 1) * t31;
        dNdx[3] = iJ(0, 0) * t34 + iJ(0, 1) * t36;
        dNdx[4] = t43;
        dNdx[5] = t48;
        dNdx[6] = t51;
        dNdx[7] = t55;

        dNdy[0] = iJ(1, 0) * t7 + iJ(1, 1) * t15;
        dNdy[1] = iJ(1, 0) * t18 + iJ(1, 1) * t23;
        dNdy[2] = iJ(1, 0) * t29 + iJ(1, 1) * t31;
        dNdy[3] = iJ(1, 0) * t34 + iJ(1, 1) * t36;
        dNdy[4] = t71;
        dNdy[5] = t75;
        dNdy[6] = t78;
        dNdy[7] = t82;

        for (int i = 0; i < 8; i++)
        {
            B(0, i * 3) = dNdx[i];
            B(1, i * 3 + 1) = dNdy[i];
            B(2, i * 3) = dNdy[i];
            B(2, i * 3 + 1) = dNdx[i];
            B(3, i * 3 + 2) = dNdx[i];
            B(4, i * 3 + 2) = dNdy[i];
        }
        return B;
    }

    /// @brief Q8单元插值物理坐标（由参考坐标）
    /// @param pr 节点坐标数组
    /// @param xi, eta 参考单元坐标
    /// @param[out] x, y 物理坐标
    inline void Q8_InterpPos(const double *pr, double xi, double eta,
                             double &x, double &y)
    {
        double t1 = 1 - xi, t2 = 1 - eta, t4 = eta * eta, t5 = 1 - t4, t6 = t1 * t5;
        double t7 = xi * xi, t8 = 1 - t7, t9 = t8 * t2, t10 = t1 * t2 - t6 - t9;
        double t11 = 1 + xi, t13 = t11 * t5, t14 = t11 * t2 - t9 - t13;
        double t15 = 1 + eta, t17 = t8 * t15, t18 = t11 * t15 - t17 - t13;
        double t20 = t1 * t15 - t17 - t6;
        double N[8] = {t10 / 4.0, t14 / 4.0, t18 / 4.0, t20 / 4.0,
                       t9 / 2.0, t13 / 2.0, t17 / 2.0, t6 / 2.0};
        x = y = 0;
        for (int i = 0; i < 8; i++)
        {
            x += N[i] * pr[2 * i];
            y += N[i] * pr[2 * i + 1];
        }
    }

    /// @brief 计算Q8/Q8R单元的质量、刚度、耦合矩阵
    /// @param eidx 单元索引
    /// @param Qe 单元本构矩阵
    /// @param u 全局数据结构
    /// @param[out] Me 质量矩阵
    /// @param[out] Ee 刚度矩阵
    /// @param[out] Ce 耦合矩阵
    inline void Q8_Element(int eidx, const Mat6 &Qe, const PcsLUtils &u,
                           Eigen::MatrixXd &Me, Eigen::MatrixXd &Ee, Eigen::MatrixXd &Ce)
    {
        ElemType et = u.etype[eidx];
        const auto &ei = (et == Q8) ? u.elemInfo[1] : u.elemInfo[2]; // Q8 or Q8R
        int gp = ei.gpoints, mdim = ei.mdim;
        Eigen::MatrixXd SNa(6 * gp, mdim);
        SNa.setZero();
        Eigen::MatrixXd Ba(6 * gp, mdim);
        Ba.setZero();
        Eigen::VectorXd gw(gp);
        for (int n = 0; n < gp; n++)
        {
            Mat3 iJ;
            double detJ;
            Q8_Jacobian(ei.xxg[n], ei.yyg[n], u.pr_2d.col(eidx).data(), iJ, detJ);
            SNa.block(n * 6, 0, 6, mdim) = Q8_SNe(ei.xxg[n], ei.yyg[n]);
            Ba.block(n * 6, 0, 6, mdim) = Q8_Be(ei.xxg[n], ei.yyg[n], iJ);
            gw(n) = ei.wg[n] * detJ;
        }
        Eigen::MatrixXd Qa(6 * gp, 6 * gp);
        Qa.setZero();
        for (int n = 0; n < gp; n++)
            Qa.block(n * 6, n * 6, 6, 6) = Qe * gw(n);
        Me = (Qa * SNa).transpose() * SNa;
        Ee = Ba.transpose() * Qa * Ba;
        Ce = Ba.transpose() * Qa * SNa;
    }

    /// @brief 计算Q8/Q8R单元的质量积分属性（惯性矩、面积、质心等）
    /// @param eidx 单元索引
    /// @param u 全局数据结构
    /// @param[out] Ixx, Iyy, Ixy 惯性矩
    /// @param[out] Axx, Ayy, Axy 面积二次矩
    /// @param[out] Area, Ax, Ay 面积及一阶矩
    /// @param[out] Mass, MassX, MassY 质量及质量一阶矩
    inline void Q8_ElementMassProps(int eidx, const PcsLUtils &u,
                                    double &Ixx, double &Iyy, double &Ixy,
                                    double &Axx, double &Ayy, double &Axy, double &Area,
                                    double &Ax, double &Ay, double &Mass, double &MassX, double &MassY)
    {
        Ixx = Iyy = Ixy = Axx = Ayy = Axy = Area = Ax = Ay = Mass = MassX = MassY = 0;
        ElemType et = u.etype[eidx];
        const auto &ei = (et == Q8) ? u.elemInfo[1] : u.elemInfo[2];
        double rho = u.density[eidx];
        for (int n = 0; n < ei.gpoints; n++)
        {
            Mat3 iJ;
            double detJ;
            Q8_Jacobian(ei.xxg[n], ei.yyg[n], u.pr_2d.col(eidx).data(), iJ, detJ);
            double x, y;
            Q8_InterpPos(u.pr_2d.col(eidx).data(), ei.xxg[n], ei.yyg[n], x, y);
            double w = ei.wg[n] * detJ;
            Iyy += x * x * rho * w;
            Ixx += y * y * rho * w;
            Ixy += x * y * rho * w;
            Ayy += x * x * w;
            Axx += y * y * w;
            Axy += x * y * w;
            Area += w;
            Ax += x * w;
            Ay += y * w;
            Mass += rho * w;
            MassX += x * rho * w;
            MassY += y * rho * w;
        }
    }

    // ============================================================================
    //  T6 Element (6-node quadratic triangle, 3 DOF/node)
    // ============================================================================

    // T6 uses area coordinates (L1, L2). Third coord L3 = 1 - L1 - L2.
    // 形函数：角点 N_i = L_i*(2*L_i - 1)，中点 N_ij = 4*L_i*L_j
    // 节点顺序: 1(L1=1), 2(L2=1), 3(L3=1), 4(1-2中点), 5(2-3中点), 6(3-1中点)

    /// @brief 计算T6单元的雅可比矩阵逆和行列式
    /// @param L1, L2 面积坐标
    /// @param pr 节点坐标数组 [x1,y1,...,x6,y6]
    /// @param[out] iJ 雅可比逆矩阵
    /// @param[out] detJ 雅可比行列式
    inline void T6_Jacobian(double L1, double L2, const double *pr,
                            Mat3 &iJ, double &detJ)
    {
        double L3 = 1.0 - L1 - L2;
        // dN/dL1, dN/dL2 for 6 nodes
        double dNdL1[6] = {4 * L1 - 1, 0, -(4 * L3 - 1), 4 * L2, -4 * L2, 4 * (L3 - L1)};
        double dNdL2[6] = {0, 4 * L2 - 1, -(4 * L3 - 1), 4 * L1, 4 * (L3 - L2), -4 * L1};

        double J11 = 0, J12 = 0, J21 = 0, J22 = 0;
        for (int i = 0; i < 6; i++)
        {
            J11 += dNdL1[i] * pr[2 * i];
            J12 += dNdL1[i] * pr[2 * i + 1];
            J21 += dNdL2[i] * pr[2 * i];
            J22 += dNdL2[i] * pr[2 * i + 1];
        }
        detJ = J11 * J22 - J12 * J21;
        iJ.setIdentity();
        double inv = 1.0 / detJ;
        iJ(0, 0) = inv * J22;
        iJ(0, 1) = -inv * J12;
        iJ(1, 0) = -inv * J21;
        iJ(1, 1) = inv * J11;
    }

    /// @brief T6单元的SN矩阵（6×18）
    /// @param L1, L2 面积坐标
    /// @return 6x18矩阵
    inline Mat6x18 T6_SNe(double L1, double L2)
    {
        Mat6x18 SN = Mat6x18::Zero();
        double L3 = 1.0 - L1 - L2;
        double N[6] = {L1 * (2 * L1 - 1), L2 * (2 * L2 - 1), L3 * (2 * L3 - 1),
                       4 * L1 * L2, 4 * L2 * L3, 4 * L3 * L1};
        for (int i = 0; i < 6; i++)
        {
            SN(3, i * 3) = N[i];
            SN(4, i * 3 + 1) = N[i];
            SN(5, i * 3 + 2) = N[i];
        }
        return SN;
    }

    /// @brief T6单元的B矩阵（应变-位移，6×18）
    /// @param L1, L2 面积坐标
    /// @param iJ 雅可比逆矩阵
    /// @return 6x18矩阵
    inline Mat6x18 T6_Be(double L1, double L2, const Mat3 &iJ)
    {
        Mat6x18 B = Mat6x18::Zero();
        double L3 = 1.0 - L1 - L2;
        double dNdL1[6] = {4 * L1 - 1, 0, -(4 * L3 - 1), 4 * L2, -4 * L2, 4 * (L3 - L1)};
        double dNdL2[6] = {0, 4 * L2 - 1, -(4 * L3 - 1), 4 * L1, 4 * (L3 - L2), -4 * L1};
        double dNdx[6], dNdy[6];
        for (int i = 0; i < 6; i++)
        {
            dNdx[i] = iJ(0, 0) * dNdL1[i] + iJ(0, 1) * dNdL2[i];
            dNdy[i] = iJ(1, 0) * dNdL1[i] + iJ(1, 1) * dNdL2[i];
        }
        for (int i = 0; i < 6; i++)
        {
            B(0, i * 3) = dNdx[i];
            B(1, i * 3 + 1) = dNdy[i];
            B(2, i * 3) = dNdy[i];
            B(2, i * 3 + 1) = dNdx[i];
            B(3, i * 3 + 2) = dNdx[i];
            B(4, i * 3 + 2) = dNdy[i];
        }
        return B;
    }

    /// @brief T6单元插值物理坐标（由面积坐标）
    /// @param pr 节点坐标数组
    /// @param L1, L2 面积坐标
    /// @param[out] x, y 物理坐标
    inline void T6_InterpPos(const double *pr, double L1, double L2,
                             double &x, double &y)
    {
        double L3 = 1.0 - L1 - L2;
        double N[6] = {L1 * (2 * L1 - 1), L2 * (2 * L2 - 1), L3 * (2 * L3 - 1),
                       4 * L1 * L2, 4 * L2 * L3, 4 * L3 * L1};
        x = y = 0;
        for (int i = 0; i < 6; i++)
        {
            x += N[i] * pr[2 * i];
            y += N[i] * pr[2 * i + 1];
        }
    }

    /// @brief 计算T6单元的质量、刚度、耦合矩阵
    /// @param eidx 单元索引
    /// @param Qe 单元本构矩阵
    /// @param u 全局数据结构
    /// @param[out] Me 质量矩阵
    /// @param[out] Ee 刚度矩阵
    /// @param[out] Ce 耦合矩阵
    inline void T6_Element(int eidx, const Mat6 &Qe, const PcsLUtils &u,
                           Eigen::MatrixXd &Me, Eigen::MatrixXd &Ee, Eigen::MatrixXd &Ce)
    {
        const auto &ei = u.elemInfo[3]; // T6
        int gp = ei.gpoints, mdim = ei.mdim;
        Eigen::MatrixXd SNa(6 * gp, mdim);
        SNa.setZero();
        Eigen::MatrixXd Ba(6 * gp, mdim);
        Ba.setZero();
        Eigen::VectorXd gw(gp);
        for (int n = 0; n < gp; n++)
        {
            double L1 = ei.xxg[n], L2 = ei.yyg[n];
            Mat3 iJ;
            double detJ;
            T6_Jacobian(L1, L2, u.pr_2d.col(eidx).data(), iJ, detJ);
            SNa.block(n * 6, 0, 6, mdim) = T6_SNe(L1, L2);
            Ba.block(n * 6, 0, 6, mdim) = T6_Be(L1, L2, iJ);
            // T6 weight includes 0.5 factor (triangle area in parametric space)
            gw(n) = ei.wg[n] * detJ * 0.5;
        }
        Eigen::MatrixXd Qa(6 * gp, 6 * gp);
        Qa.setZero();
        for (int n = 0; n < gp; n++)
            Qa.block(n * 6, n * 6, 6, 6) = Qe * gw(n);
        Me = (Qa * SNa).transpose() * SNa;
        Ee = Ba.transpose() * Qa * Ba;
        Ce = Ba.transpose() * Qa * SNa;
    }

    /// @brief 计算T6单元的质量积分属性（惯性矩、面积、质心等）
    /// @param eidx 单元索引
    /// @param u 全局数据结构
    /// @param[out] Ixx, Iyy, Ixy 惯性矩
    /// @param[out] Axx, Ayy, Axy 面积二次矩
    /// @param[out] Area, Ax, Ay 面积及一阶矩
    /// @param[out] Mass, MassX, MassY 质量及质量一阶矩
    inline void T6_ElementMassProps(int eidx, const PcsLUtils &u,
                                    double &Ixx, double &Iyy, double &Ixy,
                                    double &Axx, double &Ayy, double &Axy, double &Area,
                                    double &Ax, double &Ay, double &Mass, double &MassX, double &MassY)
    {
        Ixx = Iyy = Ixy = Axx = Ayy = Axy = Area = Ax = Ay = Mass = MassX = MassY = 0;
        const auto &ei = u.elemInfo[3];
        double rho = u.density[eidx];
        for (int n = 0; n < ei.gpoints; n++)
        {
            double L1 = ei.xxg[n], L2 = ei.yyg[n];
            Mat3 iJ;
            double detJ;
            T6_Jacobian(L1, L2, u.pr_2d.col(eidx).data(), iJ, detJ);
            double x, y;
            T6_InterpPos(u.pr_2d.col(eidx).data(), L1, L2, x, y);
            double w = ei.wg[n] * detJ * 0.5;
            Iyy += x * x * rho * w;
            Ixx += y * y * rho * w;
            Ixy += x * y * rho * w;
            Ayy += x * x * w;
            Axx += y * y * w;
            Axy += x * y * w;
            Area += w;
            Ax += x * w;
            Ay += y * w;
            Mass += rho * w;
            MassX += x * rho * w;
            MassY += y * rho * w;
        }
    }

    // ============================================================================
    //  Dispatch element calculation by type
    // ============================================================================

    /// @brief 按单元类型调度计算单元矩阵
    /// @param eidx 单元索引
    /// @param Qe 单元本构矩阵
    /// @param u 全局数据结构
    /// @param[out] Me 质量矩阵
    /// @param[out] Ee 刚度矩阵
    /// @param[out] Ce 耦合矩阵
    inline void ComputeElementMatrices(int eidx, const Mat6 &Qe, const PcsLUtils &u,
                                       Eigen::MatrixXd &Me, Eigen::MatrixXd &Ee,
                                       Eigen::MatrixXd &Ce)
    {
        switch (u.etype[eidx])
        {
        case Q4:
            Q4_Element(eidx, Qe, u, Me, Ee, Ce);
            break;
        case Q8:
        case Q8R:
            Q8_Element(eidx, Qe, u, Me, Ee, Ce);
            break;
        case T6:
            T6_Element(eidx, Qe, u, Me, Ee, Ce);
            break;
        }
    }

    /// @brief 按单元类型调度计算单元质量积分属性
    /// @param eidx 单元索引
    /// @param u 全局数据结构
    /// @param[out] Ixx, Iyy, Ixy 惯性矩
    /// @param[out] Axx, Ayy, Axy 面积二次矩
    /// @param[out] Area, Ax, Ay 面积及一阶矩
    /// @param[out] Mass, MassX, MassY 质量及质量一阶矩
    inline void ComputeElementMassProps(int eidx, const PcsLUtils &u,
                                        double &Ixx, double &Iyy, double &Ixy,
                                        double &Axx, double &Ayy, double &Axy, double &Area,
                                        double &Ax, double &Ay, double &Mass, double &MassX, double &MassY)
    {
        switch (u.etype[eidx])
        {
        case Q4:
            Q4_ElementMassProps(eidx, u, Ixx, Iyy, Ixy, Axx, Ayy, Axy, Area, Ax, Ay, Mass, MassX, MassY);
            break;
        case Q8:
        case Q8R:
            Q8_ElementMassProps(eidx, u, Ixx, Iyy, Ixy, Axx, Ayy, Axy, Area, Ax, Ay, Mass, MassX, MassY);
            break;
        case T6:
            T6_ElementMassProps(eidx, u, Ixx, Iyy, Ixy, Axx, Ayy, Axy, Area, Ax, Ay, Mass, MassX, MassY);
            break;
        }
    }

} // namespace PCSL
