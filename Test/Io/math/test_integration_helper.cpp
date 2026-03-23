// test_integration_helper.cpp - IntegrationHelper 类的单元测试
#define _USE_MATH_DEFINES
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <Eigen/Dense>
#include "../../../src/IO/Math/IntegrationHelper.h"

using namespace HawtC3::IO::Math;

// ============================================================================
// Trapz - 双精度数组版本
// ============================================================================

TEST(IntegrationHelperTest, Trapz_DoubleArray_Linear) {
	// y = x, x in [0,1] => 积分 = 0.5
	std::vector<double> x = {0.0, 0.25, 0.5, 0.75, 1.0};
	std::vector<double> y = {0.0, 0.25, 0.5, 0.75, 1.0};
	double result = IntegrationHelper::Trapz(x, y);
	EXPECT_NEAR(result, 0.5, 1e-10);
}

TEST(IntegrationHelperTest, Trapz_DoubleArray_Constant) {
	// y = 2, x in [0,3] => 积分 = 6
	std::vector<double> x = {0.0, 1.0, 2.0, 3.0};
	std::vector<double> y = {2.0, 2.0, 2.0, 2.0};
	double result = IntegrationHelper::Trapz(x, y);
	EXPECT_NEAR(result, 6.0, 1e-10);
}

TEST(IntegrationHelperTest, Trapz_DoubleArray_Quadratic) {
	// y = x^2, x in [0,1], 用足够多的点 => 积分 ≈ 1/3
	int n = 1000;
	std::vector<double> x(n), y(n);
	for (int i = 0; i < n; ++i) {
		x[i] = static_cast<double>(i) / (n - 1);
		y[i] = x[i] * x[i];
	}
	double result = IntegrationHelper::Trapz(x, y);
	EXPECT_NEAR(result, 1.0 / 3.0, 1e-4);
}

TEST(IntegrationHelperTest, Trapz_DoubleArray_RefOutput) {
	std::vector<double> x = {0.0, 1.0, 2.0};
	std::vector<double> y = {1.0, 1.0, 1.0};
	double integral = 0.0;
	IntegrationHelper::Trapz(x, y, integral);
	EXPECT_NEAR(integral, 2.0, 1e-10);
}


// ============================================================================
// Trapz - 单精度数组版本
// ============================================================================

TEST(IntegrationHelperTest, Trapz_FloatArray_Linear) {
	std::vector<float> x = {0.0f, 0.5f, 1.0f};
	std::vector<float> y = {0.0f, 0.5f, 1.0f};
	float result = IntegrationHelper::Trapz(x, y);
	EXPECT_NEAR(result, 0.5f, 1e-5f);
}

TEST(IntegrationHelperTest, Trapz_FloatArray_RefOutput) {
	std::vector<float> x = {0.0f, 1.0f, 2.0f};
	std::vector<float> y = {1.0f, 1.0f, 1.0f};
	float integral = 0.0f;
	IntegrationHelper::Trapz(x, y, integral);
	EXPECT_NEAR(integral, 2.0f, 1e-5f);
}


// ============================================================================
// Trapz - 双精度向量版本
// ============================================================================

TEST(IntegrationHelperTest, Trapz_VectorXd_Linear) {
	Eigen::VectorXd x(5);
	x << 0.0, 0.25, 0.5, 0.75, 1.0;
	Eigen::VectorXd y = x;
	double result = IntegrationHelper::Trapz(x, y);
	EXPECT_NEAR(result, 0.5, 1e-10);
}

TEST(IntegrationHelperTest, Trapz_VectorXd_Sine) {
	// sin(x) from 0 to PI => 积分 = 2
	int n = 1000;
	Eigen::VectorXd x(n), y(n);
	for (int i = 0; i < n; ++i) {
		x(i) = static_cast<double>(i) / (n - 1) * M_PI;
		y(i) = std::sin(x(i));
	}
	double result = IntegrationHelper::Trapz(x, y);
	EXPECT_NEAR(result, 2.0, 1e-4);
}


// ============================================================================
// Trapz - 单精度向量版本
// ============================================================================

TEST(IntegrationHelperTest, Trapz_VectorXf_Linear) {
	Eigen::VectorXf x(3);
	x << 0.0f, 0.5f, 1.0f;
	Eigen::VectorXf y = x;
	float result = IntegrationHelper::Trapz(x, y);
	EXPECT_NEAR(result, 0.5f, 1e-5f);
}


// ============================================================================
// Trapz - 矩阵版本（双精度）
// ============================================================================

TEST(IntegrationHelperTest, Trapz_MatrixXd_Dim1) {
	// dim=1: 对每列沿行积分
	Eigen::VectorXd x(3);
	x << 0.0, 1.0, 2.0;
	Eigen::MatrixXd y(3, 2);
	y << 1.0, 2.0,
		 1.0, 2.0,
		 1.0, 2.0;
	Eigen::VectorXd result = IntegrationHelper::Trapz(x, y, 1);
	ASSERT_EQ(result.size(), 2);
	EXPECT_NEAR(result(0), 2.0, 1e-10);  // 常数1在[0,2]上积分
	EXPECT_NEAR(result(1), 4.0, 1e-10);  // 常数2在[0,2]上积分
}


// ============================================================================
// TrapzA (累积积分)
// ============================================================================

TEST(IntegrationHelperTest, TrapzA_VectorXd) {
	Eigen::VectorXd x(4);
	x << 0.0, 1.0, 2.0, 3.0;
	Eigen::VectorXd y(4);
	y << 1.0, 1.0, 1.0, 1.0;
	Eigen::VectorXd result = IntegrationHelper::TrapzA(x, y);
	ASSERT_EQ(result.size(), 4);
}

TEST(IntegrationHelperTest, TrapzA_VectorXf) {
	Eigen::VectorXf x(3);
	x << 0.0f, 1.0f, 2.0f;
	Eigen::VectorXf y(3);
	y << 1.0f, 1.0f, 1.0f;
	Eigen::VectorXf result = IntegrationHelper::TrapzA(x, y);
	ASSERT_EQ(result.size(), 3);
}
