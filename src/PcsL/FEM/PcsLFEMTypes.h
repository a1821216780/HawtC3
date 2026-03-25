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

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>
#include <string>
#include <cmath>

/// @brief 基于有限元方法的截面特性计算方法命名空间
namespace HawtC3::PcsL::FEM
{

    /// @brief 2D有限元分析的支持单元类型
    enum ElemType
    {
        Q4 = 1,  ///< 四节点四边形单元（常用的线性单元）
        Q8 = 2,  ///< 八节点二次四边形单元（更高阶精度）
        Q8R = 3, ///< 八节点二次四边形单元（简化积分，减少剪切锁定）
        T6 = 4   ///< 六节点二次三角形单元
    };

    // ---------- per-element-type constants --------------------------------------

    /// @brief 每种单元类型的常数信息，包括节点数、积分点数、积分点位置和权重等
    struct ElemInfo
    {
        ElemType etype;                   ///< 单元类型（Q4/Q8/Q8R/T6）
        int nnpe;                         ///< 每个单元的节点数（如Q4为4，Q8为8）
        int mdim;                         ///< 每个单元的自由度数（通常为3*nnpe，3自由度/节点）
        int gpoints;                      ///< 每个单元的高斯积分点数
        std::vector<double> xxg, yyg, wg; ///< 积分点在参考单元上的x/y坐标和权重（长度为gpoints）
    };

    /// @brief 创建Q4单元的常数信息
    /// @return Q4单元的积分和其他常数信息
    /// @details Q4单元采用2x2高斯积分，积分点坐标为±0.577...，权重均为1
    inline ElemInfo MakeQ4Info()
    {
        ElemInfo e;
        e.etype = Q4;
        e.nnpe = 4;
        e.mdim = 12;
        e.gpoints = 4;
        double g = 0.577350269189626; // 2点高斯积分点
        e.xxg = {-g, -g, g, g};       // x方向积分点
        e.yyg = {-g, g, -g, g};       // y方向积分点
        e.wg = {1, 1, 1, 1};          // 权重
        return e;
    }
    /// @brief 创建Q8单元的常数信息
    /// @return Q8单元的积分和其他常数信息
    /// @details Q8单元采用3x3高斯积分，积分点坐标为±0.774...和0，权重不等
    inline ElemInfo MakeQ8Info()
    {
        ElemInfo e;
        e.etype = Q8;
        e.nnpe = 8;
        e.mdim = 24;
        e.gpoints = 9;
        double g = 0.774596669241483;           // 3点高斯积分点
        e.xxg = {-g, -g, -g, 0, 0, 0, g, g, g}; // x方向积分点
        e.yyg = {-g, 0, g, -g, 0, g, -g, 0, g}; // y方向积分点
        e.wg = {0.308641975308642, 0.493827160493827, 0.308641975308642,
                0.493827160493827, 0.790123456790123, 0.493827160493827,
                0.308641975308642, 0.493827160493827, 0.308641975308642}; // 权重
        return e;
    }

    /// @brief 创建Q8R单元的常数信息
    /// @return Q8R单元的积分和其他常数信息
    /// @details Q8R单元采用2x2高斯积分，积分点与Q4相同，适用于简化积分
    inline ElemInfo MakeQ8RInfo()
    {
        ElemInfo e;
        e.etype = Q8R;
        e.nnpe = 8;
        e.mdim = 24;
        e.gpoints = 4;
        double g = 0.577350269189626;
        e.xxg = {-g, -g, g, g};
        e.yyg = {-g, g, -g, g};
        e.wg = {1, 1, 1, 1};
        return e;
    }

    /// @brief 创建T6单元的常数信息
    /// @return T6单元的积分和其他常数信息
    /// @details T6单元采用7点高斯积分，积分点和权重适用于二次三角形单元
    inline ElemInfo MakeT6Info()
    {
        ElemInfo e;
        e.etype = T6;
        e.nnpe = 6;
        e.mdim = 18;
        e.gpoints = 7;
        double a1 = 0.1012865073235;
        double a2 = 0.7974269853531;
        double b1 = 0.4701420641051;
        double b2 = 0.0597158717898;
        double w0 = 0.225;           // 重心点权重
        double w1 = 0.1259391805448; // a1/a2点权重
        double w2 = 0.1323941527885; // b1/b2点权重
        // 三角形的7点规则（映射区域坐标）
        e.xxg = {1.0 / 3.0, a1, a1, a2, b1, b1, b2};
        e.yyg = {1.0 / 3.0, a1, a2, a1, b1, b2, b1};
        e.wg = {w0, w1, w1, w1, w2, w2, w2};
        return e;
    }

    //******************************************************************************
    //
    //   全局范围的数据结构定义，包括输入数据、结果结构和一个结果包结构
    //
    //******************************************************************************

    /// @brief 全局范围的数据结构定义，包括输入数据、结果结构和一个结果包结构
    struct PcsLUtils
    {
        // ================= 输入数据 =================
        Eigen::MatrixXd nl_2d;    ///< (节点数量, 3)：[全局节点id, x, y]，节点坐标表
        Eigen::MatrixXi el_2d;    ///< (单元数量, 1+单元节点数): [内部id, n1, n2, ...]，单元连通性表
        Eigen::MatrixXi emat;     ///< (单元数量, 4): [元素标签, 材料id, 纤维角度, 纤维平面角度]，单元材料属性
        Eigen::MatrixXd matprops; ///< (材料数量, 10): E1 E2 E3 G12 G13 G23 nu12 nu13 nu23 rho，材料属性表

        int ne_2d = 0;    ///< 单元总数
        int nn_2d = 0;    ///< 节点总数
        int nmat = 0;     ///< 材料总数
        int max_nnpe = 0; ///< 所有单元类型中最大节点数

        // ============= 单元类型与常数信息 =============
        std::vector<ElemType> etype;    ///< 每个单元的类型（Q4/Q8/Q8R/T6）
        std::vector<ElemInfo> elemInfo; ///< 单元类型常数信息表（Q4=0, Q8=1, Q8R=2, T6=3）

        // ============= 派生数据 =============
        Eigen::MatrixXd pr_2d;                      ///< (2*max_nnpe, ne): 每个单元的节点坐标
        std::vector<Eigen::Matrix<double, 6, 6>> Q; ///< 每个单元的旋转本构矩阵
        std::vector<double> density;                ///< 每个单元的密度
        Eigen::MatrixXd Zg;                         ///< (3*nn, 6) 约束矩阵
    };

    // Result structures
    /// @brief 截面翘曲解结构体
    struct PcsLSolutions
    {
        Eigen::MatrixXd X, Y;   ///< 翘曲位移解（每个节点的X/Y方向翘曲）
        Eigen::MatrixXd dX, dY; ///< 翘曲位移对坐标的导数（每个节点的X/Y方向导数）
    };

    /// @brief 截面刚度与质量矩阵结构体
    struct PcsLConstitutive
    {
        Eigen::Matrix<double, 6, 6> Ks; ///< 6x6 截面刚度矩阵
        Eigen::Matrix<double, 6, 6> Ms; ///< 6x6 截面质量矩阵
    };

    /// @brief 截面属性结构体
    struct PcsLCrossSectionProps
    {
        double ShearX = 0, ShearY = 0;               ///< 剪切中心坐标
        double ElasticX = 0, ElasticY = 0;           ///< 弹性中心坐标
        double MassTotal = 0, MassX = 0, MassY = 0;  ///< 总质量及质心坐标
        double Ixx = 0, Iyy = 0, Ixy = 0;            ///< 惯性矩
        double AreaTotal = 0, AreaX = 0, AreaY = 0;  ///< 总面积及分区面积
        double Axx = 0, Ayy = 0, Axy = 0;            ///< 剪切面积
        double AlphaPrincipleAxis_Ref = 0;           ///< 主轴与参考轴夹角
        double AlphaPrincipleAxis_ElasticCenter = 0; ///< 主轴与弹性中心轴夹角
    };

    // Result bag
    /// @brief 截面分析结果包结构体
    struct PcsLResult
    {
        PcsLConstitutive constitutive; ///< 截面刚度与质量矩阵
        PcsLSolutions solutions;       ///< 翘曲解
        PcsLCrossSectionProps props;   ///< 截面属性
    };

} // namespace PCSL
