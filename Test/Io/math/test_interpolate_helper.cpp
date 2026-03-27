// test_interpolate_helper.cpp - InterpolateHelper 类的单元测试
#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <Eigen/Dense>
#include "../../../src/IO/Math/InterpolateHelper.h"

using namespace Qahse::IO::Math;

// ============================================================================
// 1D 线性插值 - Interp1DL (vector)
// ============================================================================

TEST(InterpolateHelperTest, Interp1DL_Vector_KnownPoints) {
	std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
	std::vector<double> y = {0.0, 2.0, 4.0, 6.0, 8.0};  // y = 2x

	auto interp = InterpolateHelper::Interp1DL(x, y);

	// 已知节点
	EXPECT_NEAR((*interp)(0.0), 0.0, 1e-10);
	EXPECT_NEAR((*interp)(2.0), 4.0, 1e-10);
	// 中间点
	EXPECT_NEAR((*interp)(1.5), 3.0, 1e-10);
	EXPECT_NEAR((*interp)(3.5), 7.0, 1e-10);
}

// ============================================================================
// 1D 线性插值 - Interp1DL (Eigen)
// ============================================================================

TEST(InterpolateHelperTest, Interp1DL_Eigen_KnownPoints) {
	Eigen::VectorXd x(5), y(5);
	x << 0.0, 1.0, 2.0, 3.0, 4.0;
	y << 0.0, 2.0, 4.0, 6.0, 8.0;

	auto interp = InterpolateHelper::Interp1DL(x, y);

	EXPECT_NEAR((*interp)(1.0), 2.0, 1e-10);
	EXPECT_NEAR((*interp)(2.5), 5.0, 1e-10);
}

// ============================================================================
// 1D 三次样条插值 - Interp1DS (vector)
// ============================================================================

TEST(InterpolateHelperTest, Interp1DS_Vector_LinearData) {
	// 对线性数据，三次样条应精确还原
	std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
	std::vector<double> y = {0.0, 1.0, 2.0, 3.0, 4.0};

	auto interp = InterpolateHelper::Interp1DS(x, y);

	EXPECT_NEAR((*interp)(0.5), 0.5, 1e-6);
	EXPECT_NEAR((*interp)(2.5), 2.5, 1e-6);
}

TEST(InterpolateHelperTest, Interp1DS_Vector_QuadraticData) {
	// y = x^2，样条应在节点间保持较好近似
	std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
	std::vector<double> y = {0.0, 1.0, 4.0, 9.0, 16.0};

	auto interp = InterpolateHelper::Interp1DS(x, y);

	// 三次样条应很好地逼近 x^2
	EXPECT_NEAR((*interp)(1.5), 2.25, 0.1);
	EXPECT_NEAR((*interp)(2.5), 6.25, 0.1);
}

// ============================================================================
// 1D 直接插值 - Interp1D (单值, vector)
// ============================================================================

TEST(InterpolateHelperTest, Interp1D_SingleValue_Linear) {
	std::vector<double> x = {0.0, 1.0, 2.0, 3.0};
	std::vector<double> y = {0.0, 3.0, 6.0, 9.0};  // y = 3x

	double result = InterpolateHelper::Interp1D(x, y, 1.5,
		InterpolateHelper::Interp1DType::Linear);
	EXPECT_NEAR(result, 4.5, 1e-10);
}

TEST(InterpolateHelperTest, Interp1D_SingleValue_CubicSpline) {
	std::vector<double> x = {0.0, 1.0, 2.0, 3.0};
	std::vector<double> y = {0.0, 3.0, 6.0, 9.0};

	double result = InterpolateHelper::Interp1D(x, y, 1.5,
		InterpolateHelper::Interp1DType::CubicSpline);
	EXPECT_NEAR(result, 4.5, 1e-6);
}

// ============================================================================
// 1D 直接插值 - Interp1D (单值, Eigen)
// ============================================================================

TEST(InterpolateHelperTest, Interp1D_SingleValue_Eigen) {
	Eigen::VectorXd x(4), y(4);
	x << 0.0, 1.0, 2.0, 3.0;
	y << 0.0, 3.0, 6.0, 9.0;

	double result = InterpolateHelper::Interp1D(x, y, 1.5,
		InterpolateHelper::Interp1DType::Linear);
	EXPECT_NEAR(result, 4.5, 1e-10);
}

// ============================================================================
// 1D 批量插值 - Interp1D (vector of values)
// ============================================================================

TEST(InterpolateHelperTest, Interp1D_BatchValues_Linear) {
	std::vector<double> x = {0.0, 1.0, 2.0, 3.0};
	std::vector<double> y = {0.0, 2.0, 4.0, 6.0};
	std::vector<double> query = {0.0, 0.5, 1.0, 1.5};

	auto result = InterpolateHelper::Interp1D(x, y, query,
		InterpolateHelper::Interp1DType::Linear);

	ASSERT_EQ(result.size(), x.size());
	EXPECT_NEAR(result[0], 0.0, 1e-10);
	EXPECT_NEAR(result[1], 1.0, 1e-10);
	EXPECT_NEAR(result[2], 2.0, 1e-10);
	EXPECT_NEAR(result[3], 3.0, 1e-10);
}

// ============================================================================
// 2D 规则网格 - Interp2DRL (双线性, vector)
// ============================================================================

TEST(InterpolateHelperTest, Interp2DRL_Vector_Bilinear) {
	// z = x + y，在规则网格 (0,0), (0,1), (1,0), (1,1) 上
	std::vector<double> x = {0.0, 0.0, 1.0, 1.0};
	std::vector<double> y = {0.0, 1.0, 0.0, 1.0};
	std::vector<double> z = {0.0, 1.0, 1.0, 2.0};

	auto interp = InterpolateHelper::Interp2DRL(x, y, z);

	EXPECT_NEAR((*interp)(0.0, 0.0), 0.0, 1e-6);
	EXPECT_NEAR((*interp)(1.0, 1.0), 2.0, 1e-6);
	EXPECT_NEAR((*interp)(0.5, 0.5), 1.0, 1e-6);
}

// ============================================================================
// 2D 规则网格 - Interp2DRS (双三次, vector)
// ============================================================================

TEST(InterpolateHelperTest, Interp2DRS_Vector_Bicubic) {
	// 3x3 网格上 z = x + y
	std::vector<double> x, y, z;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			x.push_back(static_cast<double>(i));
			y.push_back(static_cast<double>(j));
			z.push_back(static_cast<double>(i + j));
		}
	}

	auto interp = InterpolateHelper::Interp2DRS(x, y, z);

	EXPECT_NEAR((*interp)(1.0, 1.0), 2.0, 1e-4);
	EXPECT_NEAR((*interp)(0.5, 0.5), 1.0, 0.2);
}

// ============================================================================
// 2D 规则网格 - Interp2DRN (最近邻, vector)
// ============================================================================

TEST(InterpolateHelperTest, Interp2DRN_Vector_NearestNeighbor) {
	std::vector<double> x = {0.0, 0.0, 1.0, 1.0};
	std::vector<double> y = {0.0, 1.0, 0.0, 1.0};
	std::vector<double> z = {10.0, 20.0, 30.0, 40.0};

	auto interp = InterpolateHelper::Interp2DRN(x, y, z);

	// 在节点处应精确
	EXPECT_NEAR((*interp)(0.0, 0.0), 10.0, 1e-6);
	EXPECT_NEAR((*interp)(1.0, 1.0), 40.0, 1e-6);
}

// ============================================================================
// 2D 直接插值 - Interp2DR (单值, vector)
// ============================================================================

TEST(InterpolateHelperTest, Interp2DR_SingleValue_Bilinear) {
	std::vector<double> x = {0.0, 0.0, 1.0, 1.0};
	std::vector<double> y = {0.0, 1.0, 0.0, 1.0};
	std::vector<double> z = {0.0, 1.0, 1.0, 2.0};

	double result = InterpolateHelper::Interp2DR(x, y, z, 0.5, 0.5,
		InterpolateHelper::Interp2DRegularType::Bilinear);
	EXPECT_NEAR(result, 1.0, 1e-6);
}

TEST(InterpolateHelperTest, Interp2DR_SingleValue_NearestNeighbor) {
	std::vector<double> x = {0.0, 0.0, 1.0, 1.0};
	std::vector<double> y = {0.0, 1.0, 0.0, 1.0};
	std::vector<double> z = {0.0, 1.0, 1.0, 2.0};

	double result = InterpolateHelper::Interp2DR(x, y, z, 0.0, 0.0,
		InterpolateHelper::Interp2DRegularType::NearestNeighbor);
	EXPECT_NEAR(result, 0.0, 1e-6);
}

// ============================================================================
// 2D 规则网格插值 - Eigen 变体
// ============================================================================

TEST(InterpolateHelperTest, Interp2DR_Eigen_Bilinear) {
	Eigen::VectorXd x(4), y(4), z(4);
	x << 0.0, 0.0, 1.0, 1.0;
	y << 0.0, 1.0, 0.0, 1.0;
	z << 0.0, 1.0, 1.0, 2.0;

	double result = InterpolateHelper::Interp2DR(x, y, z, 0.5, 0.5,
		InterpolateHelper::Interp2DRegularType::Bilinear);
	EXPECT_NEAR(result, 1.0, 1e-6);
}

// ============================================================================
// 2D 不规则网格 - LinearDelaunayTriangles (vector)
// ============================================================================

TEST(InterpolateHelperTest, Interp2DIL_Vector_Delaunay) {
	// 三角形顶点: (0,0)=0, (1,0)=1, (0,1)=1
	std::vector<double> x = {0.0, 1.0, 0.0};
	std::vector<double> y = {0.0, 0.0, 1.0};
	std::vector<double> z = {0.0, 1.0, 1.0};

	auto interp = InterpolateHelper::Interp2DIL(x, y, z);

	// 在节点处应精确
	EXPECT_NEAR((*interp)(0.0, 0.0), 0.0, 1e-6);
	EXPECT_NEAR((*interp)(1.0, 0.0), 1.0, 1e-6);
}

// ============================================================================
// 2D 不规则网格 - ThinPlateSpline (vector)
// ============================================================================

TEST(InterpolateHelperTest, Interp2DIT_Vector_ThinPlateSpline) {
	// 5 个散点
	std::vector<double> x = {0.0, 1.0, 0.0, 1.0, 0.5};
	std::vector<double> y = {0.0, 0.0, 1.0, 1.0, 0.5};
	std::vector<double> z = {0.0, 1.0, 1.0, 2.0, 1.0};  // z ≈ x + y

	auto interp = InterpolateHelper::Interp2DIT(x, y, z);

	// 在节点处应精确或接近
	EXPECT_NEAR((*interp)(0.0, 0.0), 0.0, 1e-4);
	EXPECT_NEAR((*interp)(1.0, 1.0), 2.0, 1e-4);
	EXPECT_NEAR((*interp)(0.5, 0.5), 1.0, 1e-4);
}

// ============================================================================
// 2D 不规则网格直接插值 - Interp2DI (vector)
// ============================================================================

TEST(InterpolateHelperTest, Interp2DI_ThinPlateSpline) {
	std::vector<double> x = {0.0, 1.0, 0.0, 1.0, 0.5};
	std::vector<double> y = {0.0, 0.0, 1.0, 1.0, 0.5};
	std::vector<double> z = {0.0, 1.0, 1.0, 2.0, 1.0};

	double result = InterpolateHelper::Interp2DI(x, y, z, 0.5, 0.5,
		InterpolateHelper::Interp2DIrregularType::ThinPlateSpline);
	EXPECT_NEAR(result, 1.0, 1e-4);
}

TEST(InterpolateHelperTest, Interp2DI_Delaunay) {
	std::vector<double> x = {0.0, 1.0, 0.0, 1.0};
	std::vector<double> y = {0.0, 0.0, 1.0, 1.0};
	std::vector<double> z = {0.0, 1.0, 1.0, 2.0};

	double result = InterpolateHelper::Interp2DI(x, y, z, 0.5, 0.5,
		InterpolateHelper::Interp2DIrregularType::LinearDelaunayTriangles);
	EXPECT_NEAR(result, 1.0, 1e-4);
}

// ============================================================================
// 2D 不规则网格直接插值 - Interp2DI (Eigen)
// ============================================================================

TEST(InterpolateHelperTest, Interp2DI_Eigen_ThinPlateSpline) {
	Eigen::VectorXd x(5), y(5), z(5);
	x << 0.0, 1.0, 0.0, 1.0, 0.5;
	y << 0.0, 0.0, 1.0, 1.0, 0.5;
	z << 0.0, 1.0, 1.0, 2.0, 1.0;

	double result = InterpolateHelper::Interp2DI(x, y, z, 0.5, 0.5,
		InterpolateHelper::Interp2DIrregularType::ThinPlateSpline);
	EXPECT_NEAR(result, 1.0, 1e-4);
}
