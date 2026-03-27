#include <gtest/gtest.h>


#include <string>
#include <csignal>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define _DEBUG

int main(int argc, char** argv) {

#ifdef _WIN32
    // Force UTF-8 console code page so UTF-8 literals print correctly on Windows.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::setlocale(LC_ALL, ".UTF-8");

	
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
