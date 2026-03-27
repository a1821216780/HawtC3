// test_pcsl_clt_run.cpp - PcsL CLT RunCLT 主函数单元测试
#define _USE_MATH_DEFINES
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <Eigen/Dense>
#include "../../src/PcsL/CLT/PcsLCLTRun.h"

using namespace Qahse::PcsL;
using namespace Qahse::PcsL::CLT;
using Eigen::VectorXd;

// ============================================================================
// 辅助函数：构建简化的叶片截面输入
// ============================================================================

/// @brief 创建一个简化的均匀铝合金（单材料、矩形截面、单铺层）叶片输入
/// @return 填充好的 PcsLInput 对象
static PcsLInput CreateSimpleBladeInput()
{
    PcsLInput pre;
    pre.path = "test_clt";
    pre.SectionCount = 3;
    pre.MaterialTypeCount = 1;
    pre.CalStress = false;
    pre.FEMSolve = false;
    pre.Nweb = 0;
    pre.Ib_sp_stn = 0;
    pre.Ob_sp_stn = 0;

    // 单一材料：仿铝
    CompositeMaterial mat;
    mat.Id = 1;
    mat.E1 = 7.0e10;
    mat.E2 = 7.0e10;
    mat.G12 = 2.6e10;
    mat.Nu12 = 0.33;
    mat.Density = 2700.0;
    pre.Material.push_back(mat);

    // 单层铺层定义
    Lamina ply;
    ply.LayerId = 1;
    ply.PlyCount = 1;
    ply.Thickness = 0.005; // 5mm
    ply.FiberOrientation = 0.0;
    ply.MaterialId = 1;

    SectorLaminae sector;
    sector.Sect_num = 1;
    sector.N_laminas = 1;
    sector.Laminae.push_back(ply);

    // 3个截面
    for (int i = 0; i < 3; i++)
    {
        BladeSection sec;
        sec.SpanLocation = static_cast<double>(i) * 10.0;
        sec.ChordLength = 1.0;
        sec.TwistAngle = 0.0;
        sec.AerodynamicCenter = 0.25;

        // 简化翼型：对称NACA型（5个点）
        sec.Airfoil.N_af_nodes = 9;
        sec.Airfoil.X.resize(9);
        sec.Airfoil.Y.resize(9);
        sec.Airfoil.X << 0.0, 0.25, 0.5, 0.75, 1.0, 0.75, 0.5, 0.25, 0.0;
        sec.Airfoil.Y << 0.0, 0.05, 0.06, 0.04, 0.0, -0.04, -0.06, -0.05, 0.0;

        // 上翼面
        sec.Airfoil.Xu.resize(5);
        sec.Airfoil.Yu.resize(5);
        sec.Airfoil.Xu << 0.0, 0.25, 0.5, 0.75, 1.0;
        sec.Airfoil.Yu << 0.0, 0.05, 0.06, 0.04, 0.0;

        // 下翼面
        sec.Airfoil.Xl.resize(5);
        sec.Airfoil.Yl.resize(5);
        sec.Airfoil.Xl << 0.0, 0.25, 0.5, 0.75, 1.0;
        sec.Airfoil.Yl << 0.0, -0.05, -0.06, -0.04, 0.0;

        // 上翼面铺层配置：1个扇区
        sec.TopConfiguration.N_scts = 1;
        sec.TopConfiguration.xsec_node = {0.0, 1.0};
        sec.TopConfiguration.Sector.push_back(sector);

        // 下翼面铺层配置：1个扇区
        sec.LowConfiguration.N_scts = 1;
        sec.LowConfiguration.xsec_node = {0.0, 1.0};
        sec.LowConfiguration.Sector.push_back(sector);

        // 无腹板
        sec.WebConfiguration.N_scts = 0;

        pre.Sections.push_back(sec);
    }

    return pre;
}

// ============================================================================
// RunCLT 集成测试
// ============================================================================

/// @brief 基本运行测试：确保 RunCLT 不崩溃并返回正确数量的结果
TEST(RunCLTTest, BasicExecution)
{
    PcsLInput pre = CreateSimpleBladeInput();
    std::vector<CLTOutput> results = RunCLT(pre);

    ASSERT_EQ(static_cast<int>(results.size()), 3);
}

/// @brief 验证输出刚度和质量为正值
TEST(RunCLTTest, PositiveStiffnessAndMass)
{
    PcsLInput pre = CreateSimpleBladeInput();
    std::vector<CLTOutput> results = RunCLT(pre);

    for (int i = 0; i < 3; i++)
    {
        EXPECT_GT(results[i].EiFlap, 0.0) << "EiFlap should be positive at section " << i;
        EXPECT_GT(results[i].EiLag, 0.0) << "EiLag should be positive at section " << i;
        EXPECT_GT(results[i].GJ, 0.0) << "GJ should be positive at section " << i;
        EXPECT_GT(results[i].EA, 0.0) << "EA should be positive at section " << i;
        EXPECT_GT(results[i].Mass, 0.0) << "Mass should be positive at section " << i;
    }
}

/// @brief 对称翼型时剪切中心和质心应在Y=0附近
TEST(RunCLTTest, SymmetricAirfoilCenters)
{
    PcsLInput pre = CreateSimpleBladeInput();
    std::vector<CLTOutput> results = RunCLT(pre);

    for (int i = 0; i < 3; i++)
    {
        // 对称翼型的Y方向中心应接近0
        EXPECT_NEAR(results[i].XSc, 0.0, 0.1) << "XSc at section " << i;
        EXPECT_NEAR(results[i].XCm, 0.0, 0.1) << "XCm at section " << i;
    }
}

/// @brief 相同截面应产生相同结果
TEST(RunCLTTest, IdenticalSectionsGiveSameResults)
{
    PcsLInput pre = CreateSimpleBladeInput();
    std::vector<CLTOutput> results = RunCLT(pre);

    // 所有截面几何相同，结果应基本一致
    for (int i = 1; i < 3; i++)
    {
        EXPECT_NEAR(results[i].EiFlap, results[0].EiFlap, results[0].EiFlap * 1e-10)
            << "EiFlap mismatch at section " << i;
        EXPECT_NEAR(results[i].EiLag, results[0].EiLag, results[0].EiLag * 1e-10)
            << "EiLag mismatch at section " << i;
        EXPECT_NEAR(results[i].GJ, results[0].GJ, results[0].GJ * 1e-10)
            << "GJ mismatch at section " << i;
        EXPECT_NEAR(results[i].EA, results[0].EA, results[0].EA * 1e-10)
            << "EA mismatch at section " << i;
        EXPECT_NEAR(results[i].Mass, results[0].Mass, results[0].Mass * 1e-10)
            << "Mass mismatch at section " << i;
    }
}

/// @brief 带腹板的截面测试
TEST(RunCLTTest, WithWebs)
{
    PcsLInput pre = CreateSimpleBladeInput();

    // 添加一个腹板
    pre.Nweb = 1;
    pre.Ib_sp_stn = 1; // 1-based
    pre.Ob_sp_stn = 3; // 1-based

    WebStructure web;
    web.WebId = 1;
    web.InnerChordPosition = 0.3;
    web.OuterChordPosition = 0.3;
    pre.Webs.push_back(web);

    // 为所有截面添加腹板铺层配置
    Lamina ply;
    ply.LayerId = 1;
    ply.PlyCount = 1;
    ply.Thickness = 0.005;
    ply.FiberOrientation = 0.0;
    ply.MaterialId = 1;

    SectorLaminae webSector;
    webSector.Sect_num = 1;
    webSector.N_laminas = 1;
    webSector.Laminae.push_back(ply);

    for (int i = 0; i < 3; i++)
    {
        pre.Sections[i].WebConfiguration.N_scts = 1;
        pre.Sections[i].WebConfiguration.xsec_node = {0.3};
        pre.Sections[i].WebConfiguration.Sector.push_back(webSector);
    }

    std::vector<CLTOutput> results = RunCLT(pre);

    ASSERT_EQ(static_cast<int>(results.size()), 3);
    for (int i = 0; i < 3; i++)
    {
        EXPECT_GT(results[i].EiFlap, 0.0);
        EXPECT_GT(results[i].Mass, 0.0);
    }
}

/// @brief 惯性矩应为正值
TEST(RunCLTTest, PositiveInertia)
{
    PcsLInput pre = CreateSimpleBladeInput();
    std::vector<CLTOutput> results = RunCLT(pre);

    for (int i = 0; i < 3; i++)
    {
        EXPECT_GE(results[i].FlapIner, 0.0) << "FlapIner should be non-negative at section " << i;
        EXPECT_GE(results[i].LagIner, 0.0) << "LagIner should be non-negative at section " << i;
    }
}

/// @brief CLTOutput 默认初始化测试
TEST(CLTOutputTest, DefaultValues)
{
    CLTOutput out;
    EXPECT_EQ(out.EiFlap, 0.0);
    EXPECT_EQ(out.EiLag, 0.0);
    EXPECT_EQ(out.GJ, 0.0);
    EXPECT_EQ(out.EA, 0.0);
    EXPECT_EQ(out.Mass, 0.0);
}
