// test_yaml_type_converter.cpp - YamlTypeConverter 类的单元测试
#include <gtest/gtest.h>
#include <cmath>
#include "../../../src/IO/Yaml/YamlTypeConverter.h"

using namespace Qahse::IO::Yaml;

// ============================================================================
// YmlToBool
// ============================================================================

TEST(YamlTypeConverterTest, YmlToBool_True) {
	EXPECT_TRUE(YamlTypeConverter::YmlToBool("true"));
	EXPECT_TRUE(YamlTypeConverter::YmlToBool("True"));
	EXPECT_TRUE(YamlTypeConverter::YmlToBool("TRUE"));
	EXPECT_TRUE(YamlTypeConverter::YmlToBool("  true  "));
}

TEST(YamlTypeConverterTest, YmlToBool_False) {
	EXPECT_FALSE(YamlTypeConverter::YmlToBool("false"));
	EXPECT_FALSE(YamlTypeConverter::YmlToBool("False"));
	EXPECT_FALSE(YamlTypeConverter::YmlToBool("FALSE"));
}

TEST(YamlTypeConverterTest, YmlToBool_Empty) {
	EXPECT_FALSE(YamlTypeConverter::YmlToBool(""));
	EXPECT_TRUE(YamlTypeConverter::YmlToBool("", "", true));
}

TEST(YamlTypeConverterTest, YmlToBool_Invalid) {
	// 无效字符串返回默认值
	EXPECT_FALSE(YamlTypeConverter::YmlToBool("yes"));
	EXPECT_TRUE(YamlTypeConverter::YmlToBool("invalid", "test", true));
}


// ============================================================================
// YmlToBoolArray
// ============================================================================

TEST(YamlTypeConverterTest, YmlToBoolArray_Normal) {
	auto result = YamlTypeConverter::YmlToBoolArray("[true, false, true]");
	ASSERT_EQ(result.size(), 3);
	EXPECT_TRUE(result[0]);
	EXPECT_FALSE(result[1]);
	EXPECT_TRUE(result[2]);
}

TEST(YamlTypeConverterTest, YmlToBoolArray_Empty) {
	auto result = YamlTypeConverter::YmlToBoolArray("");
	EXPECT_TRUE(result.empty());
}


// ============================================================================
// YmlToInt
// ============================================================================

TEST(YamlTypeConverterTest, YmlToInt_Normal) {
	EXPECT_EQ(YamlTypeConverter::YmlToInt("42"), 42);
	EXPECT_EQ(YamlTypeConverter::YmlToInt("-1"), -1);
	EXPECT_EQ(YamlTypeConverter::YmlToInt("0"), 0);
}

TEST(YamlTypeConverterTest, YmlToInt_WithSpaces) {
	EXPECT_EQ(YamlTypeConverter::YmlToInt("  100  "), 100);
}

TEST(YamlTypeConverterTest, YmlToInt_Empty) {
	EXPECT_EQ(YamlTypeConverter::YmlToInt(""), 0);
}

TEST(YamlTypeConverterTest, YmlToInt_Invalid) {
	EXPECT_EQ(YamlTypeConverter::YmlToInt("abc"), 0);
}


// ============================================================================
// YmlToIntArray
// ============================================================================

TEST(YamlTypeConverterTest, YmlToIntArray_Normal) {
	auto result = YamlTypeConverter::YmlToIntArray("[1, 2, 3, 4]");
	ASSERT_EQ(result.size(), 4);
	EXPECT_EQ(result[0], 1);
	EXPECT_EQ(result[1], 2);
	EXPECT_EQ(result[2], 3);
	EXPECT_EQ(result[3], 4);
}

TEST(YamlTypeConverterTest, YmlToIntArray_Empty) {
	auto result = YamlTypeConverter::YmlToIntArray("");
	EXPECT_TRUE(result.empty());
}

TEST(YamlTypeConverterTest, YmlToIntArray_Single) {
	auto result = YamlTypeConverter::YmlToIntArray("[99]");
	ASSERT_EQ(result.size(), 1);
	EXPECT_EQ(result[0], 99);
}


// ============================================================================
// YmlToDouble
// ============================================================================

TEST(YamlTypeConverterTest, YmlToDouble_Normal) {
	EXPECT_DOUBLE_EQ(YamlTypeConverter::YmlToDouble("3.14"), 3.14);
	EXPECT_DOUBLE_EQ(YamlTypeConverter::YmlToDouble("-2.5"), -2.5);
	EXPECT_DOUBLE_EQ(YamlTypeConverter::YmlToDouble("0.0"), 0.0);
}

TEST(YamlTypeConverterTest, YmlToDouble_Integer) {
	EXPECT_DOUBLE_EQ(YamlTypeConverter::YmlToDouble("5"), 5.0);
}

TEST(YamlTypeConverterTest, YmlToDouble_Scientific) {
	EXPECT_NEAR(YamlTypeConverter::YmlToDouble("1.5e2"), 150.0, 1e-10);
	EXPECT_NEAR(YamlTypeConverter::YmlToDouble("2.0E-3"), 0.002, 1e-10);
}

TEST(YamlTypeConverterTest, YmlToDouble_Empty) {
	EXPECT_DOUBLE_EQ(YamlTypeConverter::YmlToDouble(""), 0.0);
}

TEST(YamlTypeConverterTest, YmlToDouble_Invalid) {
	EXPECT_DOUBLE_EQ(YamlTypeConverter::YmlToDouble("abc"), 0.0);
}


// ============================================================================
// YmlToDoubleArray
// ============================================================================

TEST(YamlTypeConverterTest, YmlToDoubleArray_Normal) {
	auto result = YamlTypeConverter::YmlToDoubleArray("[1.1, 2.2, 3.3]");
	ASSERT_EQ(result.size(), 3);
	EXPECT_NEAR(result[0], 1.1, 1e-10);
	EXPECT_NEAR(result[1], 2.2, 1e-10);
	EXPECT_NEAR(result[2], 3.3, 1e-10);
}

TEST(YamlTypeConverterTest, YmlToDoubleArray_Empty) {
	auto result = YamlTypeConverter::YmlToDoubleArray("");
	EXPECT_TRUE(result.empty());
}


// ============================================================================
// YmlToFloat
// ============================================================================

TEST(YamlTypeConverterTest, YmlToFloat_Normal) {
	EXPECT_FLOAT_EQ(YamlTypeConverter::YmlToFloat("1.5"), 1.5f);
	EXPECT_FLOAT_EQ(YamlTypeConverter::YmlToFloat("-0.25"), -0.25f);
}

TEST(YamlTypeConverterTest, YmlToFloat_Empty) {
	EXPECT_FLOAT_EQ(YamlTypeConverter::YmlToFloat(""), 0.0f);
}

TEST(YamlTypeConverterTest, YmlToFloat_Invalid) {
	EXPECT_FLOAT_EQ(YamlTypeConverter::YmlToFloat("xyz"), 0.0f);
}


// ============================================================================
// YmlToFloatArray
// ============================================================================

TEST(YamlTypeConverterTest, YmlToFloatArray_Normal) {
	auto result = YamlTypeConverter::YmlToFloatArray("[1.0, 2.0, 3.0]");
	ASSERT_EQ(result.size(), 3);
	EXPECT_FLOAT_EQ(result[0], 1.0f);
	EXPECT_FLOAT_EQ(result[1], 2.0f);
	EXPECT_FLOAT_EQ(result[2], 3.0f);
}

TEST(YamlTypeConverterTest, YmlToFloatArray_Empty) {
	auto result = YamlTypeConverter::YmlToFloatArray("");
	EXPECT_TRUE(result.empty());
}


// ============================================================================
// YmlToVector
// ============================================================================

TEST(YamlTypeConverterTest, YmlToVector_Normal) {
	auto vec = YamlTypeConverter::YmlToVector("[1.0, 2.0, 3.0]");
	ASSERT_EQ(vec.size(), 3);
	EXPECT_DOUBLE_EQ(vec(0), 1.0);
	EXPECT_DOUBLE_EQ(vec(1), 2.0);
	EXPECT_DOUBLE_EQ(vec(2), 3.0);
}

TEST(YamlTypeConverterTest, YmlToVector_Single) {
	auto vec = YamlTypeConverter::YmlToVector("[42.0]");
	ASSERT_EQ(vec.size(), 1);
	EXPECT_DOUBLE_EQ(vec(0), 42.0);
}

TEST(YamlTypeConverterTest, YmlToVector_Empty) {
	auto vec = YamlTypeConverter::YmlToVector("");
	EXPECT_EQ(vec.size(), 0);
}


// ============================================================================
// YmlToString
// ============================================================================

TEST(YamlTypeConverterTest, YmlToString_Normal) {
	EXPECT_EQ(YamlTypeConverter::YmlToString("hello world"), "hello world");
}

TEST(YamlTypeConverterTest, YmlToString_Empty) {
	EXPECT_EQ(YamlTypeConverter::YmlToString(""), "");
}


// ============================================================================
// YmlToStringArray
// ============================================================================

TEST(YamlTypeConverterTest, YmlToStringArray_Normal) {
	auto result = YamlTypeConverter::YmlToStringArray("[apple, banana, cherry]");
	ASSERT_EQ(result.size(), 3);
	EXPECT_EQ(result[0], "apple");
	EXPECT_EQ(result[1], "banana");
	EXPECT_EQ(result[2], "cherry");
}

TEST(YamlTypeConverterTest, YmlToStringArray_Empty) {
	auto result = YamlTypeConverter::YmlToStringArray("");
	EXPECT_TRUE(result.empty());
}


// ============================================================================
// YmlTo2DDoubleArray
// ============================================================================

TEST(YamlTypeConverterTest, YmlTo2DDoubleArray_Normal) {
	std::string yml = "\n-  [1.0, 2.0, 3.0]\n-  [4.0, 5.0, 6.0]";
	auto [arr, rows, cols] = YamlTypeConverter::YmlTo2DDoubleArray(yml);
	EXPECT_EQ(rows, 2);
	EXPECT_EQ(cols, 3);
	EXPECT_NEAR(arr[0][0], 1.0, 1e-10);
	EXPECT_NEAR(arr[0][2], 3.0, 1e-10);
	EXPECT_NEAR(arr[1][0], 4.0, 1e-10);
	EXPECT_NEAR(arr[1][2], 6.0, 1e-10);
}

TEST(YamlTypeConverterTest, YmlTo2DDoubleArray_Empty) {
	auto [arr, rows, cols] = YamlTypeConverter::YmlTo2DDoubleArray("");
	EXPECT_EQ(rows, 0);
	EXPECT_EQ(cols, 0);
	EXPECT_TRUE(arr.empty());
}


// ============================================================================
// YmlToMatrix
// ============================================================================

TEST(YamlTypeConverterTest, YmlToMatrix_Normal) {
	std::string yml = "\n-  [1.0, 2.0]\n-  [3.0, 4.0]";
	auto [mat, rows, cols] = YamlTypeConverter::YmlToMatrix(yml);
	EXPECT_EQ(rows, 2);
	EXPECT_EQ(cols, 2);
	EXPECT_DOUBLE_EQ(mat(0, 0), 1.0);
	EXPECT_DOUBLE_EQ(mat(0, 1), 2.0);
	EXPECT_DOUBLE_EQ(mat(1, 0), 3.0);
	EXPECT_DOUBLE_EQ(mat(1, 1), 4.0);
}

TEST(YamlTypeConverterTest, YmlToMatrix_Empty) {
	auto [mat, rows, cols] = YamlTypeConverter::YmlToMatrix("");
	EXPECT_EQ(rows, 0);
	EXPECT_EQ(cols, 0);
}


// ============================================================================
// YmlToEnum
// ============================================================================

enum class TestEnum : int { A = 0, B = 1, C = 2 };

TEST(YamlTypeConverterTest, YmlToEnum_Normal) {
	EXPECT_EQ(YamlTypeConverter::YmlToEnum<TestEnum>("0"), TestEnum::A);
	EXPECT_EQ(YamlTypeConverter::YmlToEnum<TestEnum>("1"), TestEnum::B);
	EXPECT_EQ(YamlTypeConverter::YmlToEnum<TestEnum>("2"), TestEnum::C);
}

TEST(YamlTypeConverterTest, YmlToEnum_Invalid) {
	// 无效字符串返回默认值 0
	EXPECT_EQ(YamlTypeConverter::YmlToEnum<TestEnum>("invalid"), TestEnum::A);
}
