// test_pcsl_clt_subs.cpp - PcsL CLT 辅助函数单元测试
#define _USE_MATH_DEFINES
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <Eigen/Dense>
#include "../../src/PcsL/CLT/Pcs_CLT_Subs.h"

using namespace Qahse::PcsL;
using namespace Qahse::PcsL::CLT;
using Eigen::VectorXd;
using Eigen::Matrix2d;

// ============================================================================
// tw_rate 函数测试
// ============================================================================

/// @brief 均匀间距、线性扭转角：变化率应为常数
TEST(TwRateTest, LinearTwist_UniformSpacing)
{
    VectorXd sloc(5);
    sloc << 0.0, 1.0, 2.0, 3.0, 4.0;
    VectorXd tw(5);
    tw << 0.0, 2.0, 4.0, 6.0, 8.0; // 线性：2度/单位长度

    VectorXd result = tw_rate(sloc, tw);

    ASSERT_EQ(result.size(), 5);
    for (int i = 0; i < 5; i++)
    {
        EXPECT_NEAR(result[i], 2.0, 1e-10);
    }
}

/// @brief 非均匀间距测试
TEST(TwRateTest, NonUniformSpacing)
{
    VectorXd sloc(3);
    sloc << 0.0, 1.0, 4.0;
    VectorXd tw(3);
    tw << 0.0, 10.0, 40.0; // 线性：10度/单位长度

    VectorXd result = tw_rate(sloc, tw);

    ASSERT_EQ(result.size(), 3);
    EXPECT_NEAR(result[0], 10.0, 1e-10); // 前向差分
    EXPECT_NEAR(result[2], 10.0, 1e-10); // 后向差分
    EXPECT_NEAR(result[1], 10.0, 1e-10); // 中心差分（线性应精确）
}

/// @brief 零扭转角时变化率应为零
TEST(TwRateTest, ZeroTwist)
{
    VectorXd sloc(4);
    sloc << 0.0, 5.0, 10.0, 15.0;
    VectorXd tw = VectorXd::Zero(4);

    VectorXd result = tw_rate(sloc, tw);

    for (int i = 0; i < 4; i++)
    {
        EXPECT_NEAR(result[i], 0.0, 1e-15);
    }
}

/// @brief 仅2个点时仅使用前向/后向差分
TEST(TwRateTest, TwoPoints)
{
    VectorXd sloc(2);
    sloc << 0.0, 5.0;
    VectorXd tw(2);
    tw << 10.0, 20.0;

    VectorXd result = tw_rate(sloc, tw);

    ASSERT_EQ(result.size(), 2);
    EXPECT_NEAR(result[0], 2.0, 1e-10);
    EXPECT_NEAR(result[1], 2.0, 1e-10);
}

// ============================================================================
// Q_Bars 函数测试
// ============================================================================

/// @brief 0度角度时：Q̄矩阵等于原始Q矩阵
TEST(QBarsTest, ZeroAngle)
{
    VectorXd density(1), q11(1), q22(1), q12(1), q66(1);
    density << 1600.0;
    q11 << 1.0e11;
    q22 << 1.0e10;
    q12 << 3.0e9;
    q66 << 5.0e9;

    QBarResult r = Q_Bars(0, 0.0, density, q11, q22, q12, q66);

    EXPECT_NEAR(r.qbar11, 1.0e11, 1.0);
    EXPECT_NEAR(r.qbar22, 1.0e10, 1.0);
    EXPECT_NEAR(r.qbar12, 3.0e9, 1.0);
    EXPECT_NEAR(r.qbar16, 0.0, 1.0);
    EXPECT_NEAR(r.qbar26, 0.0, 1.0);
    EXPECT_NEAR(r.qbar66, 5.0e9, 1.0);
    EXPECT_NEAR(r.rho_m, 1600.0, 1e-10);
}

/// @brief 90度角度时：Q̄₁₁和Q̄₂₂互换
TEST(QBarsTest, NinetyDegreeAngle)
{
    VectorXd density(1), q11(1), q22(1), q12(1), q66(1);
    density << 1800.0;
    q11 << 1.0e11;
    q22 << 1.0e10;
    q12 << 3.0e9;
    q66 << 5.0e9;

    QBarResult r = Q_Bars(0, M_PI / 2.0, density, q11, q22, q12, q66);

    EXPECT_NEAR(r.qbar11, 1.0e10, 1.0);
    EXPECT_NEAR(r.qbar22, 1.0e11, 1.0);
    EXPECT_NEAR(r.qbar12, 3.0e9, 1.0);
    EXPECT_NEAR(r.qbar16, 0.0, 1e3);
    EXPECT_NEAR(r.qbar26, 0.0, 1e3);
}

/// @brief 45度角度时：Q̄₁₆和Q̄₂₆不为零
TEST(QBarsTest, FortyFiveDegreeAngle)
{
    VectorXd density(1), q11(1), q22(1), q12(1), q66(1);
    density << 1600.0;
    q11 << 1.40e11;
    q22 << 1.0e10;
    q12 << 3.0e9;
    q66 << 5.0e9;

    QBarResult r = Q_Bars(0, M_PI / 4.0, density, q11, q22, q12, q66);

    // 45度时 Q̄₁₁ = Q̄₂₂（对称性）
    EXPECT_NEAR(r.qbar11, r.qbar22, 1.0);
    // Q̄₁₆ 和 Q̄₂₆ 不为零
    EXPECT_GT(std::abs(r.qbar16), 1e3);
    EXPECT_GT(std::abs(r.qbar26), 1e3);
}

/// @brief 多材料时正确引用指定材料
TEST(QBarsTest, MultipleMaterials)
{
    VectorXd density(3), q11(3), q22(3), q12(3), q66(3);
    density << 1500.0, 1600.0, 1700.0;
    q11 << 1.0e11, 2.0e11, 3.0e11;
    q22 << 1.0e10, 2.0e10, 3.0e10;
    q12 << 3.0e9, 6.0e9, 9.0e9;
    q66 << 5.0e9, 1.0e10, 1.5e10;

    // 选择第2个材料（0-based index=1）
    QBarResult r = Q_Bars(1, 0.0, density, q11, q22, q12, q66);

    EXPECT_NEAR(r.qbar11, 2.0e11, 1.0);
    EXPECT_NEAR(r.rho_m, 1600.0, 1e-10);
}

// ============================================================================
// QTildas 函数测试
// ============================================================================

/// @brief 正常物理参数时 Q̃ 矩阵计算正确
TEST(QTildasTest, NormalParameters)
{
    double qbar11 = 1.0e11;
    double qbar22 = 1.0e10;
    double qbar12 = 3.0e9;
    double qbar16 = 1.0e8;
    double qbar26 = 5.0e7;
    double qbar66 = 5.0e9;

    Matrix2d qtil = QTildas(qbar11, qbar22, qbar12, qbar16, qbar26, qbar66, 1);

    // Q̃₁₁ = Q̄₁₁ - Q̄₁₂²/Q̄₂₂
    double expected_q11 = qbar11 - (qbar12 * qbar12) / qbar22;
    EXPECT_NEAR(qtil(0, 0), expected_q11, 1.0);

    // Q̃₁₂ = Q̄₁₆ - Q̄₁₂·Q̄₂₆/Q̄₂₂
    double expected_q12 = qbar16 - (qbar12 * qbar26) / qbar22;
    EXPECT_NEAR(qtil(0, 1), expected_q12, 1.0);

    // Q̃₂₂ = Q̄₆₆ - Q̄₂₆²/Q̄₂₂
    double expected_q22 = qbar66 - (qbar26 * qbar26) / qbar22;
    EXPECT_NEAR(qtil(1, 1), expected_q22, 1.0);

    // 对称性
    EXPECT_EQ(qtil(0, 1), qtil(1, 0));
}

/// @brief 对角矩阵（无耦合项）
TEST(QTildasTest, DiagonalCase)
{
    Matrix2d qtil = QTildas(1.0e11, 1.0e10, 0.0, 0.0, 0.0, 5.0e9, 1);

    // 无耦合时简化
    EXPECT_NEAR(qtil(0, 0), 1.0e11, 1.0);
    EXPECT_NEAR(qtil(0, 1), 0.0, 1.0);
    EXPECT_NEAR(qtil(1, 1), 5.0e9, 1.0);
}

// ============================================================================
// InsertNodeIfAbsent 函数测试
// ============================================================================

/// @brief 插入新节点到中间位置
TEST(InsertNodeTest, InsertInMiddle)
{
    VectorXd x(4), y(4);
    x << 0.0, 0.3, 0.7, 1.0;
    y << 0.0, 0.1, 0.05, 0.0;

    bool inserted = InsertNodeIfAbsent(x, y, 0.5, 0.08);

    EXPECT_TRUE(inserted);
    ASSERT_EQ(x.size(), 5);
    EXPECT_NEAR(x[2], 0.5, 1e-15);
    EXPECT_NEAR(y[2], 0.08, 1e-15);
}

/// @brief 已存在的节点不重复插入
TEST(InsertNodeTest, NoInsertIfExists)
{
    VectorXd x(4), y(4);
    x << 0.0, 0.3, 0.7, 1.0;
    y << 0.0, 0.1, 0.05, 0.0;

    bool inserted = InsertNodeIfAbsent(x, y, 0.3, 0.12);

    EXPECT_FALSE(inserted);
    ASSERT_EQ(x.size(), 4);
}

/// @brief 连续插入多个节点
TEST(InsertNodeTest, MultipleInsertions)
{
    VectorXd x(3), y(3);
    x << 0.0, 0.5, 1.0;
    y << 0.0, 0.1, 0.0;

    InsertNodeIfAbsent(x, y, 0.25, 0.05);
    InsertNodeIfAbsent(x, y, 0.75, 0.05);

    ASSERT_EQ(x.size(), 5);
    EXPECT_NEAR(x[0], 0.0, 1e-15);
    EXPECT_NEAR(x[1], 0.25, 1e-15);
    EXPECT_NEAR(x[2], 0.5, 1e-15);
    EXPECT_NEAR(x[3], 0.75, 1e-15);
    EXPECT_NEAR(x[4], 1.0, 1e-15);
}

// ============================================================================
// SegInfo 函数测试
// ============================================================================

/// @brief 基本分段信息计算：简单矩形截面仅上下翼面
TEST(SegInfoTest, BasicRectangularSection)
{
    // 构建最简单的PcsLInput
    PcsLInput pre;
    pre.SectionCount = 1;
    pre.MaterialTypeCount = 1;

    BladeSection sec;
    sec.ChordLength = 1.0;
    sec.AerodynamicCenter = 0.25;
    sec.TopConfiguration.N_scts = 1;
    sec.TopConfiguration.xsec_node = {0.0, 1.0};

    SectorLaminae topSector;
    topSector.Sect_num = 1;
    topSector.N_laminas = 1;
    Lamina ply;
    ply.Thickness = 0.01;
    ply.PlyCount = 1;
    ply.FiberOrientation = 0.0;
    ply.MaterialId = 1;
    topSector.Laminae.push_back(ply);
    sec.TopConfiguration.Sector.push_back(topSector);

    sec.LowConfiguration.N_scts = 1;
    sec.LowConfiguration.xsec_node = {0.0, 1.0};
    sec.LowConfiguration.Sector.push_back(topSector);

    // 上翼面节点：3个点
    sec.Airfoil.Xu.resize(3);
    sec.Airfoil.Xu << 0.0, 0.5, 1.0;
    sec.Airfoil.Yu.resize(3);
    sec.Airfoil.Yu << 0.0, 0.05, 0.0;

    // 下翼面节点：3个点
    sec.Airfoil.Xl.resize(3);
    sec.Airfoil.Xl << 0.0, 0.5, 1.0;
    sec.Airfoil.Yl.resize(3);
    sec.Airfoil.Yl << 0.0, -0.05, 0.0;

    pre.Sections.push_back(sec);

    int nseg_u = 2; // Xu.size()-1
    int nseg_l = 2; // Xl.size()-1
    int nseg_p = nseg_u + nseg_l;
    int nseg = nseg_p;

    std::vector<int> n_scts = {1, 1};
    std::vector<double> locw, weby_u_v, weby_l_v;

    SegInfoResult result = SegInfo(pre, 1e-5,
        1.0, 0.25, nseg, nseg_u, nseg_p,
        sec.Airfoil.Xu, sec.Airfoil.Yu,
        sec.Airfoil.Xl, sec.Airfoil.Yl,
        1, 1, locw, weby_u_v, weby_l_v, n_scts, 0);

    // 验证分段数量
    ASSERT_EQ(static_cast<int>(result.isur.size()), nseg);

    // 验证上翼面分段类型
    for (int k = 0; k < nseg_u; k++)
    {
        EXPECT_EQ(result.isur[k], 1);
    }
    // 验证下翼面分段类型
    for (int k = nseg_u; k < nseg_p; k++)
    {
        EXPECT_EQ(result.isur[k], 2);
    }

    // 验证分段长度非零
    for (int k = 0; k < nseg; k++)
    {
        EXPECT_GT(result.wseg[k], 0.0);
    }
}
