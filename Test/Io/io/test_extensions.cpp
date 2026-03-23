// test_extensions.cpp - Extensions 类的单元测试
#include <gtest/gtest.h>
#include <cmath>
#include <string>
#include <vector>
#include <Eigen/Dense>
#include "../../../src/IO/IO/Extensions.h"

using namespace HawtC3::IO::IO;

// ============================================================================
// Matrix Max (double)
// ============================================================================

TEST(ExtensionsTest, MaxDouble_SimpleMatrix) {
	Eigen::MatrixXd m(2, 2);
	m << 1.0, 2.0,
		 3.0, 4.0;
	EXPECT_DOUBLE_EQ(Extensions::Max(m), 4.0);
}

TEST(ExtensionsTest, MaxDouble_Negative) {
	Eigen::MatrixXd m(2, 2);
	m << -4.0, -3.0,
		 -2.0, -1.0;
	EXPECT_DOUBLE_EQ(Extensions::Max(m), -1.0);
}

TEST(ExtensionsTest, MaxDouble_SingleElement) {
	Eigen::MatrixXd m(1, 1);
	m << 5.0;
	EXPECT_DOUBLE_EQ(Extensions::Max(m), 5.0);
}


// ============================================================================
// Matrix Min (double)
// ============================================================================

TEST(ExtensionsTest, MinDouble_SimpleMatrix) {
	Eigen::MatrixXd m(2, 2);
	m << 1.0, 2.0,
		 3.0, 4.0;
	EXPECT_DOUBLE_EQ(Extensions::Min(m), 1.0);
}

TEST(ExtensionsTest, MinDouble_Negative) {
	Eigen::MatrixXd m(2, 2);
	m << -4.0, -3.0,
		 -2.0, -1.0;
	EXPECT_DOUBLE_EQ(Extensions::Min(m), -4.0);
}


// ============================================================================
// Matrix Mean (double)
// ============================================================================

TEST(ExtensionsTest, MeanDouble_SimpleMatrix) {
	Eigen::MatrixXd m(2, 2);
	m << 1.0, 2.0,
		 3.0, 4.0;
	EXPECT_DOUBLE_EQ(Extensions::Mean(m), 2.5);
}

TEST(ExtensionsTest, MeanDouble_AllSame) {
	Eigen::MatrixXd m(3, 3);
	m.setConstant(5.0);
	EXPECT_DOUBLE_EQ(Extensions::Mean(m), 5.0);
}


// ============================================================================
// Matrix AbsMax / AbsMin (double)
// ============================================================================

TEST(ExtensionsTest, AbsMaxDouble) {
	Eigen::MatrixXd m(2, 2);
	m << -5.0, 2.0,
		  3.0, -4.0;
	EXPECT_DOUBLE_EQ(Extensions::AbsMax(m), 5.0);
}

TEST(ExtensionsTest, AbsMinDouble) {
	Eigen::MatrixXd m(2, 2);
	m << -5.0, 2.0,
		  3.0, -4.0;
	EXPECT_DOUBLE_EQ(Extensions::AbsMin(m), 2.0);
}


// ============================================================================
// Std (double)
// ============================================================================

TEST(ExtensionsTest, StdDouble_Dim1) {
	Eigen::MatrixXd m(2, 3);
	m << 1.0, 2.0, 3.0,
		 4.0, 5.0, 6.0;
	Eigen::VectorXd result = Extensions::Std(m, 1);
	ASSERT_EQ(result.size(), 2);
	// 每行的标准差: row0=[1,2,3] std=1, row1=[4,5,6] std=1
	EXPECT_NEAR(result(0), 1.0, 1e-10);
	EXPECT_NEAR(result(1), 1.0, 1e-10);
}

TEST(ExtensionsTest, StdDouble_Dim2) {
	Eigen::MatrixXd m(2, 3);
	m << 1.0, 2.0, 3.0,
		 4.0, 5.0, 6.0;
	Eigen::VectorXd result = Extensions::Std(m, 2);
	ASSERT_EQ(result.size(), 3);
	// 每列的标准差: col0=[1,4] std=2.1213..., col1=[2,5], col2=[3,6]
	double expected = std::sqrt(((1.0 - 2.5) * (1.0 - 2.5) + (4.0 - 2.5) * (4.0 - 2.5)) / 1.0);
	EXPECT_NEAR(result(0), expected, 1e-10);
}


// ============================================================================
// Float Matrix Operations
// ============================================================================

TEST(ExtensionsTest, MaxFloat) {
	Eigen::MatrixXf m(2, 2);
	m << 1.0f, 2.0f,
		 3.0f, 4.0f;
	EXPECT_FLOAT_EQ(Extensions::Max(m), 4.0f);
}

TEST(ExtensionsTest, MinFloat) {
	Eigen::MatrixXf m(2, 2);
	m << 1.0f, 2.0f,
		 3.0f, 4.0f;
	EXPECT_FLOAT_EQ(Extensions::Min(m), 1.0f);
}

TEST(ExtensionsTest, MeanFloat) {
	Eigen::MatrixXf m(2, 2);
	m << 1.0f, 2.0f,
		 3.0f, 4.0f;
	EXPECT_FLOAT_EQ(Extensions::Mean(m), 2.5f);
}

TEST(ExtensionsTest, AbsMaxFloat) {
	Eigen::MatrixXf m(2, 2);
	m << -5.0f, 2.0f,
		  3.0f, -4.0f;
	EXPECT_FLOAT_EQ(Extensions::AbsMax(m), 5.0f);
}

TEST(ExtensionsTest, AbsMinFloat) {
	Eigen::MatrixXf m(2, 2);
	m << -5.0f, 2.0f,
		  3.0f, -4.0f;
	EXPECT_FLOAT_EQ(Extensions::AbsMin(m), 2.0f);
}


// ============================================================================
// String Operations
// ============================================================================

TEST(ExtensionsTest, GetFileNameWithoutExtension) {
	EXPECT_EQ(Extensions::GetFileNameWithoutExtension("test.txt"), "test");
	EXPECT_EQ(Extensions::GetFileNameWithoutExtension("archive.tar.gz"), "archive.tar");
	EXPECT_EQ(Extensions::GetFileNameWithoutExtension("noext"), "noext");
}


// ============================================================================
// RemoveNull / TrimNull
// ============================================================================

TEST(ExtensionsTest, RemoveNull_RemovesEmpty) {
	std::vector<std::string> input = {"a", "", "b", "", "c"};
	auto result = Extensions::RemoveNull(input);
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

TEST(ExtensionsTest, RemoveNull_AllEmpty) {
	std::vector<std::string> input = {"", "", ""};
	auto result = Extensions::RemoveNull(input);
	EXPECT_TRUE(result.empty());
}

TEST(ExtensionsTest, RemoveNull_NoEmpty) {
	std::vector<std::string> input = {"a", "b"};
	auto result = Extensions::RemoveNull(input);
	ASSERT_EQ(result.size(), 2);
}

TEST(ExtensionsTest, TrimNull_TrimsWhitespace) {
	std::vector<std::string> input = {"  hello  ", "\tworld\t", " foo "};
	auto result = Extensions::TrimNull(input);
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result[0], "hello");
	EXPECT_EQ(result[1], "world");
	EXPECT_EQ(result[2], "foo");
}


// ============================================================================
// ToInt / ToDouble / ToBool
// ============================================================================

TEST(ExtensionsTest, ToInt_Normal) {
	EXPECT_EQ(Extensions::ToInt("42"), 42);
	EXPECT_EQ(Extensions::ToInt("-1"), -1);
	EXPECT_EQ(Extensions::ToInt("0"), 0);
}

TEST(ExtensionsTest, ToInt_WithSpaces) {
	EXPECT_EQ(Extensions::ToInt("  100  "), 100);
}

TEST(ExtensionsTest, ToDouble_Normal) {
	EXPECT_DOUBLE_EQ(Extensions::ToDouble("3.14"), 3.14);
	EXPECT_DOUBLE_EQ(Extensions::ToDouble("-2.5"), -2.5);
}

TEST(ExtensionsTest, ToDouble_WithSpaces) {
	EXPECT_DOUBLE_EQ(Extensions::ToDouble("  1.5  "), 1.5);
}

TEST(ExtensionsTest, ToDouble_Scientific) {
	EXPECT_NEAR(Extensions::ToDouble("1.5e2"), 150.0, 1e-10);
}

TEST(ExtensionsTest, ToBool_True) {
	EXPECT_TRUE(Extensions::ToBool("true"));
	EXPECT_TRUE(Extensions::ToBool("  True  "));
	EXPECT_TRUE(Extensions::ToBool("TRUE"));
}

TEST(ExtensionsTest, ToBool_False) {
	EXPECT_FALSE(Extensions::ToBool("false"));
	EXPECT_FALSE(Extensions::ToBool("  False  "));
}

TEST(ExtensionsTest, ToBool_Invalid) {
	EXPECT_THROW(Extensions::ToBool("yes"), std::invalid_argument);
	EXPECT_THROW(Extensions::ToBool(""), std::invalid_argument);
}


// ============================================================================
// ToIntArray / ToDoubleArray / ToStringArray
// ============================================================================

TEST(ExtensionsTest, ToIntArray_Normal) {
	auto result = Extensions::ToIntArray("1,2,3,4", {','});
	ASSERT_EQ(result.size(), 4);
	EXPECT_EQ(result[0], 1);
	EXPECT_EQ(result[1], 2);
	EXPECT_EQ(result[2], 3);
	EXPECT_EQ(result[3], 4);
}

TEST(ExtensionsTest, ToIntArray_WithSpaces) {
	auto result = Extensions::ToIntArray("1, 2, 3", {','});
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result[0], 1);
	EXPECT_EQ(result[1], 2);
	EXPECT_EQ(result[2], 3);
}

TEST(ExtensionsTest, ToDoubleArray_Normal) {
	auto result = Extensions::ToDoubleArray("1.1,2.2,3.3", {','});
	ASSERT_EQ(result.size(), 3);
	EXPECT_NEAR(result[0], 1.1, 1e-10);
	EXPECT_NEAR(result[1], 2.2, 1e-10);
	EXPECT_NEAR(result[2], 3.3, 1e-10);
}

TEST(ExtensionsTest, ToDoubleArray_MultipleDelimiters) {
	auto result = Extensions::ToDoubleArray("1.0 2.0\t3.0", {' ', '\t'});
	ASSERT_EQ(result.size(), 3);
	EXPECT_NEAR(result[0], 1.0, 1e-10);
	EXPECT_NEAR(result[1], 2.0, 1e-10);
	EXPECT_NEAR(result[2], 3.0, 1e-10);
}

TEST(ExtensionsTest, ToStringArray_Normal) {
	auto result = Extensions::ToStringArray("a,b,c", {','});
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}
