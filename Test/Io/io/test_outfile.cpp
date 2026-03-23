// test_outfile.cpp - OutFile 类的单元测试
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <Eigen/Dense>
#include "../../../src/IO/IO/Outfile.h"
#include "../../../src/IO/Log/LogHelper.h"

using namespace HawtC3::IO::IO;
using namespace HawtC3::IO::Log;
namespace fs = std::filesystem;

/// 临时输出文件 RAII 清理
class TempOutPath {
public:
	explicit TempOutPath(const std::string& name) {
		path_ = (fs::temp_directory_path() / ("hawtc3_test_" + name)).string();
	}
	~TempOutPath() {
		if (fs::exists(path_)) fs::remove(path_);
	}
	const std::string& path() const { return path_; }
private:
	std::string path_;
};

/// 读取文件全部内容
static std::string ReadAll(const std::string& path) {
	std::ifstream f(path);
	return std::string((std::istreambuf_iterator<char>(f)),
		std::istreambuf_iterator<char>());
}

/// 测试夹具：每个测试前清空 OutFilelist 以防悬空指针
class OutFileTest : public ::testing::Test {
protected:
	void SetUp() override {
		LogData::OutFilelist.clear();
	}
	void TearDown() override {
		LogData::OutFilelist.clear();
	}
};


// ============================================================================
// 构造和基本写入
// ============================================================================

TEST_F(OutFileTest, Constructor_Creates) {
	TempOutPath tmp("outfile_ctor.3zout");
	{
		OutFile out(tmp.path());
		out.WriteLine("hello");
		out.Outfinish(false);
	}
	EXPECT_TRUE(fs::exists(tmp.path()));
	std::string content = ReadAll(tmp.path());
	EXPECT_NE(content.find("hello"), std::string::npos);
}

TEST_F(OutFileTest, WriteString) {
	TempOutPath tmp("outfile_wstr.3zout");
	{
		OutFile out(tmp.path());
		out.Write("abc");
		out.Write("def");
		out.Outfinish(false);
	}
	std::string content = ReadAll(tmp.path());
	EXPECT_NE(content.find("abc"), std::string::npos);
	EXPECT_NE(content.find("def"), std::string::npos);
}

TEST_F(OutFileTest, WriteDouble) {
	TempOutPath tmp("outfile_wdbl.3zout");
	{
		OutFile out(tmp.path(), 2);
		out.Write(3.14);
		out.Outfinish(false);
	}
	std::string content = ReadAll(tmp.path());
	EXPECT_NE(content.find("3.14"), std::string::npos);
}

TEST_F(OutFileTest, WriteLineDouble) {
	TempOutPath tmp("outfile_wld.3zout");
	{
		OutFile out(tmp.path(), 2);
		out.WriteLine(1.23);
		out.WriteLine(4.56);
		out.Outfinish(false);
	}
	std::string content = ReadAll(tmp.path());
	EXPECT_NE(content.find("1.23"), std::string::npos);
	EXPECT_NE(content.find("4.56"), std::string::npos);
}


// ============================================================================
// Vector / Matrix 写入
// ============================================================================

TEST_F(OutFileTest, WriteEigenVector) {
	TempOutPath tmp("outfile_vec.3zout");
	{
		OutFile out(tmp.path(), 2);
		Eigen::VectorXd v(3);
		v << 1.0, 2.0, 3.0;
		out.Write(v);
		out.Outfinish(false);
	}
	std::string content = ReadAll(tmp.path());
	EXPECT_NE(content.find("1."), std::string::npos);
	EXPECT_NE(content.find("2."), std::string::npos);
	EXPECT_NE(content.find("3."), std::string::npos);
}

TEST_F(OutFileTest, WriteEigenMatrix) {
	TempOutPath tmp("outfile_mat.3zout");
	{
		OutFile out(tmp.path(), 2);
		Eigen::MatrixXd m(2, 2);
		m << 1.0, 2.0,
			 3.0, 4.0;
		out.Write(m);
		out.Outfinish(false);
	}
	EXPECT_TRUE(fs::exists(tmp.path()));
	std::string content = ReadAll(tmp.path());
	EXPECT_NE(content.find("1."), std::string::npos);
	EXPECT_NE(content.find("4."), std::string::npos);
}


// ============================================================================
// GetFilename
// ============================================================================

TEST_F(OutFileTest, GetFilename) {
	TempOutPath tmp("outfile_name.3zout");
	{
		OutFile out(tmp.path());
		std::string name = out.GetFilename();
		EXPECT_FALSE(name.empty());
		out.Outfinish(false);
	}
}


// ============================================================================
// Outfinish remove
// ============================================================================

TEST_F(OutFileTest, Outfinish_Remove) {
	std::string path = (fs::temp_directory_path() / "hawtc3_test_outfinish_rm.3zout").string();
	{
		OutFile out(path);
		out.WriteLine("test");
		// Outfinish(true) 从 OutFilelist 中移除当前文件流
		EXPECT_FALSE(LogData::OutFilelist.empty());
		out.Outfinish(true);
		EXPECT_TRUE(LogData::OutFilelist.empty());
	}
	// 手动清理磁盘文件
	if (fs::exists(path)) fs::remove(path);
}
