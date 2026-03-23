// test_binary_file_handler.cpp - BinaryFile 类的单元测试
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstdio>
#include <filesystem>
#include <Eigen/Dense>
#include "../../../src/IO/IO/BinaryFileHandler.h"

using namespace HawtC3::IO::IO;
namespace fs = std::filesystem;

// RAII 临时文件助手
struct TempBinaryFile {
	std::string path;
	TempBinaryFile(const std::string& name = "test_binary_temp.bin")
		: path((fs::temp_directory_path() / name).string()) {}
	~TempBinaryFile() {
		try { fs::remove(path); } catch (...) {}
	}
};


// ============================================================================
// 构造 / 析构
// ============================================================================

TEST(BinaryFileTest, WriteMode_CreatesFile) {
	TempBinaryFile tmp("bf_write_test.bin");
	{
		BinaryFile bf(tmp.path, "write");
		bf.WriteData(42);
		bf.Close();
	}
	EXPECT_TRUE(fs::exists(tmp.path));
}

TEST(BinaryFileTest, ReadMode_OpensExistingFile) {
	TempBinaryFile tmp("bf_read_test.bin");
	{
		BinaryFile writer(tmp.path, "write");
		writer.WriteData(100);
		writer.Close();
	}
	{
		BinaryFile reader(tmp.path, "read");
		int val = reader.ReadData<int>();
		EXPECT_EQ(val, 100);
		reader.Close();
	}
}


// ============================================================================
// 基本类型读写
// ============================================================================

TEST(BinaryFileTest, WriteAndRead_Int) {
	TempBinaryFile tmp("bf_int.bin");
	{
		BinaryFile bf(tmp.path, "w");
		bf.WriteData(12345);
		bf.WriteData(-99);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		EXPECT_EQ(bf.ReadData<int>(), 12345);
		EXPECT_EQ(bf.ReadData<int>(), -99);
		bf.Close();
	}
}

TEST(BinaryFileTest, WriteAndRead_Double) {
	TempBinaryFile tmp("bf_double.bin");
	{
		BinaryFile bf(tmp.path, "w");
		bf.WriteData(3.14159265);
		bf.WriteData(-2.71828);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		EXPECT_NEAR(bf.ReadData<double>(), 3.14159265, 1e-10);
		EXPECT_NEAR(bf.ReadData<double>(), -2.71828, 1e-10);
		bf.Close();
	}
}

TEST(BinaryFileTest, WriteAndRead_Float) {
	TempBinaryFile tmp("bf_float.bin");
	{
		BinaryFile bf(tmp.path, "w");
		bf.WriteData(1.5f);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		EXPECT_NEAR(bf.ReadData<float>(), 1.5f, 1e-6f);
		bf.Close();
	}
}

TEST(BinaryFileTest, WriteAndRead_String) {
	TempBinaryFile tmp("bf_string.bin");
	std::string msg = "Hello BinaryFile!";
	{
		BinaryFile bf(tmp.path, "w");
		bf.WriteData(msg);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		EXPECT_EQ(bf.ReadData<std::string>(), msg);
		bf.Close();
	}
}

TEST(BinaryFileTest, WriteAndRead_Bool) {
	TempBinaryFile tmp("bf_bool.bin");
	{
		BinaryFile bf(tmp.path, "w");
		bf.WriteData(true);
		bf.WriteData(false);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		EXPECT_EQ(bf.ReadData<bool>(), true);
		EXPECT_EQ(bf.ReadData<bool>(), false);
		bf.Close();
	}
}


// ============================================================================
// 数组类型读写
// ============================================================================

TEST(BinaryFileTest, WriteAndRead_VectorDouble) {
	TempBinaryFile tmp("bf_vecdouble.bin");
	std::vector<double> data = {1.1, 2.2, 3.3, 4.4, 5.5};
	{
		BinaryFile bf(tmp.path, "w");
		bf.WriteData(data);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		auto result = bf.ReadData<std::vector<double>>();
		ASSERT_EQ(result.size(), data.size());
		for (size_t i = 0; i < data.size(); ++i) {
			EXPECT_NEAR(result[i], data[i], 1e-10);
		}
		bf.Close();
	}
}

TEST(BinaryFileTest, WriteAndRead_VectorInt) {
	TempBinaryFile tmp("bf_vecint.bin");
	std::vector<int> data = {10, 20, 30, 40};
	{
		BinaryFile bf(tmp.path, "w");
		bf.WriteData(data);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		auto result = bf.ReadData<std::vector<int>>();
		ASSERT_EQ(result.size(), data.size());
		for (size_t i = 0; i < data.size(); ++i) {
			EXPECT_EQ(result[i], data[i]);
		}
		bf.Close();
	}
}


// ============================================================================
// Append 模式
// ============================================================================

TEST(BinaryFileTest, AppendMode) {
	TempBinaryFile tmp("bf_append.bin");
	{
		BinaryFile bf(tmp.path, "w");
		bf.WriteData(1);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "a");
		bf.WriteData(2);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		EXPECT_EQ(bf.ReadData<int>(), 1);
		EXPECT_EQ(bf.ReadData<int>(), 2);
		bf.Close();
	}
}


// ============================================================================
// Read2last (EOF 检测)
// ============================================================================

TEST(BinaryFileTest, Read2last_DetectsEOF) {
	TempBinaryFile tmp("bf_eof.bin");
	{
		BinaryFile bf(tmp.path, "w");
		bf.WriteData(42);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		EXPECT_FALSE(bf.Read2last());
		bf.ReadData<int>();
		EXPECT_TRUE(bf.Read2last());
		bf.Close();
	}
}


// ============================================================================
// 混合类型读写
// ============================================================================

TEST(BinaryFileTest, MixedTypes) {
	TempBinaryFile tmp("bf_mixed.bin");
	{
		BinaryFile bf(tmp.path, "w");
		bf.WriteData(42);
		bf.WriteData(3.14);
		bf.WriteData(std::string("test"));
		bf.WriteData(true);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		EXPECT_EQ(bf.ReadData<int>(), 42);
		EXPECT_NEAR(bf.ReadData<double>(), 3.14, 1e-10);
		EXPECT_EQ(bf.ReadData<std::string>(), "test");
		EXPECT_EQ(bf.ReadData<bool>(), true);
		bf.Close();
	}
}


// ============================================================================
// Write / WriteLine 便捷方法
// ============================================================================

TEST(BinaryFileTest, Write_TemplateOverload) {
	TempBinaryFile tmp("bf_write_tpl.bin");
	{
		BinaryFile bf(tmp.path, "w");
		bf.Write(99);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		EXPECT_EQ(bf.ReadData<int>(), 99);
		bf.Close();
	}
}

TEST(BinaryFileTest, WriteLine_TemplateOverload) {
	TempBinaryFile tmp("bf_writeline_tpl.bin");
	{
		BinaryFile bf(tmp.path, "w");
		bf.WriteLine(77);
		bf.Close();
	}
	{
		BinaryFile bf(tmp.path, "r");
		EXPECT_EQ(bf.ReadData<int>(), 77);
		bf.Close();
	}
}
