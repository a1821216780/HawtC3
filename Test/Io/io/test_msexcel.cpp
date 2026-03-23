// test_msexcel.cpp - MSExcel 类的单元测试
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <filesystem>
#include <Eigen/Dense>
#include "../../../src/IO/IO/MSExcel.h"

using namespace HawtC3::IO::IO;
namespace fs = std::filesystem;

// RAII 临时 Excel 文件助手
struct TempExcelFile {
	std::string path;
	TempExcelFile(const std::string& name = "test_msexcel_temp.xlsx")
		: path((fs::temp_directory_path() / name).string()) {}
	~TempExcelFile() {
		try { fs::remove(path); } catch (...) {}
	}
};


// ============================================================================
// 构造 / 析构
// ============================================================================

TEST(MSExcelTest, WriteMode_CreatesFile) {
	TempExcelFile tmp("msexcel_write_test.xlsx");
	{
		MSExcel excel(tmp.path, "write");
		excel.Close();
	}
	EXPECT_TRUE(fs::exists(tmp.path));
}


// ============================================================================
// Sheet 操作
// ============================================================================

TEST(MSExcelTest, AddSheet_CreatesNewSheet) {
	TempExcelFile tmp("msexcel_addsheet.xlsx");
	{
		MSExcel excel(tmp.path, "write");
		excel.AddSheet("TestSheet");
		EXPECT_TRUE(excel.SheetExist("TestSheet"));
		excel.Close();
	}
}

TEST(MSExcelTest, SheetExist_ReturnsFalseForMissing) {
	TempExcelFile tmp("msexcel_nosheet.xlsx");
	{
		MSExcel excel(tmp.path, "write");
		EXPECT_FALSE(excel.SheetExist("NonExistent"));
		excel.Close();
	}
}


// ============================================================================
// 基本类型读写 - double
// ============================================================================

TEST(MSExcelTest, WriteAndRead_Double) {
	TempExcelFile tmp("msexcel_double.xlsx");
	{
		MSExcel excel(tmp.path, "write");
		excel.AddSheet("Data");
		excel.WCellValue<double>("Data", 3.14159, 1, 1);
		excel.WCellValue<double>("Data", -2.71828, 2, 1);
		excel.Close();
	}
	{
		MSExcel excel(tmp.path, "read");
		double v1 = excel.RCellValue<double>("Data", 1, 1);
		double v2 = excel.RCellValue<double>("Data", 2, 1);
		EXPECT_NEAR(v1, 3.14159, 1e-5);
		EXPECT_NEAR(v2, -2.71828, 1e-5);
		excel.Close();
	}
}


// ============================================================================
// 基本类型读写 - int
// ============================================================================

TEST(MSExcelTest, WriteAndRead_Int) {
	TempExcelFile tmp("msexcel_int.xlsx");
	{
		MSExcel excel(tmp.path, "write");
		excel.AddSheet("Data");
		excel.WCellValue<int>("Data", 42, 1, 1);
		excel.WCellValue<int>("Data", -100, 1, 2);
		excel.Close();
	}
	{
		MSExcel excel(tmp.path, "read");
		int v1 = excel.RCellValue<int>("Data", 1, 1);
		int v2 = excel.RCellValue<int>("Data", 1, 2);
		EXPECT_EQ(v1, 42);
		EXPECT_EQ(v2, -100);
		excel.Close();
	}
}


// ============================================================================
// 基本类型读写 - string
// ============================================================================

TEST(MSExcelTest, WriteAndRead_String) {
	TempExcelFile tmp("msexcel_string.xlsx");
	{
		MSExcel excel(tmp.path, "write");
		excel.AddSheet("Data");
		excel.WCellValue<std::string>("Data", std::string("Hello"), 1, 1);
		excel.WCellValue<std::string>("Data", std::string("World"), 2, 1);
		excel.Close();
	}
	{
		MSExcel excel(tmp.path, "read");
		std::string v1 = excel.RCellValue<std::string>("Data", 1, 1);
		std::string v2 = excel.RCellValue<std::string>("Data", 2, 1);
		EXPECT_EQ(v1, "Hello");
		EXPECT_EQ(v2, "World");
		excel.Close();
	}
}


// ============================================================================
// 基本类型读写 - bool
// ============================================================================

TEST(MSExcelTest, WriteAndRead_Bool) {
	TempExcelFile tmp("msexcel_bool.xlsx");
	{
		MSExcel excel(tmp.path, "write");
		excel.AddSheet("Data");
		excel.WCellValue<bool>("Data", true, 1, 1);
		excel.WCellValue<bool>("Data", false, 2, 1);
		excel.Close();
	}
	{
		MSExcel excel(tmp.path, "read");
		bool v1 = excel.RCellValue<bool>("Data", 1, 1);
		bool v2 = excel.RCellValue<bool>("Data", 2, 1);
		EXPECT_TRUE(v1);
		EXPECT_FALSE(v2);
		excel.Close();
	}
}


// ============================================================================
// vector<double> 读写
// ============================================================================

TEST(MSExcelTest, WriteAndRead_VectorDouble) {
	TempExcelFile tmp("msexcel_vecdouble.xlsx");
	std::vector<double> data = {1.1, 2.2, 3.3, 4.4, 5.5};
	{
		MSExcel excel(tmp.path, "write");
		excel.AddSheet("Data");
		// columnwr=true 按列方向写入，与 RCellValue 的读取方向一致
		excel.WCellValue<std::vector<double>>("Data", data, 1, 1, true);
		excel.Close();
	}
	{
		MSExcel excel(tmp.path, "read");
		auto result = excel.RCellValue<std::vector<double>>("Data", 1, 1,
			static_cast<int>(data.size()), 0);
		ASSERT_EQ(result.size(), data.size());
		for (size_t i = 0; i < data.size(); ++i) {
			EXPECT_NEAR(result[i], data[i], 1e-10);
		}
		excel.Close();
	}
}


// ============================================================================
// Eigen::VectorXd 读写
// ============================================================================

TEST(MSExcelTest, WriteAndRead_EigenVectorXd) {
	TempExcelFile tmp("msexcel_eigvec.xlsx");
	Eigen::VectorXd vec(4);
	vec << 10.0, 20.0, 30.0, 40.0;
	{
		MSExcel excel(tmp.path, "write");
		excel.AddSheet("Data");
		// columnwr=true 使 RCellValue 能正确按列读取
		excel.WCellValue<Eigen::VectorXd>("Data", vec, 1, 1, true);
		excel.Close();
	}
	{
		MSExcel excel(tmp.path, "read");
		auto result = excel.RCellValue<Eigen::VectorXd>("Data", 1, 1,
			static_cast<int>(vec.size()), 0);
		ASSERT_EQ(result.size(), vec.size());
		for (int i = 0; i < vec.size(); ++i) {
			EXPECT_NEAR(result(i), vec(i), 1e-10);
		}
		excel.Close();
	}
}


// ============================================================================
// Eigen::MatrixXd 读写
// ============================================================================

TEST(MSExcelTest, WriteAndRead_EigenMatrixXd) {
	TempExcelFile tmp("msexcel_eigmat.xlsx");
	Eigen::MatrixXd mat(2, 3);
	mat << 1.0, 2.0, 3.0,
		   4.0, 5.0, 6.0;
	{
		MSExcel excel(tmp.path, "write");
		excel.AddSheet("Data");
		excel.WCellValue<Eigen::MatrixXd>("Data", mat, 1, 1);
		excel.Close();
	}
	{
		MSExcel excel(tmp.path, "read");
		auto result = excel.RCellValue<Eigen::MatrixXd>("Data", 1, 1,
			static_cast<int>(mat.rows()), static_cast<int>(mat.cols()));
		ASSERT_EQ(result.rows(), mat.rows());
		ASSERT_EQ(result.cols(), mat.cols());
		for (int i = 0; i < mat.rows(); ++i) {
			for (int j = 0; j < mat.cols(); ++j) {
				EXPECT_NEAR(result(i, j), mat(i, j), 1e-10);
			}
		}
		excel.Close();
	}
}


// ============================================================================
// RowCount / ColumnCount
// ============================================================================

TEST(MSExcelTest, RowCount_And_ColumnCount) {
	TempExcelFile tmp("msexcel_count.xlsx");
	{
		MSExcel excel(tmp.path, "write");
		excel.AddSheet("Data");
		// 从 (0,0) 开始写入，对应 cell(1,1)，确保 RowCount/ColumnCount 与写入维度一致
		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 2; ++c) {
				excel.WCellValue<int>("Data", (r + 1) * 10 + (c + 1), r, c);
			}
		}
		excel.Close();
	}
	{
		MSExcel excel(tmp.path, "read");
		EXPECT_EQ(excel.RowCount("Data"), 3);
		EXPECT_EQ(excel.ColumnCount("Data"), 2);
		excel.Close();
	}
}


// ============================================================================
// columnwr 参数测试（按列写入）
// ============================================================================

TEST(MSExcelTest, WCellValue_ColumnWrite) {
	TempExcelFile tmp("msexcel_colwrite.xlsx");
	std::vector<double> data = {10.0, 20.0, 30.0};
	{
		MSExcel excel(tmp.path, "write");
		excel.AddSheet("Data");
		// columnwr=true -> 按列方向写入
		excel.WCellValue<std::vector<double>>("Data", data, 1, 1, true);
		excel.Close();
	}
	{
		MSExcel excel(tmp.path, "read");
		// 验证数据确实写入了列方向
		for (int i = 0; i < static_cast<int>(data.size()); ++i) {
			double v = excel.RCellValue<double>("Data", i + 1, 1);
			EXPECT_NEAR(v, data[i], 1e-10);
		}
		excel.Close();
	}
}
