// test_math_helper.cpp - MathHelper 类的单元测试
#define _USE_MATH_DEFINES
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <set>
#include <Eigen/Dense>
#include "../../../src/IO/Math/MathHelper.h"

using namespace Qahse::IO::Math;

// ============================================================================
// RandomSeed 类测试
// ============================================================================

TEST(RandomSeedTest, Randomd_ReturnsInRange) {
	MathHelper::RandomSeed rng(42);
	for (int i = 0; i < 100; ++i) {
		double v = rng.Randomd();
		EXPECT_GE(v, 0.0);
		EXPECT_LT(v, 1.0);
	}
}

TEST(RandomSeedTest, Randomd_SameSeedSameSequence) {
	MathHelper::RandomSeed rng1(12345);
	MathHelper::RandomSeed rng2(12345);
	for (int i = 0; i < 50; ++i) {
		EXPECT_EQ(rng1.Randomd(), rng2.Randomd());
	}
}

TEST(RandomSeedTest, Randomd_DifferentSeedDifferentSequence) {
	MathHelper::RandomSeed rng1(111);
	MathHelper::RandomSeed rng2(222);
	bool allSame = true;
	for (int i = 0; i < 10; ++i) {
		if (rng1.Randomd() != rng2.Randomd()) {
			allSame = false;
			break;
		}
	}
	EXPECT_FALSE(allSame);
}

TEST(RandomSeedTest, Randomi_ReturnsNonNegative) {
	MathHelper::RandomSeed rng(42);
	for (int i = 0; i < 100; ++i) {
		int v = rng.Randomi();
		EXPECT_GE(v, 0);
	}
}

TEST(RandomSeedTest, Randomi_ArrayWithRange) {
	MathHelper::RandomSeed rng(42);
	auto arr = rng.Randomi(50, 10, 20);
	ASSERT_EQ(static_cast<int>(arr.size()), 50);
	for (auto v : arr) {
		EXPECT_GE(v, 10);
		EXPECT_LE(v, 20);
	}
}

TEST(RandomSeedTest, Randomi_ArrayDefaultRange) {
	MathHelper::RandomSeed rng(42);
	auto arr = rng.Randomi(30);
	ASSERT_EQ(static_cast<int>(arr.size()), 30);
}

TEST(RandomSeedTest, Randomi_MatrixWithRange) {
	MathHelper::RandomSeed rng(42);
	auto mat = rng.Randomi(3, 4, 5, 15);
	ASSERT_EQ(mat.rows(), 3);
	ASSERT_EQ(mat.cols(), 4);
	for (int i = 0; i < mat.rows(); ++i) {
		for (int j = 0; j < mat.cols(); ++j) {
			EXPECT_GE(mat(i, j), 5);
			EXPECT_LE(mat(i, j), 15);
		}
	}
}

TEST(RandomSeedTest, Randomd_ArrayWithRange) {
	MathHelper::RandomSeed rng(42);
	auto arr = rng.Randomd(50, 2.0, 5.0);
	ASSERT_EQ(static_cast<int>(arr.size()), 50);
	for (auto v : arr) {
		EXPECT_GE(v, 2.0);
		EXPECT_LT(v, 5.0);
	}
}

TEST(RandomSeedTest, Randomd_RangeScalar) {
	MathHelper::RandomSeed rng(42);
	for (int i = 0; i < 100; ++i) {
		double v = rng.Randomd(5.0, 10.0);
		EXPECT_GE(v, 5.0);
		EXPECT_LT(v, 10.0);
	}
}

TEST(RandomSeedTest, Randomd_Matrix) {
	MathHelper::RandomSeed rng(42);
	auto mat = rng.Randomd(3, 4, -1.0, 1.0);
	ASSERT_EQ(mat.rows(), 3);
	ASSERT_EQ(mat.cols(), 4);
	for (int i = 0; i < mat.rows(); ++i) {
		for (int j = 0; j < mat.cols(); ++j) {
			EXPECT_GE(mat(i, j), -1.0);
			EXPECT_LT(mat(i, j), 1.0);
		}
	}
}

TEST(RandomSeedTest, Randomf_ArrayWithRange) {
	MathHelper::RandomSeed rng(42);
	auto arr = rng.Randomf(20, 0.0f, 10.0f);
	ASSERT_EQ(static_cast<int>(arr.size()), 20);
	for (auto v : arr) {
		EXPECT_GE(v, 0.0f);
		EXPECT_LT(v, 10.0f);
	}
}

TEST(RandomSeedTest, Randomf_VectorXf) {
	MathHelper::RandomSeed rng(42);
	auto vec = rng.Randomf(10, 1.0f, 5.0f, true);
	ASSERT_EQ(vec.size(), 10);
	for (int i = 0; i < vec.size(); ++i) {
		EXPECT_GE(vec(i), 1.0f);
		EXPECT_LT(vec(i), 5.0f);
	}
}

TEST(RandomSeedTest, Randomf_Matrix) {
	MathHelper::RandomSeed rng(42);
	auto mat = rng.Randomf(2, 3, 0.0f, 1.0f);
	ASSERT_EQ(mat.rows(), 2);
	ASSERT_EQ(mat.cols(), 3);
}

TEST(RandomSeedTest, Randomf_RangeScalar) {
	MathHelper::RandomSeed rng(42);
	for (int i = 0; i < 100; ++i) {
		float v = rng.Randomf(1.0f, 5.0f);
		EXPECT_GE(v, 1.0f);
		EXPECT_LT(v, 5.0f);
	}
}


// ============================================================================
// MathHelper 静态方法测试
// ============================================================================

TEST(MathHelperTest, StaticRandomd_ReturnsInRange) {
	for (int i = 0; i < 100; ++i) {
		double v = MathHelper::Randomd();
		EXPECT_GE(v, 0.0);
		EXPECT_LT(v, 1.0);
	}
}

TEST(MathHelperTest, StaticRandomi_Default) {
	for (int i = 0; i < 100; ++i) {
		int v = MathHelper::Randomi();
		EXPECT_GE(v, 0);
		EXPECT_LE(v, 100);
	}
}

TEST(MathHelperTest, StaticRandomi_Array) {
	// 使用函数指针消除 vector/matrix 重载歧义
	auto fn = static_cast<std::vector<int>(*)(int, int, int)>(&MathHelper::Randomi);
	auto arr = fn(10, 5, 20);
	ASSERT_EQ(static_cast<int>(arr.size()), 10);
	for (auto v : arr) {
		EXPECT_GE(v, 5);
		EXPECT_LE(v, 20);
	}
}

TEST(MathHelperTest, StaticRandomd_Array) {
	auto arr = MathHelper::Randomd(20, 0.0, 100.0);
	ASSERT_EQ(static_cast<int>(arr.size()), 20);
	for (auto v : arr) {
		EXPECT_GE(v, 0.0);
		EXPECT_LT(v, 100.0);
	}
}

TEST(MathHelperTest, StaticRandomf_Array) {
	auto arr = MathHelper::Randomf(10, 0.0f, 5.0f);
	ASSERT_EQ(static_cast<int>(arr.size()), 10);
}

TEST(MathHelperTest, StaticRandomi_Matrix) {
	auto mat = MathHelper::Randomi(3, 3, 0, 50);
	ASSERT_EQ(mat.rows(), 3);
	ASSERT_EQ(mat.cols(), 3);
	for (int i = 0; i < mat.rows(); ++i) {
		for (int j = 0; j < mat.cols(); ++j) {
			EXPECT_GE(mat(i, j), 0);
			EXPECT_LE(mat(i, j), 50);
		}
	}
}

TEST(MathHelperTest, StaticRandomd_Matrix) {
	auto mat = MathHelper::Randomd(2, 4, -1.0, 1.0);
	ASSERT_EQ(mat.rows(), 2);
	ASSERT_EQ(mat.cols(), 4);
}

TEST(MathHelperTest, StaticRandomf_Matrix) {
	auto mat = MathHelper::Randomf(3, 2);
	ASSERT_EQ(mat.rows(), 3);
	ASSERT_EQ(mat.cols(), 2);
}

TEST(MathHelperTest, StaticRandomd_RangeScalar) {
	double v = MathHelper::Randomd(10.0, 20.0);
	EXPECT_GE(v, 10.0);
	EXPECT_LT(v, 20.0);
}

TEST(MathHelperTest, StaticRandomf_RangeScalar) {
	float v = MathHelper::Randomf(1.0f, 2.0f);
	EXPECT_GE(v, 1.0f);
	EXPECT_LT(v, 2.0f);
}

TEST(MathHelperTest, StaticRandomi_SingleValue) {
	// 使用函数指针消除歧义: Randomi(int hh=0) 返回 [0,100] 随机整数
	auto fn = static_cast<int(*)(int)>(&MathHelper::Randomi);
	int v = fn(0);
	EXPECT_GE(v, 0);
	EXPECT_LE(v, 100);
}


// ============================================================================
// Zero2TwoPi
// ============================================================================

TEST(MathHelperTest, Zero2TwoPi_PositiveAngle) {
	double angle = MathHelper::Zero2TwoPi(M_PI / 4);
	EXPECT_NEAR(angle, M_PI / 4, 1e-10);
}

TEST(MathHelperTest, Zero2TwoPi_NegativeAngle) {
	double angle = MathHelper::Zero2TwoPi(-M_PI / 2);
	EXPECT_NEAR(angle, 3.0 * M_PI / 2.0, 1e-10);
}

TEST(MathHelperTest, Zero2TwoPi_LargeAngle) {
	double angle = MathHelper::Zero2TwoPi(3.0 * M_PI);
	EXPECT_NEAR(angle, M_PI, 1e-10);
}

TEST(MathHelperTest, Zero2TwoPi_Zero) {
	double angle = MathHelper::Zero2TwoPi(0.0);
	EXPECT_NEAR(angle, 0.0, 1e-10);
}


// ============================================================================
// MirrorRound
// ============================================================================

TEST(MathHelperTest, MirrorRound_TwoDecimalPlaces) {
	EXPECT_NEAR(MathHelper::MirrorRound(3.14159, 2), 3.14, 1e-10);
}

TEST(MathHelperTest, MirrorRound_ZeroDecimalPlaces) {
	EXPECT_NEAR(MathHelper::MirrorRound(123.456, 0), 123.0, 1e-10);
}

TEST(MathHelperTest, MirrorRound_FourDecimalPlaces) {
	EXPECT_NEAR(MathHelper::MirrorRound(2.71828, 4), 2.7183, 1e-10);
}


// ============================================================================
// Range
// ============================================================================

TEST(MathHelperTest, Range_IntStartCount) {
	auto result = MathHelper::Range(5, 3);
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result(0), 5);
	EXPECT_EQ(result(1), 6);
	EXPECT_EQ(result(2), 7);
}

TEST(MathHelperTest, Range_IntSingle) {
	auto result = MathHelper::Range(5);
	ASSERT_EQ(result.size(), 5);
	EXPECT_DOUBLE_EQ(result(0), 0.0);
	EXPECT_DOUBLE_EQ(result(4), 4.0);
}

TEST(MathHelperTest, Range_IntStartEndStep) {
	auto result = MathHelper::Range(0, 10, 2);
	ASSERT_EQ(result.size(), 6);
	EXPECT_EQ(result(0), 0);
	EXPECT_EQ(result(5), 10);
}

TEST(MathHelperTest, Range_DoubleStartEndStep) {
	auto result = MathHelper::Range(0.0, 10.0, 2.5);
	ASSERT_EQ(result.size(), 5);
	EXPECT_NEAR(result(0), 0.0, 1e-10);
	EXPECT_NEAR(result(4), 10.0, 1e-10);
}

TEST(MathHelperTest, Range_DoubleStartEndStep_ThrowsOnZeroStep) {
	EXPECT_THROW(MathHelper::Range(0.0, 10.0, 0.0), std::invalid_argument);
}

TEST(MathHelperTest, Range_FloatStartEndStep) {
	auto result = MathHelper::Range(0.0f, 4.0f, 1.0f);
	ASSERT_EQ(result.size(), 5);
	EXPECT_NEAR(result(0), 0.0f, 1e-5f);
	EXPECT_NEAR(result(4), 4.0f, 1e-5f);
}

TEST(MathHelperTest, Range_FloatStartStepLength) {
	auto result = MathHelper::Range(1.0f, 0.5f, 5);
	ASSERT_EQ(result.size(), 5);
	EXPECT_NEAR(result(0), 1.0f, 1e-5f);
	EXPECT_NEAR(result(4), 3.0f, 1e-5f);
}

TEST(MathHelperTest, Range_DoubleLinspace) {
	auto result = MathHelper::Range(0.0, 1.0, 5);
	ASSERT_EQ(result.size(), 5);
	EXPECT_NEAR(result(0), 0.0, 1e-10);
	EXPECT_NEAR(result(4), 1.0, 1e-10);
}


// ============================================================================
// linspace
// ============================================================================

TEST(MathHelperTest, Linspace_Double) {
	auto result = MathHelper::linspace(0.0, 10.0, 5);
	ASSERT_EQ(result.size(), 5);
	EXPECT_NEAR(result(0), 0.0, 1e-10);
	EXPECT_NEAR(result(1), 2.5, 1e-10);
	EXPECT_NEAR(result(4), 10.0, 1e-10);
}

TEST(MathHelperTest, Linspace_Single) {
	auto result = MathHelper::linspace(5.0, 5.0, 1);
	ASSERT_EQ(result.size(), 1);
	EXPECT_NEAR(result(0), 5.0, 1e-10);
}

TEST(MathHelperTest, Linspace_Float) {
	auto result = MathHelper::linspace(0.0f, 1.0f, 3);
	ASSERT_EQ(result.size(), 3);
	EXPECT_NEAR(result(0), 0.0f, 1e-5f);
	EXPECT_NEAR(result(1), 0.5f, 1e-5f);
	EXPECT_NEAR(result(2), 1.0f, 1e-5f);
}


// ============================================================================
// zeros
// ============================================================================

TEST(MathHelperTest, Zeros_2D_Default) {
	auto result = MathHelper::zeros(3, 4);
	ASSERT_EQ(result.rows(), 3);
	ASSERT_EQ(result.cols(), 4);
	EXPECT_DOUBLE_EQ(result(0, 0), 0.0);
	EXPECT_DOUBLE_EQ(result(2, 3), 0.0);
}

TEST(MathHelperTest, Zeros_2D_WithValue) {
	auto result = MathHelper::zeros(3, 4, 5.0);
	ASSERT_EQ(result.rows(), 3);
	ASSERT_EQ(result.cols(), 4);
	EXPECT_DOUBLE_EQ(result(0, 0), 5.0);
	EXPECT_DOUBLE_EQ(result(2, 3), 5.0);
}

TEST(MathHelperTest, Zeros_2D_Template) {
	auto result = MathHelper::zeros(1.0, 2, 3);
	ASSERT_EQ(result.rows(), 2);
	ASSERT_EQ(result.cols(), 3);
	EXPECT_DOUBLE_EQ(result(0, 0), 1.0);
}

TEST(MathHelperTest, Zeros_3D) {
	auto result = MathHelper::zeros(0.0, 2, 3, 4);
	ASSERT_EQ(static_cast<int>(result.size()), 2);
	ASSERT_EQ(result[0].rows(), 3);
	ASSERT_EQ(result[0].cols(), 4);
	EXPECT_DOUBLE_EQ(result[0](0, 0), 0.0);
}

TEST(MathHelperTest, Zeros_4D) {
	auto result = MathHelper::zeros(0.0, 2, 3, 4, 5);
	ASSERT_EQ(static_cast<int>(result.size()), 2);
	ASSERT_EQ(static_cast<int>(result[0].size()), 3);
	ASSERT_EQ(result[0][0].rows(), 4);
	ASSERT_EQ(result[0][0].cols(), 5);
}
