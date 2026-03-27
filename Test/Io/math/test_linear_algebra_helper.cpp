// test_linear_algebra_helper.cpp - LinearAlgebraHelper 类的单元测试
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <Eigen/Dense>
#include "../../../src/IO/Math/LinearAlgebraHelper.h"

using namespace Qahse::IO::Math;
using LA = LinearAlgebraHelper;

// ============================================================================
// Cumsum
// ============================================================================

TEST(LinearAlgebraHelperTest, Cumsum_Vector) {
	Eigen::VectorXd v(4);
	v << 1.0, 2.0, 3.0, 4.0;
	Eigen::VectorXd result = LA::Cumsum(v);
	ASSERT_EQ(result.size(), 4);
	EXPECT_DOUBLE_EQ(result(0), 1.0);
	EXPECT_DOUBLE_EQ(result(1), 3.0);
	EXPECT_DOUBLE_EQ(result(2), 6.0);
	EXPECT_DOUBLE_EQ(result(3), 10.0);
}

TEST(LinearAlgebraHelperTest, Cumsum_StdVector) {
	std::vector<double> v = {1.0, 2.0, 3.0, 4.0};
	auto result = LA::Cumsum(v);
	ASSERT_EQ(result.size(), 4);
	EXPECT_DOUBLE_EQ(result[0], 1.0);
	EXPECT_DOUBLE_EQ(result[1], 3.0);
	EXPECT_DOUBLE_EQ(result[2], 6.0);
	EXPECT_DOUBLE_EQ(result[3], 10.0);
}

TEST(LinearAlgebraHelperTest, Cumsum_Single) {
	Eigen::VectorXd v(1);
	v << 5.0;
	Eigen::VectorXd result = LA::Cumsum(v);
	ASSERT_EQ(result.size(), 1);
	EXPECT_DOUBLE_EQ(result(0), 5.0);
}


// ============================================================================
// ReCumsum (差分)
// ============================================================================

TEST(LinearAlgebraHelperTest, ReCumsum_Vector) {
	Eigen::VectorXd v(4);
	v << 1.0, 3.0, 6.0, 10.0;
	Eigen::VectorXd result = LA::ReCumsum(v);
	ASSERT_EQ(result.size(), 4);
	EXPECT_DOUBLE_EQ(result(0), 1.0);
	EXPECT_NEAR(result(1), 2.0, 1e-10);
	EXPECT_NEAR(result(2), 3.0, 1e-10);
	EXPECT_NEAR(result(3), 4.0, 1e-10);
}

TEST(LinearAlgebraHelperTest, ReCumsum_StdVector) {
	std::vector<double> v = {1.0, 3.0, 6.0, 10.0};
	auto result = LA::ReCumsum(v);
	ASSERT_EQ(result.size(), 4);
	EXPECT_DOUBLE_EQ(result[0], 1.0);
	EXPECT_NEAR(result[1], 2.0, 1e-10);
	EXPECT_NEAR(result[2], 3.0, 1e-10);
	EXPECT_NEAR(result[3], 4.0, 1e-10);
}


// ============================================================================
// EqualRealNos
// ============================================================================

TEST(LinearAlgebraHelperTest, EqualRealNos_Equal) {
	EXPECT_TRUE(LA::EqualRealNos(1.0, 1.0));
	EXPECT_TRUE(LA::EqualRealNos(3.14, 3.14));
}

TEST(LinearAlgebraHelperTest, EqualRealNos_NearEqual) {
	EXPECT_TRUE(LA::EqualRealNos(1.0, 1.0 + 1e-6));
	EXPECT_TRUE(LA::EqualRealNos(1.0, 1.0 - 1e-6));
}

TEST(LinearAlgebraHelperTest, EqualRealNos_NotEqual) {
	EXPECT_FALSE(LA::EqualRealNos(1.0, 2.0));
	EXPECT_FALSE(LA::EqualRealNos(1.0, 1.1));
}

TEST(LinearAlgebraHelperTest, EqualRealNos_CustomEpsilon) {
	EXPECT_TRUE(LA::EqualRealNos(1.0, 1.05, 0.1));
	EXPECT_FALSE(LA::EqualRealNos(1.0, 1.2, 0.1));
}


// ============================================================================
// Size
// ============================================================================

TEST(LinearAlgebraHelperTest, Size_Matrix) {
	Eigen::MatrixXd m(3, 4);
	m.setZero();
	auto [rows, cols] = LA::Size(m);
	EXPECT_EQ(rows, 3);
	EXPECT_EQ(cols, 4);
}

TEST(LinearAlgebraHelperTest, Size_MatrixDim) {
	Eigen::MatrixXd m(3, 4);
	m.setZero();
	EXPECT_EQ(LA::Size(m, 1), 3);
	EXPECT_EQ(LA::Size(m, 2), 4);
}

TEST(LinearAlgebraHelperTest, Size_2DVector) {
	std::vector<std::vector<double>> m = {{1, 2, 3}, {4, 5, 6}};
	auto [rows, cols] = LA::Size(m);
	EXPECT_EQ(rows, 2);
	EXPECT_EQ(cols, 3);
}


// ============================================================================
// AddSortedValue
// ============================================================================

TEST(LinearAlgebraHelperTest, AddSortedValue_Middle) {
	Eigen::VectorXd v(3);
	v << 1.0, 3.0, 5.0;
	Eigen::VectorXd result = LA::AddSortedValue(v, 2.0);
	ASSERT_EQ(result.size(), 4);
	// 应按排序顺序包含 1, 2, 3, 5
	for (int i = 0; i < result.size() - 1; ++i) {
		EXPECT_LE(result(i), result(i + 1));
	}
}

TEST(LinearAlgebraHelperTest, AddSortedValue_Beginning) {
	Eigen::VectorXd v(3);
	v << 2.0, 3.0, 5.0;
	Eigen::VectorXd result = LA::AddSortedValue(v, 1.0);
	ASSERT_EQ(result.size(), 4);
	EXPECT_DOUBLE_EQ(result(0), 1.0);
}

TEST(LinearAlgebraHelperTest, AddSortedValue_End) {
	Eigen::VectorXd v(3);
	v << 1.0, 3.0, 5.0;
	Eigen::VectorXd result = LA::AddSortedValue(v, 10.0);
	ASSERT_EQ(result.size(), 4);
	EXPECT_DOUBLE_EQ(result(3), 10.0);
}


// ============================================================================
// FindClosestIndexAndValue
// ============================================================================

TEST(LinearAlgebraHelperTest, FindClosestIndexAndValue_Exact) {
	std::vector<double> v = {1.0, 3.0, 5.0, 7.0};
	auto [index, value] = LA::FindClosestIndexAndValue(v, 3.0);
	EXPECT_EQ(index, 1);
	EXPECT_DOUBLE_EQ(value, 3.0);
}

TEST(LinearAlgebraHelperTest, FindClosestIndexAndValue_Approximate) {
	std::vector<double> v = {1.0, 3.0, 5.0, 7.0};
	auto [index, value] = LA::FindClosestIndexAndValue(v, 4.0);
	EXPECT_TRUE(index == 1 || index == 2);
}

TEST(LinearAlgebraHelperTest, FindClosestIndexAndValue_Int) {
	std::vector<int> v = {10, 20, 30, 40};
	auto [index, value] = LA::FindClosestIndexAndValue(v, 25);
	EXPECT_TRUE(index == 1 || index == 2);
}


// ============================================================================
// Sort
// ============================================================================

TEST(LinearAlgebraHelperTest, Sort_Double) {
	std::vector<double> v = {3.0, 1.0, 4.0, 1.5, 9.0, 2.6};
	auto result = LA::Sort(v);
	for (size_t i = 0; i < result.size() - 1; ++i) {
		EXPECT_LE(result[i], result[i + 1]);
	}
}

TEST(LinearAlgebraHelperTest, Sort_Int) {
	std::vector<int> v = {5, 2, 8, 1, 9};
	auto result = LA::Sort(v);
	for (size_t i = 0; i < result.size() - 1; ++i) {
		EXPECT_LE(result[i], result[i + 1]);
	}
}

TEST(LinearAlgebraHelperTest, Sort_AlreadySorted) {
	std::vector<double> v = {1.0, 2.0, 3.0};
	auto result = LA::Sort(v);
	EXPECT_EQ(result[0], 1.0);
	EXPECT_EQ(result[1], 2.0);
	EXPECT_EQ(result[2], 3.0);
}


// ============================================================================
// meshgrid
// ============================================================================

TEST(LinearAlgebraHelperTest, Meshgrid_Basic) {
	Eigen::VectorXd x(3);
	x << 1.0, 2.0, 3.0;
	Eigen::VectorXd y(2);
	y << 10.0, 20.0;

	auto [xx, yy] = LA::meshgrid(x, y);
	EXPECT_EQ(xx.rows(), 3);  // x.size()
	EXPECT_EQ(xx.cols(), 2);  // y.size()
	EXPECT_EQ(yy.rows(), 3);
	EXPECT_EQ(yy.cols(), 2);

	// xx的每一列都应该是x
	for (int j = 0; j < xx.cols(); ++j) {
		EXPECT_DOUBLE_EQ(xx(0, j), 1.0);
		EXPECT_DOUBLE_EQ(xx(1, j), 2.0);
		EXPECT_DOUBLE_EQ(xx(2, j), 3.0);
	}

	// yy的每一行都应该是y
	for (int i = 0; i < yy.rows(); ++i) {
		EXPECT_DOUBLE_EQ(yy(i, 0), 10.0);
		EXPECT_DOUBLE_EQ(yy(i, 1), 20.0);
	}
}


// ============================================================================
// LDL decomposition
// ============================================================================

TEST(LinearAlgebraHelperTest, LDL_SymmetricPositiveDefinite) {
	Eigen::MatrixXd A(3, 3);
	A << 4, 2, 1,
		 2, 5, 3,
		 1, 3, 6;

	auto [L, D] = LA::LDL(A);
	EXPECT_EQ(L.rows(), 3);
	EXPECT_EQ(L.cols(), 3);
	EXPECT_EQ(D.rows(), 3);
	EXPECT_EQ(D.cols(), 3);

	// 验证 L*D*L^T ≈ A
	Eigen::MatrixXd reconstructed = L * D * L.transpose();
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(reconstructed(i, j), A(i, j), 1e-10);
		}
	}
}


// ============================================================================
// Mul
// ============================================================================

TEST(LinearAlgebraHelperTest, Mul_Basic) {
	Eigen::MatrixXd A(2, 3);
	A << 1, 2, 3,
		 4, 5, 6;
	Eigen::MatrixXd B(3, 2);
	B << 7, 8,
		 9, 10,
		 11, 12;
	Eigen::MatrixXd C = Eigen::MatrixXd::Zero(2, 2);

	Eigen::MatrixXd result = LA::Mul(C, A, B);
	Eigen::MatrixXd expected = A * B;
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			EXPECT_NEAR(result(i, j), expected(i, j), 1e-10);
		}
	}
}

TEST(LinearAlgebraHelperTest, Mul_WithAlphaBeta) {
	Eigen::MatrixXd A(2, 2);
	A << 1, 2,
		 3, 4;
	Eigen::MatrixXd B(2, 2);
	B << 5, 6,
		 7, 8;
	Eigen::MatrixXd C(2, 2);
	C << 1, 1,
		 1, 1;

	double alpha = 2.0, beta = 3.0;
	Eigen::MatrixXd result = LA::Mul(C, A, B, alpha, beta);

	// 预期: alpha * (A*B) + beta * C_original
	Eigen::MatrixXd C_orig(2, 2);
	C_orig << 1, 1, 1, 1;
	Eigen::MatrixXd expected = alpha * (A * B) + beta * C_orig;
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			EXPECT_NEAR(result(i, j), expected(i, j), 1e-10);
		}
	}
}
