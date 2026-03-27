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
#include <algorithm>
#include <Eigen/Dense>

#include "../PcsL_Types.h"
#include "../../IO/Math/LinearAlgebraHelper.h"
#include "../../IO/Math/MathHelper.h"
#include "../../IO/Math/InterpolateHelper.h"
#include "../../IO/IO/PhysicalParameters.h"
#include "../../IO/Log/LogHelper.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
namespace Qahse::PcsL::CLT
{
    using Eigen::VectorXd;
    using Eigen::Matrix2d;
    using Qahse::IO::Math::LinearAlgebraHelper;
    using Qahse::IO::Math::MathHelper;
    using Qahse::IO::Math::InterpolateHelper;
    using Qahse::IO::IO::PhysicalParameters;
    using Qahse::IO::Log::LogHelper;

    // ===================================================================
    //  CLT 辅助结构体
    // ===================================================================

    /// @brief Q_Bars 函数返回值：旋转后的刚度矩阵分量及材料密度
    struct QBarResult
    {
        double qbar11 = 0; ///< 旋转后的 Q̄₁₁ 分量 [Pa]
        double qbar22 = 0; ///< 旋转后的 Q̄₂₂ 分量 [Pa]
        double qbar12 = 0; ///< 旋转后的 Q̄₁₂ 分量 [Pa]
        double qbar16 = 0; ///< 旋转后的 Q̄₁₆ 分量 [Pa]
        double qbar26 = 0; ///< 旋转后的 Q̄₂₆ 分量 [Pa]
        double qbar66 = 0; ///< 旋转后的 Q̄₆₆ 分量 [Pa]
        double rho_m = 0;  ///< 材料密度 [kg/m³]
    };

    /// @brief SegInfo 函数返回值：翼型剖面各分段几何信息
    struct SegInfoResult
    {
        std::vector<int> isur;       ///< 表面类型数组（0=腹板，1=上表面，2=下表面）
        std::vector<int> idsect;     ///< 截面ID数组（1-based索引）
        std::vector<double> yseg;    ///< 分段中点的y坐标数组
        std::vector<double> zseg;    ///< 分段中点的z坐标数组
        std::vector<double> wseg;    ///< 分段长度数组
        std::vector<double> sthseg;  ///< 分段角度正弦值数组
        std::vector<double> cthseg;  ///< 分段角度余弦值数组
        std::vector<double> s2thseg; ///< 分段角度2倍正弦值数组
        std::vector<double> c2thseg; ///< 分段角度2倍余弦值数组
    };

    // ===================================================================
    //  CLT 辅助函数
    // ===================================================================

    /// @brief 计算气动扭转角沿叶片展向位置的变化率
    /// @details 使用有限差分法：内部点采用非均匀网格中心差分，边界点采用前向/后向差分。
    ///          输入角度为度，内部转换为弧度计算后再转回度。
    /// @param sloc 叶片展向位置向量（升序排列，至少2个元素）
    /// @param tw_aero_d 各展向位置处的气动扭转角向量 [度]
    /// @return 各展向位置处的扭转角变化率 [度/单位长度]
    inline VectorXd tw_rate(const VectorXd& sloc, const VectorXd& tw_aero_d)
    {
        int naf = static_cast<int>(tw_aero_d.size());
        VectorXd tw_aero = tw_aero_d * PhysicalParameters::D2R;
        VectorXd th_prime = VectorXd::Zero(naf);

        // 中间点：非均匀网格中心差分
        for (int i = 1; i < naf - 1; i++)
        {
            double f0 = tw_aero[i];
            double f1 = tw_aero[i - 1];
            double f2 = tw_aero[i + 1];
            double h1 = sloc[i] - sloc[i - 1];
            double h2 = sloc[i + 1] - sloc[i];
            th_prime[i] = (h1 * (f2 - f0) + h2 * (f0 - f1)) / (2.0 * h1 * h2);
        }

        // 边界点：前向/后向差分
        th_prime[0] = (tw_aero[1] - tw_aero[0]) / (sloc[1] - sloc[0]);
        th_prime[naf - 1] = (tw_aero[naf - 1] - tw_aero[naf - 2]) / (sloc[naf - 1] - sloc[naf - 2]);

        // 弧度转度
        for (int i = 0; i < naf; i++)
        {
            th_prime[i] *= PhysicalParameters::R2D;
        }

        return th_prime;
    }

    /// @brief 计算复合材料层合板在指定角度下的旋转刚度矩阵分量（Q̄矩阵）及材料密度
    /// @details 经典层合板理论坐标变换公式，将材料坐标系刚度转换到全局坐标系。
    /// @param mat 材料索引（0-based，用于从属性向量中取值）
    /// @param thp 铺层角度 [弧度]
    /// @param density 材料密度向量 [kg/m³]
    /// @param q11 材料 Q₁₁ 向量 [Pa]
    /// @param q22 材料 Q₂₂ 向量 [Pa]
    /// @param q12 材料 Q₁₂ 向量 [Pa]
    /// @param q66 材料 Q₆₆ 向量 [Pa]
    /// @return QBarResult 旋转后的刚度分量及密度
    inline QBarResult Q_Bars(
        int mat, double thp,
        const VectorXd& density,
        const VectorXd& q11, const VectorXd& q22,
        const VectorXd& q12, const VectorXd& q66)
    {
        double st = std::sin(thp);
        double ct = std::cos(thp);

        double c2t = ct * ct;
        double c3t = c2t * ct;
        double c4t = c2t * c2t;
        double s2t = st * st;
        double s3t = s2t * st;
        double s4t = s2t * s2t;

        double s2thsq = 4.0 * s2t * c2t;

        double k11 = q11[mat];
        double k22 = q22[mat];
        double k12 = q12[mat];
        double k66 = q66[mat];

        double kmm = k11 - k12 - 2.0 * k66;
        double kmp = k12 - k22 + 2.0 * k66;

        QBarResult r;
        r.qbar11 = k11 * c4t + 0.5 * (k12 + 2.0 * k66) * s2thsq + k22 * s4t;
        r.qbar22 = k11 * s4t + 0.5 * (k12 + 2.0 * k66) * s2thsq + k22 * c4t;
        r.qbar12 = 0.25 * (k11 + k22 - 4.0 * k66) * s2thsq + k12 * (s4t + c4t);
        r.qbar16 = kmm * st * c3t + kmp * s3t * ct;
        r.qbar26 = kmm * s3t * ct + kmp * st * c3t;
        r.qbar66 = 0.25 * (kmm + k22 - k12) * s2thsq + k66 * (s4t + c4t);
        r.rho_m = density[mat];

        return r;
    }

    /// @brief 计算复合材料层合板理论中的简化刚度矩阵 Q̃（Q-tilde）
    /// @details 消除 Q̄₂₂ 分量，将完整刚度矩阵简化为 2×2 对称矩阵。
    ///          自动检查 Q̄₂₂≠0 及 Q̃₁₁>0（物理可行性）。
    /// @param qbar11 Q̄₁₁ 分量
    /// @param qbar22 Q̄₂₂ 分量
    /// @param qbar12 Q̄₁₂ 分量
    /// @param qbar16 Q̄₁₆ 分量
    /// @param qbar26 Q̄₂₆ 分量
    /// @param qbar66 Q̄₆₆ 分量
    /// @param mat 材料编号（1-based，用于错误提示）
    /// @return 2×2 对称简化刚度矩阵 Q̃
    inline Matrix2d QTildas(
        double qbar11, double qbar22, double qbar12,
        double qbar16, double qbar26, double qbar66,
        int mat)
    {
        Matrix2d qtil;
        qtil.setZero();

        if (qbar22 == 0.0)
        {
            LogHelper::ErrorLog("The Dev num qbar22 is 0!", "", "", 20, "QTildas");
        }

        qtil(0, 0) = qbar11 - (qbar12 * qbar12) / qbar22;

        if (qtil(0, 0) < 0.0)
        {
            LogHelper::ErrorLog(
                "Material " + std::to_string(mat) + " properties are not physically realizable",
                "", "", 20, "QTildas");
        }

        qtil(0, 1) = qbar16 - (qbar12 * qbar26) / qbar22;
        qtil(1, 1) = qbar66 - (qbar26 * qbar26) / qbar22;
        qtil(1, 0) = qtil(0, 1);

        return qtil;
    }

    /// @brief 计算翼型剖面各分段的几何信息和属性
    /// @details 将剖面划分为上表面（surfaceType=1）、下表面（surfaceType=2）和腹板（surfaceType=0）分段，
    ///          计算每个分段的位置、长度、角度等几何参数。
    /// @param pre 预处理数据对象
    /// @param eps 数值容差
    /// @param ch 弦长参考值
    /// @param rle 前缘位置参考值
    /// @param nseg 总分段数量
    /// @param nseg_u 上表面分段数量
    /// @param nseg_p 上下表面分段总数（不含腹板）
    /// @param xnode_u 上表面节点x坐标向量
    /// @param ynode_u 上表面节点y坐标向量
    /// @param xnode_l 下表面节点x坐标向量
    /// @param ynode_l 下表面节点y坐标向量
    /// @param ndl1 下表面起始节点索引（1-based）
    /// @param ndu1 上表面起始节点索引（1-based）
    /// @param loc_web 腹板位置的x坐标数组
    /// @param weby_u 腹板上端点的y坐标数组
    /// @param weby_l 腹板下端点的y坐标数组
    /// @param n_scts 各表面的扇区数量数组（索引0=上表面，1=下表面）
    /// @param sec 当前处理的截面索引
    /// @return SegInfoResult 包含9组分段几何信息的结构体
    inline SegInfoResult SegInfo(
        const PcsLInput& pre, double eps,
        double ch, double rle, int nseg, int nseg_u, int nseg_p,
        const VectorXd& xnode_u, const VectorXd& ynode_u,
        const VectorXd& xnode_l, const VectorXd& ynode_l,
        int ndl1, int ndu1, const std::vector<double>& loc_web,
        const std::vector<double>& weby_u, const std::vector<double>& weby_l,
        const std::vector<int>& n_scts, int sec)
    {
        SegInfoResult result;
        result.isur.resize(nseg);
        result.idsect.resize(nseg);
        result.yseg.resize(nseg);
        result.zseg.resize(nseg);
        result.wseg.resize(nseg);
        result.sthseg.resize(nseg);
        result.cthseg.resize(nseg);
        result.s2thseg.resize(nseg);
        result.c2thseg.resize(nseg);

        for (int iseg = 0; iseg < nseg; iseg++)
        {
            int surfaceType = -1;
            double xa = 0, ya = 0, xb = 0, yb = 0;
            int currentWeb = 0;

            if (iseg < nseg_u)
            {
                int ndIndex = ndu1 - 1 + iseg;
                xa = xnode_u[ndIndex];
                ya = ynode_u[ndIndex];
                xb = xnode_u[ndIndex + 1];
                yb = ynode_u[ndIndex + 1];
                surfaceType = 1;
            }
            else if (iseg < nseg_p)
            {
                int offset = iseg - nseg_u;
                int ndIndex = ndl1 - 1 + offset;
                xa = xnode_l[ndIndex];
                ya = ynode_l[ndIndex];
                xb = xnode_l[ndIndex + 1];
                yb = ynode_l[ndIndex + 1];
                surfaceType = 2;
            }
            else
            {
                currentWeb = iseg - nseg_p;
                xa = loc_web[currentWeb];
                xb = xa;
                ya = weby_u[currentWeb];
                yb = weby_l[currentWeb];
                surfaceType = 0;
            }

            if (surfaceType == -1)
            {
                LogHelper::ErrorLog(
                    "无效的分段索引: " + std::to_string(iseg + 1),
                    "", "", 20, "SegInfo");
            }

            result.isur[iseg] = surfaceType;

            // 确定截面ID
            if (surfaceType > 0)
            {
                bool found = false;
                int sections = n_scts[surfaceType - 1];

                for (int i = 0; i < sections; i++)
                {
                    double lowerBound = 0;
                    double upperBound = 0;
                    if (surfaceType == 1)
                    {
                        lowerBound = pre.Sections[sec].TopConfiguration.xsec_node[i] - eps;
                        upperBound = pre.Sections[sec].TopConfiguration.xsec_node[i + 1] + eps;
                    }
                    if (surfaceType == 2)
                    {
                        lowerBound = pre.Sections[sec].LowConfiguration.xsec_node[i] - eps;
                        upperBound = pre.Sections[sec].LowConfiguration.xsec_node[i + 1] + eps;
                    }
                    if (xa > lowerBound && xb < upperBound)
                    {
                        result.idsect[iseg] = i + 1; // 1-based
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    LogHelper::ErrorLog(
                        "分段 " + std::to_string(iseg + 1) + " 找不到对应截面",
                        "", "", 20, "SegInfo");
                }
            }
            else
            {
                result.idsect[iseg] = currentWeb + 1; // 1-based
            }

            // 几何计算
            double xDiff = xb - xa;
            double yDiff = ya - yb;

            result.yseg[iseg] = ch * (2.0 * rle - xa - xb) / 2.0;
            result.zseg[iseg] = ch * (ya + yb) / 2.0;
            result.wseg[iseg] = ch * std::sqrt(xDiff * xDiff + yDiff * yDiff);

            double theta;
            if (surfaceType == 0)
            {
                theta = -M_PI / 2.0;
            }
            else
            {
                theta = std::atan2(yDiff, xDiff);
            }

            result.sthseg[iseg] = std::sin(theta);
            result.cthseg[iseg] = std::cos(theta);
            result.s2thseg[iseg] = std::sin(2.0 * theta);
            result.c2thseg[iseg] = std::cos(2.0 * theta);
        }

        return result;
    }

    /// @brief 对预处理器数据执行完整性检查和验证
    /// @details 验证翼型几何一致性、前缘后缘位置、非自交性、扇区边界合理性和材料属性物理可实现性。
    /// @param pre 要验证的预处理器数据对象
    /// @param eps 数值容差（默认1E-15）
    inline void CheckPreComl(const PcsLInput& pre, double eps = 1E-15)
    {
        for (int i = 0; i < pre.SectionCount; i++)
        {
            const auto& node = pre.Sections[i];

            if (node.Airfoil.X.size() != node.Airfoil.Y.size())
            {
                LogHelper::ErrorLog(
                    "x and y node lengths do not match at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            if (static_cast<int>(pre.Material.size()) != pre.MaterialTypeCount)
            {
                LogHelper::ErrorLog(
                    "lengths of specified material properties do not match at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            if (node.AerodynamicCenter < 0)
            {
                LogHelper::ErrorLog(
                    "leading edge aft of reference axis->AerodynamicCenter smaller than 0 at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            if (node.Airfoil.N_af_nodes <= 2)
            {
                LogHelper::ErrorLog(
                    "min 3 nodes reqd to define airfoil geom at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            if (MathHelper::FindIndex(node.Airfoil.X, node.Airfoil.X.minCoeff(), false) != 0)
            {
                LogHelper::ErrorLog(
                    "the first airfoil node not a leading node at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            if (std::abs(node.Airfoil.X[0]) > eps || std::abs(node.Airfoil.Y[0]) > eps)
            {
                LogHelper::ErrorLog(
                    "leading-edge node not located at (0,0) at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            if (MathHelper::FindIndex(node.Airfoil.X, node.Airfoil.X.maxCoeff(), false) <= 0)
            {
                LogHelper::ErrorLog(
                    "trailing-edge node not located at (1,0) at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            if (node.Airfoil.X.maxCoeff() > 1)
            {
                LogHelper::ErrorLog(
                    "trailing-edge node exceeds chord boundary at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            // 检查翼型方向
            if (node.Airfoil.Yu[1] / node.Airfoil.Xu[1] <=
                node.Airfoil.Yl[1] / node.Airfoil.Xl[1])
            {
                LogHelper::ErrorLog(
                    "airfoil node numbering not clockwise " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            // 检查翼型非自交
            for (int j = 1; j < static_cast<int>(node.Airfoil.Xl.size()) - 1; j++)
            {
                double x = node.Airfoil.Xl[j];
                for (int k = 0; k < static_cast<int>(node.Airfoil.Xu.size()) - 1; k++)
                {
                    double xl = node.Airfoil.Xu[k];
                    double xr = node.Airfoil.Xu[k + 1];
                    if (x >= xl && x <= xr)
                    {
                        double yl = node.Airfoil.Yu[k];
                        double yr = node.Airfoil.Yu[k + 1];
                        double y = yl + (yr - yl) * (x - xl) / (xr - xl);
                        if (node.Airfoil.Yl[j] >= y)
                        {
                            LogHelper::ErrorLog(
                                "airfoil shape self-crossing at section " + std::to_string(i + 1),
                                "", "", 20, "CheckPreComl");
                        }
                    }
                }
            }

            // 检查上翼面扇区
            if (pre.Sections[i].TopConfiguration.N_scts <= 0)
            {
                LogHelper::ErrorLog(
                    "upper-surf no of sectors not positive at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            if (pre.Sections[i].TopConfiguration.xsec_node[0] < 0)
            {
                LogHelper::ErrorLog(
                    "upper-surf sector node x-location not positive at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            if (!LinearAlgebraHelper::EqualRealNos(
                    pre.Sections[i].TopConfiguration.xsec_node[pre.Sections[i].TopConfiguration.N_scts],
                    pre.Sections[i].Airfoil.Xu[pre.Sections[i].Airfoil.Xu.size() - 1], 1E-3))
            {
                LogHelper::ErrorLog(
                    "upper-surf last sector node out of bounds at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            for (int j = 0; j < pre.Sections[i].TopConfiguration.N_scts; j++)
            {
                if (pre.Sections[i].TopConfiguration.xsec_node[j + 1] <=
                    pre.Sections[i].TopConfiguration.xsec_node[j])
                {
                    LogHelper::ErrorLog(
                        "upper-surf sector nodal x-locations not in ascending order at section " + std::to_string(i + 1),
                        "", "", 20, "CheckPreComl");
                }
            }

            // 检查下翼面扇区
            if (pre.Sections[i].LowConfiguration.N_scts <= 0)
            {
                LogHelper::ErrorLog(
                    "Lower-surf no of sectors not positive at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            if (pre.Sections[i].LowConfiguration.xsec_node[0] < 0)
            {
                LogHelper::ErrorLog(
                    "Lower-surf sector node x-location not positive at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            if (!LinearAlgebraHelper::EqualRealNos(
                    pre.Sections[i].LowConfiguration.xsec_node[pre.Sections[i].LowConfiguration.N_scts],
                    pre.Sections[i].Airfoil.Xl[pre.Sections[i].Airfoil.Xl.size() - 1], 1E-3))
            {
                LogHelper::ErrorLog(
                    "Lower-surf last sector node out of bounds at section " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }

            for (int j = 0; j < pre.Sections[i].LowConfiguration.N_scts; j++)
            {
                if (pre.Sections[i].LowConfiguration.xsec_node[j + 1] <=
                    pre.Sections[i].LowConfiguration.xsec_node[j])
                {
                    LogHelper::ErrorLog(
                        "Lower-surf sector nodal x-locations not in ascending order at section " + std::to_string(i + 1),
                        "", "", 20, "CheckPreComl");
                }
            }
        }

        // 检查材料系数
        for (int i = 0; i < pre.MaterialTypeCount; i++)
        {
            const auto& m = pre.Material[i];
            if (m.Nu12 > std::sqrt(m.E1 / m.E2))
            {
                LogHelper::ErrorLog(
                    "material coefficient Nu12 larger than sqrt(E1/E2) at material " + std::to_string(i + 1),
                    "", "", 20, "CheckPreComl");
            }
        }
    }

    // ===================================================================
    //  向量节点插入辅助函数
    // ===================================================================

    /// @brief 向已排序的VectorXd中插入一个新值（如果不存在），同时在对应位置插入y值
    /// @param xvec 待插入的x坐标向量（引用，会被修改）
    /// @param yvec 待插入的y坐标向量（引用，会被修改）
    /// @param xval 要插入的x值
    /// @param yval 要插入的y值
    /// @return 是否执行了插入操作
    inline bool InsertNodeIfAbsent(VectorXd& xvec, VectorXd& yvec, double xval, double yval)
    {
        // 检查是否已存在
        for (int i = 0; i < xvec.size(); i++)
        {
            if (xvec[i] == xval) return false;
        }

        int insertPos = MathHelper::FindIndex(xvec, xval) + 1;
        int n = static_cast<int>(xvec.size());

        VectorXd newX(n + 1);
        VectorXd newY(n + 1);

        newX.head(insertPos) = xvec.head(insertPos);
        newX[insertPos] = xval;
        newX.tail(n - insertPos) = xvec.tail(n - insertPos);

        newY.head(insertPos) = yvec.head(insertPos);
        newY[insertPos] = yval;
        newY.tail(n - insertPos) = yvec.tail(n - insertPos);

        xvec = newX;
        yvec = newY;
        return true;
    }

} // namespace Qahse::PcsL::CLT
