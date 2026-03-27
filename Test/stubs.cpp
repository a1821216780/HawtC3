// stubs.cpp - 为单元测试提供最小化的依赖桩函数
// 替代 LogHelper.cpp 及其复杂依赖链

#include "../src/IO/Log/LogHelper.h"
#include <iostream>
#include <gtest/gtest.h>

// gtest header/lib 版本不一致时 PrintStringTo 符号缺失的 workaround
namespace testing {
namespace internal {
	void PrintStringTo(const std::string& s, std::ostream* os) {
		*os << "\"" << s << "\"";
	}
}
}

namespace Qahse {
namespace IO {
namespace Log {

	// 静态成员定义
	int LogHelper::mode = 0;
	bool LogHelper::firstshowinformation = false;
	std::shared_ptr<Email> Datastore::EmailSet = nullptr;

	void LogData::Add(const std::string& message) {
		LogData::log_inf.push_back(message);
	}

	void LogData::Initialize() {
		LogData::log_inf.clear();
	}

	void LogHelper::ErrorLog(const std::string& message, const std::string& relmessage,
		const std::string& title, int outtime, const std::string& FunctionName) {
		// 测试环境中仅输出到 stderr，不抛异常也不退出
		std::cerr << "[TEST-ErrorLog] " << FunctionName << ": " << message << std::endl;
	}

	void LogHelper::WarnLog(const std::string& debugmessage, const std::string& releasemessage,
		ConsoleColor color, int outtime, const std::string& functionname) {
		std::cerr << "[TEST-WarnLog] " << functionname << ": " << debugmessage << std::endl;
	}

	void LogHelper::WriteLog(const std::string& message, const std::string& releasemes,
		bool show_title, const std::string& title, ConsoleColor color,
		bool newline, int leval) {
		// no-op in test
	}

	void LogHelper::EndProgram(bool forceTerminate, const std::string& outstring,
		bool isdebug, bool quitimmediately, int sleep_time) {
		// no-op in test - 不退出进程
	}

	void LogHelper::UnhandledExceptionHandler(int signal) {}
	void LogHelper::DisinfV1(bool lics) {}
	void LogHelper::DisinfV2(bool lics) {}
	void LogHelper::DisplayInformation(bool UnhandledException, bool lic) {}
	void LogHelper::EndInformation(double tfinal, double dt,
		const std::chrono::steady_clock::time_point& start_time) {}

	void LogHelper::WriteLogO(const std::string& message, bool show_log,
		const std::string& title) {
		// no-op in test
	}

} // namespace Log
} // namespace IO
} // namespace Qahse
