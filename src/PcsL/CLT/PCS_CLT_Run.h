#pragma once
//**********************************************************************************************************************************
// LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of Qahse.PcsL.CLT
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

#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <Eigen/Dense>

#include "../Pcs_Types.h"
#include "Pcs_CLT_Subs.h"
#include "../../IO/Math/LinearAlgebraHelper.h"
#include "../../IO/Math/MathHelper.h"
#include "../../IO/Math/InterpolateHelper.h"
#include "../../IO/IO/PhysicalParameters.h"
#include "../../IO/Log/LogHelper.h"
#include "../../IO/System/Console.h"

namespace Qahse::PcsL::CLT
{
    using Qahse::IO::System::ConsoleColor;
    using Eigen::VectorXd;
    using Eigen::Matrix2d;
    using Qahse::IO::Math::LinearAlgebraHelper;
    using Qahse::IO::Math::MathHelper;
    using Qahse::IO::Math::InterpolateHelper;
    using Qahse::IO::IO::PhysicalParameters;
    using Qahse::IO::Log::LogHelper;

    // ===================================================================
    //  CLT 输出结构体
    // ===================================================================

    /// @brief CLT 分析结果输出结构体，包含截面力学特性
    struct CLTOutput
    {
        double EiFlap = 0;  ///< 挥舞刚度 [N·m²]
        double EiLag = 0;   ///< 摆振刚度 [N·m²]
        double GJ = 0;      ///< 扭转刚度 [N·m²/rad]
        double EA = 0;      ///< 轴向刚度 [N]
        double SFl = 0;     ///< 挥舞方向耦合刚度 [N·m]
        double SAf = 0;     ///< 挥舞-轴向耦合 [N·m]
        double SAl = 0;     ///< 摆振-轴向耦合 [N·m]
        double SFt = 0;     ///< 挥舞-扭转耦合 [N·m]
        double SLt = 0;     ///< 摆振-扭转耦合 [N·m]
        double SAt = 0;     ///< 轴向-扭转耦合 [N·m]
        double XSc = 0;     ///< 剪切中心 X 坐标 [m]
        double YSc = 0;     ///< 剪切中心 Y 坐标 [m]
        double XTc = 0;     ///< 扭转中心 X 坐标 [m]
        double YTc = 0;     ///< 扭转中心 Y 坐标 [m]
        double Mass = 0;    ///< 单位长度质量 [kg/m]
        double FlapIner = 0; ///< 挥舞惯性矩 [kg·m²]
        double LagIner = 0;  ///< 摆振惯性矩 [kg·m²]
        double TwInerD = 0;  ///< 扭转惯性角 [度]
        double XCm = 0;     ///< 质心 X 坐标 [m]
        double YCm = 0;     ///< 质心 Y 坐标 [m]
        double str_tw = 0;  ///< 结构扭转角 [rad]
    };

    // ===================================================================
    //  CLT 结果输出函数
    // ===================================================================

    /// @brief 将 CLT 分析结果写入文件
    /// @param pre CLT 分析输入数据
    /// @param output CLT 分析结果数组
    inline void WriteCLTResults(const PcsLInput& pre, const std::vector<CLTOutput>& output)
    {
        namespace fs = std::filesystem;

        std::string outDir = pre.SumPath.empty()
            ? (fs::path(pre.path).parent_path() / "Result1").string()
            : pre.SumPath;
        fs::create_directories(outDir);

        std::string filename = outDir + "/CLT_Results.out";
        std::ofstream ofs(filename);
        if (!ofs.is_open())
        {
            LogHelper::ErrorLog("无法创建输出文件: " + filename, "", "", 20, "WriteCLTResults");
            return;
        }

        ofs << "============================================================\n";
        ofs << "  PcsL CLT (Classical Laminate Theory) Analysis Results\n";
        ofs << "============================================================\n\n";

        ofs << std::scientific << std::setprecision(6);

        // 表头
        ofs << std::setw(8)  << "Section"
            << std::setw(16) << "SpanLoc[m]"
            << std::setw(16) << "EiFlap[N*m2]"
            << std::setw(16) << "EiLag[N*m2]"
            << std::setw(16) << "GJ[N*m2/rad]"
            << std::setw(16) << "EA[N]"
            << std::setw(16) << "Mass[kg/m]"
            << std::setw(16) << "XSc[m]"
            << std::setw(16) << "YSc[m]"
            << std::setw(16) << "XTc[m]"
            << std::setw(16) << "YTc[m]"
            << std::setw(16) << "XCm[m]"
            << std::setw(16) << "YCm[m]"
            << std::setw(16) << "FlapIner"
            << std::setw(16) << "LagIner"
            << std::setw(16) << "TwInerD[deg]"
            << "\n";

        for (int i = 0; i < pre.SectionCount; i++)
        {
            ofs << std::setw(8)  << (i + 1)
                << std::setw(16) << pre.Sections[i].SpanLocation
                << std::setw(16) << output[i].EiFlap
                << std::setw(16) << output[i].EiLag
                << std::setw(16) << output[i].GJ
                << std::setw(16) << output[i].EA
                << std::setw(16) << output[i].Mass
                << std::setw(16) << output[i].XSc
                << std::setw(16) << output[i].YSc
                << std::setw(16) << output[i].XTc
                << std::setw(16) << output[i].YTc
                << std::setw(16) << output[i].XCm
                << std::setw(16) << output[i].YCm
                << std::setw(16) << output[i].FlapIner
                << std::setw(16) << output[i].LagIner
                << std::setw(16) << output[i].TwInerD
                << "\n";
        }

        ofs.close();
        std::cout << "  CLT results written to: " << filename << std::endl;
    }

    // ===================================================================
    //  辅助：检查 VectorXd 是否包含某值
    // ===================================================================

    /// @brief 检查 VectorXd 中是否包含指定值（精确比较）
    inline bool VectorContains(const VectorXd& v, double x)
    {
        for (int i = 0; i < v.size(); i++)
        {
            if (v[i] == x) return true;
        }
        return false;
    }

    // ===================================================================
    //  CLT 主分析函数
    // ===================================================================

    /// @brief 执行经典层合板理论（CLT）分析，计算复合材料叶片的截面刚度和惯性特性
    /// @details 实现经典层合板理论，用于分析复合材料叶片的力学性能。
    ///          主要功能包括：处理多截面叶片几何和材料属性、计算旋转刚度矩阵、
    ///          确定剪切中心和扭转中心位置、计算截面刚度参数和质量惯性特性。
    /// @param pre 预处理数据对象（非const引用，翼型节点数组可能被修改以插入腹板/扇区边界节点）
    /// @return CLT 分析结果向量，每个截面对应一个 CLTOutput
    inline std::vector<CLTOutput> RunCLT(PcsLInput& pre)
    {
        std::cout << "  Running PcsL.CLT ..." << std::endl;

        int naf = pre.SectionCount;
        std::vector<CLTOutput> output(naf);
        double eps = 1E-5;

        // 拒绝应力计算模式
        if (pre.CalStress)
        {
            LogHelper::ErrorLog("层合板理论不支持计算铺层之间的应力！", "", "PCSL.CLT", 20, "RunCLT");
            return output;
        }

        // 提取扭转角和展向位置数组
        VectorXd twist(naf);
        VectorXd sloc(naf);
        for (int i = 0; i < naf; i++)
        {
            twist[i] = pre.Sections[i].TwistAngle;
            sloc[i] = pre.Sections[i].SpanLocation;
        }

        // 计算扭转角变化率
        VectorXd tw_prime_d = VectorXd::Zero(naf);
        if (naf > 1)
        {
            tw_prime_d = tw_rate(sloc, twist);
        }

        // 检查输入数据
        CheckPreComl(pre);

        if (naf < 2)
        {
            LogHelper::ErrorLog(
                "The number of sections is less than 2, please check the input file.",
                "", "", 20, "RunCLT");
            return output;
        }

        // 提取材料属性并预计算 Q 矩阵
        int nmat = pre.MaterialTypeCount;
        VectorXd anu12(nmat), e1(nmat), e2(nmat), g12(nmat), density(nmat);
        for (int m = 0; m < nmat; m++)
        {
            anu12[m] = pre.Material[m].Nu12;
            e1[m] = pre.Material[m].E1;
            e2[m] = pre.Material[m].E2;
            g12[m] = pre.Material[m].G12;
            density[m] = pre.Material[m].Density;
        }

        // 计算层合板刚度矩阵系数
        VectorXd anud = VectorXd::Ones(nmat) - (anu12.array() * anu12.array() * e2.array() / e1.array()).matrix();
        VectorXd q11 = (e1.array() / anud.array()).matrix();
        VectorXd q22 = (e2.array() / anud.array()).matrix();
        VectorXd q12 = (anu12.array() * e2.array() / anud.array()).matrix();
        VectorXd q66 = g12;

        std::vector<double> weby_u(pre.Nweb, 0.0);
        std::vector<double> weby_l(pre.Nweb, 0.0);

        // ===================================================================
        //  截面循环
        // ===================================================================
        for (int i = 0; i < pre.SectionCount; i++)
        {
            std::cout << "  PcsL.CLT BEGIN ANALYSIS BLADE STATION " << i << std::endl;
            double tphip = tw_prime_d[i] * PhysicalParameters::D2R;
            std::vector<double> locw(pre.Nweb, 0.0);

            // ---------------------------------------------------------------
            //  腹板处理：在翼型坐标数组中插入腹板位置节点
            // ---------------------------------------------------------------
            if (i >= pre.Ib_sp_stn - 1 && i <= pre.Ob_sp_stn - 1)
            {
                for (int j = 0; j < pre.Nweb; j++)
                {
                    double rle = pre.Sections[i].AerodynamicCenter;
                    double r1w = pre.Sections[pre.Ib_sp_stn - 1].AerodynamicCenter;
                    double r2w = pre.Sections[pre.Ob_sp_stn - 1].AerodynamicCenter;
                    double p1w = pre.Webs[j].InnerChordPosition;
                    double p2w = pre.Webs[j].OuterChordPosition;
                    double ch1 = pre.Sections[pre.Ib_sp_stn - 1].ChordLength;
                    double ch2 = pre.Sections[pre.Ob_sp_stn - 1].ChordLength;
                    double x1w = pre.Sections[pre.Ib_sp_stn - 1].SpanLocation;
                    double l_web = pre.Sections[pre.Ob_sp_stn - 1].SpanLocation - x1w;
                    double xlocn = (pre.Sections[i].SpanLocation - x1w) / l_web;
                    double ch = pre.Sections[i].ChordLength;
                    locw[j] = rle - (r1w - p1w) * ch1 * (1.0 - xlocn) / ch
                                  - (r2w - p2w) * ch2 * xlocn / ch;

                    // 上翼面：插入腹板位置节点
                    double ynd = InterpolateHelper::Interp1D(
                        pre.Sections[i].Airfoil.Xu,
                        pre.Sections[i].Airfoil.Yu,
                        pre.Webs[j].InnerChordPosition);

                    if (!VectorContains(pre.Sections[i].Airfoil.Xu, pre.Webs[j].InnerChordPosition))
                    {
                        InsertNodeIfAbsent(
                            pre.Sections[i].Airfoil.Xu,
                            pre.Sections[i].Airfoil.Yu,
                            pre.Webs[j].InnerChordPosition, ynd);
                        weby_u[j] = ynd;
                    }

                    // 下翼面：插入腹板位置节点
                    ynd = InterpolateHelper::Interp1D(
                        pre.Sections[i].Airfoil.Xl,
                        pre.Sections[i].Airfoil.Yl,
                        pre.Webs[j].OuterChordPosition);

                    if (!VectorContains(pre.Sections[i].Airfoil.Xl, pre.Webs[j].OuterChordPosition))
                    {
                        InsertNodeIfAbsent(
                            pre.Sections[i].Airfoil.Xl,
                            pre.Sections[i].Airfoil.Yl,
                            pre.Webs[j].OuterChordPosition, ynd);
                        weby_l[j] = ynd;
                    }
                }

                // 检查腹板是否在扇区范围内
                if (locw[0] < pre.Sections[i].TopConfiguration.xsec_node[0] ||
                    locw[0] < pre.Sections[i].LowConfiguration.xsec_node[0])
                {
                    LogHelper::ErrorLog(
                        "first web out of sectors-bounded airfoil at section " + std::to_string(i + 1),
                        "", "", 20, "CheckPreComl");
                }
                if (locw[pre.Nweb - 1] > pre.Sections[i].TopConfiguration.xsec_node.back() ||
                    locw[pre.Nweb - 1] > pre.Sections[i].LowConfiguration.xsec_node.back())
                {
                    LogHelper::ErrorLog(
                        "last web out of sectors-bounded airfoil at section " + std::to_string(i + 1),
                        "", "", 20, "CheckPreComl");
                }
            }

            // ---------------------------------------------------------------
            //  扇区边界插值（逐点插值，避免批量版本的尺寸不匹配问题）
            // ---------------------------------------------------------------
            const auto& xsn_top = pre.Sections[i].TopConfiguration.xsec_node;
            const auto& xsn_low = pre.Sections[i].LowConfiguration.xsec_node;
            std::vector<double> ynu(xsn_top.size());
            std::vector<double> ynl(xsn_low.size());
            for (size_t j = 0; j < xsn_top.size(); j++)
            {
                ynu[j] = InterpolateHelper::Interp1D(
                    pre.Sections[i].Airfoil.Xu,
                    pre.Sections[i].Airfoil.Yu, xsn_top[j]);
            }
            for (size_t j = 0; j < xsn_low.size(); j++)
            {
                ynl[j] = InterpolateHelper::Interp1D(
                    pre.Sections[i].Airfoil.Xl,
                    pre.Sections[i].Airfoil.Yl, xsn_low[j]);
            }

            // 前缘闭合检查
            if (std::abs(pre.Sections[i].TopConfiguration.xsec_node[0] -
                         pre.Sections[i].LowConfiguration.xsec_node[0]) > eps)
            {
                LogHelper::WarnLog(
                    "the leading edge may be open; check closure at section " + std::to_string(i + 1),
                    "", ConsoleColor::DarkYellow, 0, "CheckPreComl");
            }
            else
            {
                if (ynu[0] - ynl[0] > eps)
                {
                    int wreq = 1;
                    if (i >= pre.Ib_sp_stn - 1 && i <= pre.Ob_sp_stn - 1)
                    {
                        if (std::abs(pre.Sections[i].TopConfiguration.xsec_node[0] - locw[0]) < eps)
                        {
                            wreq = 0;
                        }
                    }
                    if (wreq == 1)
                    {
                        LogHelper::WarnLog(
                            "open leading edge; check web requirement at section " + std::to_string(i + 1),
                            "", ConsoleColor::DarkYellow, 0, "CheckPreComl");
                    }
                }
            }

            // ---------------------------------------------------------------
            //  插入扇区边界节点到翼型数组
            // ---------------------------------------------------------------
            // 上翼面
            for (int j = 0; j < pre.Sections[i].TopConfiguration.N_scts; j++)
            {
                double xsn = pre.Sections[i].TopConfiguration.xsec_node[j];
                double newy = InterpolateHelper::Interp1D(
                    pre.Sections[i].Airfoil.Xu,
                    pre.Sections[i].Airfoil.Yu, xsn);

                if (!VectorContains(pre.Sections[i].Airfoil.Xu, xsn))
                {
                    InsertNodeIfAbsent(
                        pre.Sections[i].Airfoil.Xu,
                        pre.Sections[i].Airfoil.Yu, xsn, newy);
                }
            }
            // 下翼面
            for (int j = 0; j < pre.Sections[i].LowConfiguration.N_scts; j++)
            {
                double xsn = pre.Sections[i].LowConfiguration.xsec_node[j];
                double newy = InterpolateHelper::Interp1D(
                    pre.Sections[i].Airfoil.Xl,
                    pre.Sections[i].Airfoil.Yl, xsn);

                if (!VectorContains(pre.Sections[i].Airfoil.Xl, xsn))
                {
                    InsertNodeIfAbsent(
                        pre.Sections[i].Airfoil.Xl,
                        pre.Sections[i].Airfoil.Yl, xsn, newy);
                }
            }

            // 后缘闭合检查
            if (std::abs(pre.Sections[i].TopConfiguration.xsec_node.back() -
                         pre.Sections[i].LowConfiguration.xsec_node.back()) > eps)
            {
                LogHelper::WarnLog(
                    "the trailing edge may be open; check closure at section " + std::to_string(i + 1),
                    "", ConsoleColor::DarkYellow, 0, "CheckPreComl");
            }
            else
            {
                if (ynu.back() - ynl.back() > eps)
                {
                    int wreq = 1;
                    if (i >= pre.Ib_sp_stn - 1 && i <= pre.Ob_sp_stn - 1)
                    {
                        if (std::abs(pre.Sections[i].TopConfiguration.xsec_node.back() -
                                     locw[pre.Nweb - 1]) < eps)
                        {
                            wreq = 0;
                        }
                    }
                    if (wreq == 1)
                    {
                        LogHelper::WarnLog(
                            "open trailing edge; check web requirement at section " + std::to_string(i + 1),
                            "", ConsoleColor::DarkYellow, 0, "CheckPreComl");
                    }
                }
            }

            // ---------------------------------------------------------------
            //  计算分段信息
            // ---------------------------------------------------------------
            int nseg_u = static_cast<int>(pre.Sections[i].Airfoil.Xu.size()) - 1;
            int nseg_l = static_cast<int>(pre.Sections[i].Airfoil.Xl.size()) - 1;
            int nseg_p = nseg_u + nseg_l;
            int nseg = nseg_p;
            if (pre.Sections[i].WebConfiguration.N_scts != 0)
            {
                nseg = nseg_p + pre.Sections[i].WebConfiguration.N_scts;
            }

            int ndl1 = MathHelper::FindIndex(
                pre.Sections[i].Airfoil.Xl,
                pre.Sections[i].LowConfiguration.xsec_node[0]) + 1;
            int ndu1 = MathHelper::FindIndex(
                pre.Sections[i].Airfoil.Xu,
                pre.Sections[i].TopConfiguration.xsec_node[0]) + 1;

            std::vector<int> n_scts = {
                pre.Sections[i].TopConfiguration.N_scts,
                pre.Sections[i].LowConfiguration.N_scts
            };

            SegInfoResult segResult = SegInfo(
                pre, eps,
                pre.Sections[i].ChordLength, pre.Sections[i].AerodynamicCenter,
                nseg, nseg_u, nseg_p,
                pre.Sections[i].Airfoil.Xu, pre.Sections[i].Airfoil.Yu,
                pre.Sections[i].Airfoil.Xl, pre.Sections[i].Airfoil.Yl,
                ndl1, ndu1, locw, weby_u, weby_l, n_scts, i);

            // ---------------------------------------------------------------
            //  第一次积分：确定剪切中心
            // ---------------------------------------------------------------
            double eabar = 0.0;
            double q11ya = 0.0;
            double q11za = 0.0;
            double sigma = 0.0;

            for (int iseg = 0; iseg < nseg_p; iseg++)
            {
                int surfaceType = segResult.isur[iseg];
                int sectionId = segResult.idsect[iseg] - 1;
                double ysg = segResult.yseg[iseg];
                double zsg = segResult.zseg[iseg];
                double w = segResult.wseg[iseg];
                double sths = segResult.sthseg[iseg];
                double cths = segResult.cthseg[iseg];

                int nlam = 0;
                if (surfaceType == 1)
                    nlam = pre.Sections[i].TopConfiguration.Sector[sectionId].N_laminas;
                else if (surfaceType == 2)
                    nlam = pre.Sections[i].LowConfiguration.Sector[sectionId].N_laminas;

                double tbar = 0.0;
                double q11t = 0.0;
                double q11yt_u = 0.0, q11zt_u = 0.0;
                double q11yt_l = 0.0, q11zt_l = 0.0;

                for (int ilam = 0; ilam < nlam; ilam++)
                {
                    double t = 0.0, thp = 0.0;
                    int mat = 0;

                    if (surfaceType == 1)
                    {
                        t = pre.Sections[i].TopConfiguration.Sector[sectionId].Laminae[ilam].Thickness
                            * pre.Sections[i].TopConfiguration.Sector[sectionId].Laminae[ilam].PlyCount;
                        thp = pre.Sections[i].TopConfiguration.Sector[sectionId].Laminae[ilam].FiberOrientation
                            * PhysicalParameters::D2R;
                        mat = pre.Sections[i].TopConfiguration.Sector[sectionId].Laminae[ilam].MaterialId - 1;
                    }
                    else if (surfaceType == 2)
                    {
                        t = pre.Sections[i].LowConfiguration.Sector[sectionId].Laminae[ilam].Thickness
                            * pre.Sections[i].LowConfiguration.Sector[sectionId].Laminae[ilam].PlyCount;
                        thp = pre.Sections[i].LowConfiguration.Sector[sectionId].Laminae[ilam].FiberOrientation
                            * PhysicalParameters::D2R;
                        mat = pre.Sections[i].LowConfiguration.Sector[sectionId].Laminae[ilam].MaterialId - 1;
                    }

                    tbar += t / 2.0;

                    double sign = (surfaceType % 2 == 1) ? -1.0 : 1.0;
                    double y0 = ysg - (sign * tbar * sths);
                    double z0 = zsg + (sign * tbar * cths);

                    QBarResult qbars = Q_Bars(mat, thp, density, q11, q22, q12, q66);
                    Matrix2d qtil = QTildas(qbars.qbar11, qbars.qbar22, qbars.qbar12,
                                           qbars.qbar16, qbars.qbar26, qbars.qbar66,
                                           mat + 1);

                    double qtil11t = qtil(0, 0) * t;
                    q11t += qtil11t;

                    if (iseg < nseg_u)
                    {
                        q11yt_u += qtil11t * y0;
                        q11zt_u += qtil11t * z0;
                    }
                    else
                    {
                        q11yt_l += qtil11t * y0;
                        q11zt_l += qtil11t * z0;
                    }

                    tbar += t / 2.0;
                }

                double signTotal = (surfaceType % 2 == 1) ? -1.0 : 1.0;
                sigma += w * std::abs(zsg + signTotal * 0.5 * tbar * cths) * cths;
                eabar += q11t * w;
                q11ya += (q11yt_u + q11yt_l) * w;
                q11za += (q11zt_u + q11zt_l) * w;
            }

            double y_sc = q11ya / eabar;
            double z_sc = q11za / eabar;

            // ---------------------------------------------------------------
            //  第二次积分：计算刚度和惯性特性
            // ---------------------------------------------------------------
            eabar = 0.0;
            q11ya = 0.0;
            q11za = 0.0;
            double ap = 0.0, bp = 0.0, cp = 0.0, dp = 0.0, ep = 0.0;
            double q11ysqa = 0.0, q11zsqa = 0.0, q11yza = 0.0;
            double mass = 0.0;
            double rhoya = 0.0, rhoza = 0.0;
            double rhoysqa = 0.0, rhozsqa = 0.0, rhoyza = 0.0;

            for (int iseg = 0; iseg < nseg; iseg++)
            {
                int surfaceType = segResult.isur[iseg];
                int idsec = segResult.idsect[iseg];
                double ysg = segResult.yseg[iseg];
                double zsg = segResult.zseg[iseg];
                double w = segResult.wseg[iseg];
                double sths = segResult.sthseg[iseg];
                double cths = segResult.cthseg[iseg];
                double s2ths = segResult.s2thseg[iseg];
                double c2ths = segResult.c2thseg[iseg];

                int nlam = 0;
                if (surfaceType == 1)
                    nlam = pre.Sections[i].TopConfiguration.Sector[idsec - 1].N_laminas;
                else if (surfaceType == 2)
                    nlam = pre.Sections[i].LowConfiguration.Sector[idsec - 1].N_laminas;
                else if (surfaceType == 0)
                    nlam = pre.Sections[i].WebConfiguration.Sector[idsec - 1].N_laminas;
                else
                    LogHelper::ErrorLog("未知结构", "", "", 20, "RunCLT");

                double tbar = 0.0;
                double q11t = 0.0, q11yt = 0.0, q11zt = 0.0;
                double dtbar = 0.0, q2bar = 0.0;
                double zbart = 0.0, ybart = 0.0, tbart = 0.0;
                double q11ysqt = 0.0, q11zsqt = 0.0, q11yzt = 0.0;
                double rhot = 0.0, rhoyt = 0.0, rhozt = 0.0;
                double rhoysqt = 0.0, rhozsqt = 0.0, rhoyzt = 0.0;

                for (int ilam = 0; ilam < nlam; ilam++)
                {
                    double t = 0.0, thp = 0.0;
                    int mat = 0;
                    double y0 = 0.0, z0 = 0.0;

                    if (surfaceType == 1)
                    {
                        t = pre.Sections[i].TopConfiguration.Sector[idsec - 1].Laminae[ilam].Thickness
                            * pre.Sections[i].TopConfiguration.Sector[idsec - 1].Laminae[ilam].PlyCount;
                        thp = pre.Sections[i].TopConfiguration.Sector[idsec - 1].Laminae[ilam].FiberOrientation
                            * PhysicalParameters::D2R;
                        mat = pre.Sections[i].TopConfiguration.Sector[idsec - 1].Laminae[ilam].MaterialId - 1;
                        tbar += t / 2.0;
                        double sign = -1.0; // (-1)^1 = -1
                        y0 = ysg - sign * tbar * sths - y_sc;
                        z0 = zsg + sign * tbar * cths - z_sc;
                    }
                    else if (surfaceType == 2)
                    {
                        t = pre.Sections[i].LowConfiguration.Sector[idsec - 1].Laminae[ilam].Thickness
                            * pre.Sections[i].LowConfiguration.Sector[idsec - 1].Laminae[ilam].PlyCount;
                        thp = pre.Sections[i].LowConfiguration.Sector[idsec - 1].Laminae[ilam].FiberOrientation
                            * PhysicalParameters::D2R;
                        mat = pre.Sections[i].LowConfiguration.Sector[idsec - 1].Laminae[ilam].MaterialId - 1;
                        tbar += t / 2.0;
                        double sign = 1.0; // (-1)^2 = 1
                        y0 = ysg - sign * tbar * sths - y_sc;
                        z0 = zsg + sign * tbar * cths - z_sc;
                    }
                    else if (surfaceType == 0)
                    {
                        t = pre.Sections[i].WebConfiguration.Sector[idsec - 1].Laminae[ilam].Thickness
                            * pre.Sections[i].WebConfiguration.Sector[idsec - 1].Laminae[ilam].PlyCount;
                        thp = pre.Sections[i].WebConfiguration.Sector[idsec - 1].Laminae[ilam].FiberOrientation
                            * PhysicalParameters::D2R;
                        mat = pre.Sections[i].WebConfiguration.Sector[idsec - 1].Laminae[ilam].MaterialId - 1;
                        tbar += t / 2.0;
                        y0 = ysg - tbar / 2.0 - y_sc;
                        z0 = zsg - z_sc;
                    }

                    double y0sq = y0 * y0;
                    double z0sq = z0 * z0;

                    QBarResult qbars = Q_Bars(mat, thp, density, q11, q22, q12, q66);
                    Matrix2d qtil = QTildas(qbars.qbar11, qbars.qbar22, qbars.qbar12,
                                           qbars.qbar16, qbars.qbar26, qbars.qbar66,
                                           mat + 1);

                    double ieta1 = (t * t) / 12.0;
                    double izeta1 = (w * w) / 12.0;
                    double iepz = 0.5 * (ieta1 + izeta1);
                    double iemz = 0.5 * (ieta1 - izeta1);
                    double ipp = iepz + (iemz * c2ths);
                    double iqq = iepz - (iemz * c2ths);
                    double ipq = iemz * s2ths;

                    double qtil11t = qtil(0, 0) * t;
                    double rot = qbars.rho_m * t;

                    if (surfaceType == 1 || surfaceType == 2)
                    {
                        double qtil12t = qtil(0, 1) * t;
                        double qtil22t = qtil(1, 1) * t;

                        q11t += qtil11t;
                        q11yt += qtil11t * y0;
                        q11zt += qtil11t * z0;

                        dtbar += qtil12t * (y0sq + z0sq) * tphip * t;
                        q2bar += qtil22t;
                        zbart += z0 * qtil12t;
                        ybart += y0 * qtil12t;
                        tbart += qtil12t;

                        q11ysqt += qtil11t * (y0sq + iqq);
                        q11zsqt += qtil11t * (z0sq + ipp);
                        q11yzt += qtil11t * (y0 * z0 + ipq);

                        rhot += rot;
                        rhoyt += rot * y0;
                        rhozt += rot * z0;
                        rhoysqt += rot * (y0sq + iqq);
                        rhozsqt += rot * (z0sq + ipp);
                        rhoyzt += rot * (y0 * z0 + ipq);
                    }
                    else if (surfaceType == 0)
                    {
                        q11t += qtil11t;
                        q11yt += qtil11t * y0;
                        q11zt += qtil11t * z0;
                        q11ysqt += qtil11t * (y0sq + iqq);
                        q11zsqt += qtil11t * (z0sq + ipp);
                        q11yzt += qtil11t * (y0 * z0 + ipq);

                        rhot += rot;
                        rhoyt += rot * y0;
                        rhozt += rot * z0;
                        rhoysqt += rot * (y0sq + iqq);
                        rhozsqt += rot * (z0sq + ipp);
                        rhoyzt += rot * (y0 * z0 + ipq);
                    }

                    tbar += t / 2.0;
                } // 结束铺层循环

                // 累加分段贡献
                eabar += q11t * w;
                q11ya += q11yt * w;
                q11za += q11zt * w;
                q11ysqa += q11ysqt * w;
                q11zsqa += q11zsqt * w;
                q11yza += q11yzt * w;

                if (surfaceType == 1 || surfaceType == 2)
                {
                    double wdq2bar = w / q2bar;
                    ap += wdq2bar;
                    bp += wdq2bar * tbart;
                    cp += wdq2bar * dtbar;
                    dp += wdq2bar * zbart;
                    ep += wdq2bar * ybart;
                }

                mass += rhot * w;
                rhoya += rhoyt * w;
                rhoza += rhozt * w;
                rhoysqa += rhoysqt * w;
                rhozsqa += rhozsqt * w;
                rhoyza += rhoyzt * w;
            } // 结束分段循环

            // ---------------------------------------------------------------
            //  后处理：计算输出量
            // ---------------------------------------------------------------
            double y_tc = q11ya / eabar;
            double z_tc = q11za / eabar;

            double sfbar = q11za;
            double slbar = q11ya;
            double eifbar = q11zsqa;
            double eilbar = q11ysqa;
            double eiflbar = q11yza;

            double sigm2 = sigma * 2.0;
            double gjbar = sigm2 * (sigm2 + cp) / ap;
            double sftbar = -sigm2 * dp / ap;
            double sltbar = -sigm2 * ep / ap;
            double satbar = sigm2 * bp / ap;

            double ycm_sc = rhoya / mass;
            double zcm_sc = rhoza / mass;

            double iflap_sc = rhozsqa;
            double ilag_sc = rhoysqa;
            double ifl_sc = rhoyza;

            double ytc_ref = y_tc + y_sc;
            double ztc_ref = z_tc + z_sc;
            double ycm_ref = ycm_sc + y_sc;
            double zcm_ref = zcm_sc + z_sc;

            // ---------------------------------------------------------------
            //  主惯性轴计算
            // ---------------------------------------------------------------
            double iflap_cm = iflap_sc - mass * zcm_sc * zcm_sc;
            double ilag_cm = ilag_sc - mass * ycm_sc * ycm_sc;
            double ifl_cm = ifl_sc - mass * ycm_sc * zcm_sc;
            double m_inertia = 0.5 * (ilag_cm + iflap_cm);
            double r_inertia = std::sqrt(0.25 * std::pow(ilag_cm - iflap_cm, 2) + ifl_cm * ifl_cm);

            double iflap_eta = 0.0, ilag_zeta = 0.0;
            if (iflap_cm <= ilag_cm)
            {
                iflap_eta = m_inertia - r_inertia;
                ilag_zeta = m_inertia + r_inertia;
            }
            else
            {
                iflap_eta = m_inertia + r_inertia;
                ilag_zeta = m_inertia - r_inertia;
            }

            double th_pa = 0.0;
            constexpr double epsilon = 1e-6;
            if (std::abs(ilag_cm - iflap_cm) < eps)
            {
                th_pa = M_PI / 4.0;
                if (std::abs(ifl_cm / iflap_cm) < epsilon)
                {
                    th_pa = 0.0;
                }
            }
            else
            {
                th_pa = 0.5 * std::abs(std::atan(2.0 * ifl_cm / (ilag_cm - iflap_cm)));
            }

            // 调整 th_pa 符号
            if (std::abs(ifl_cm) < eps)
            {
                th_pa = 0.0;
            }
            else
            {
                if (iflap_cm >= ilag_cm)
                    th_pa = (ifl_cm > 0.0) ? -th_pa : th_pa;
                else
                    th_pa = (ifl_cm < 0.0) ? -th_pa : th_pa;
            }

            // ---------------------------------------------------------------
            //  主刚度轴计算
            // ---------------------------------------------------------------
            double em_stiff = 0.5 * (eilbar + eifbar);
            double er_stiff = std::sqrt(0.25 * std::pow(eilbar - eifbar, 2) + std::pow(eiflbar, 2));
            double pflap_stff = 0.0, plag_stff = 0.0;
            double the_pa = 0.0;

            if (eifbar <= eilbar)
            {
                pflap_stff = em_stiff - er_stiff;
                plag_stff = em_stiff + er_stiff;
            }
            else
            {
                pflap_stff = em_stiff + er_stiff;
                plag_stff = em_stiff - er_stiff;
            }

            if (std::abs(eilbar - eifbar) < eps)
            {
                the_pa = M_PI / 4.0;
            }
            else
            {
                the_pa = 0.5 * std::abs(std::atan(2.0 * eiflbar / (eilbar - eifbar)));
            }

            if (std::abs(eiflbar) > eps)
            {
                if (eifbar >= eilbar)
                    the_pa = (eiflbar > 0) ? -the_pa : the_pa;
                else
                    the_pa = (eiflbar < 0) ? -the_pa : the_pa;
            }
            else
            {
                the_pa = 0.0;
            }

            // ---------------------------------------------------------------
            //  坐标系转换（风力涡轮机坐标系）
            // ---------------------------------------------------------------
            int id_form = 1;
            double tw_iner_d = 0.0;
            double str_tw = 0.0;

            if (id_form == 1)
            {
                tw_iner_d = twist[i] * PhysicalParameters::D2R - th_pa;
                str_tw = twist[i] * PhysicalParameters::D2R - the_pa;
                y_sc = -y_sc;
                ytc_ref = -ytc_ref;
                ycm_ref = -ycm_ref;
            }
            else
            {
                tw_iner_d = twist[i] * PhysicalParameters::D2R + th_pa;
                str_tw = twist[i] * PhysicalParameters::D2R + the_pa;
            }

            // 单位转换和符号调整
            eiflbar = -eiflbar;
            sfbar = -sfbar;
            sltbar = -sltbar;
            tw_iner_d *= PhysicalParameters::R2D;

            // ---------------------------------------------------------------
            //  输出赋值
            // ---------------------------------------------------------------
            output[i].EiFlap = eifbar;
            output[i].EiLag = eilbar;
            output[i].GJ = gjbar;
            output[i].EA = eabar;
            output[i].SFl = eiflbar;
            output[i].SAf = sfbar;
            output[i].SAl = slbar;
            output[i].SFt = sftbar;
            output[i].SLt = sltbar;
            output[i].SAt = satbar;
            output[i].XSc = z_sc;
            output[i].YSc = y_sc;
            output[i].XTc = ztc_ref;
            output[i].YTc = ytc_ref;
            output[i].Mass = mass;
            output[i].FlapIner = iflap_eta;
            output[i].LagIner = ilag_zeta;
            output[i].TwInerD = tw_iner_d;
            output[i].XCm = zcm_ref;
            output[i].YCm = ycm_ref;
            output[i].str_tw = str_tw;
        } // 结束截面循环

        // 输出结果
        WriteCLTResults(pre, output);
        std::cout << std::endl;

        return output;
    }

} // namespace Qahse::PcsL::CLT
