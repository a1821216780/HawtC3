// test_check_error.cpp - CheckError 类的单元测试
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include "../../../src/IO/IO/CheckError.h"

using namespace HawtC3::IO::IO;
namespace fs = std::filesystem;

/// 创建临时文件，测试结束后自动删除
class TempFile {
public:
	explicit TempFile(const std::string& name, const std::string& content = "") {
		path_ = (fs::temp_directory_path() / ("hawtc3_test_" + name)).string();
		std::ofstream f(path_);
		f << content;
		f.close();
	}
	~TempFile() {
		if (fs::exists(path_)) fs::remove(path_);
	}
	const std::string& path() const { return path_; }
private:
	std::string path_;
};

/// 创建临时目录，测试结束后自动删除
class TempDir {
public:
	explicit TempDir(const std::string& name) {
		path_ = (fs::temp_directory_path() / ("hawtc3_test_dir_" + name)).string();
		fs::create_directories(path_);
	}
	~TempDir() {
		if (fs::exists(path_)) fs::remove_all(path_);
	}
	const std::string& path() const { return path_; }
private:
	std::string path_;
};


// ============================================================================
// Filexists (bool version)
// ============================================================================

TEST(CheckErrorTest, Filexists_ExistingFile) {
	TempFile tmp("check_exist.txt", "content");
	EXPECT_TRUE(CheckError::Filexists(tmp.path(), false, false));
}

TEST(CheckErrorTest, Filexists_NonExistingFile) {
	EXPECT_FALSE(CheckError::Filexists("C:\\NonExistent\\file_12345.txt", false, false));
}


// ============================================================================
// CheckPath (extension check)
// ============================================================================

TEST(CheckErrorTest, CheckPath_AddsExtension) {
	std::string path = (fs::temp_directory_path() / "hawtc3_test_noext").string();
	CheckError::CheckPath(path, ".txt", false);
	EXPECT_EQ(path.substr(path.size() - 4), ".txt");
}

TEST(CheckErrorTest, CheckPath_CorrectExtension) {
	std::string path = (fs::temp_directory_path() / "hawtc3_test_file.txt").string();
	std::string original = path;
	CheckError::CheckPath(path, ".txt", false);
	EXPECT_EQ(path, original);
}


// ============================================================================
// CheckDir
// ============================================================================

TEST(CheckErrorTest, CheckDir_CreateMissing) {
	std::string dir = (fs::temp_directory_path() / "hawtc3_test_newdir_ckdir").string();
	if (fs::exists(dir)) fs::remove_all(dir);

	CheckError::CheckDir(dir, true);
	EXPECT_TRUE(fs::exists(dir));

	// 清理
	fs::remove_all(dir);
}

TEST(CheckErrorTest, CheckDir_ExistingDir) {
	TempDir tmp("existing_dir");
	// 不应崩溃
	CheckError::CheckDir(tmp.path(), false);
}


// ============================================================================
// CheckInptr
// ============================================================================

TEST(CheckErrorTest, CheckInptr_NullPointer) {
	// 传入 nullptr，不应崩溃（桩函数只打印错误）
	CheckError::CheckInptr(nullptr, "TestFunc", "test.dll");
}

TEST(CheckErrorTest, CheckInptr_ValidPointer) {
	int dummy = 42;
	// 传入有效指针，不应触发错误
	CheckError::CheckInptr(&dummy, "TestFunc", "test.dll");
}


// ============================================================================
// Checkwavetime
// ============================================================================

TEST(CheckErrorTest, Checkwavetime_EmptyTimespan) {
	std::vector<double> timespan = {};
	double t = 5.0;
	// 空时间跨度应直接返回
	CheckError::Checkwavetime(timespan, t);
	EXPECT_DOUBLE_EQ(t, 5.0);
}

TEST(CheckErrorTest, Checkwavetime_WithinRange) {
	std::vector<double> timespan = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
	double t = 3.0;
	CheckError::Checkwavetime(timespan, t, true);
	// t 应该在范围内，减了微小值
	EXPECT_NEAR(t, 3.0 - 0.0000001, 1e-6);
}

TEST(CheckErrorTest, Checkwavetime_NegativeTime) {
	std::vector<double> timespan = {0.0, 1.0, 2.0};
	double t = -1.0;
	CheckError::Checkwavetime(timespan, t, true);
	// 负数时间应触发警告但不崩溃
	EXPECT_LT(t, 0);
}
