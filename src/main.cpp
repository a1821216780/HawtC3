
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <clocale>
#include <iostream>
#include <fstream>
#include <string>
#include <csignal>


#include "io/System/Console.h"
#include "io/IO/TimesHelper.h"
#include "IO/Log/LogHelper.h"
#include "IO/IO/StringHelper.h"

#include "PCSL/Pcs_Runner.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
using namespace Qahse::IO::System;
using namespace Qahse::IO::IO;
using namespace Qahse::IO::Log;




//#define EIGEN_USE_BLAS  //使用BLAS加速
//#define EIGEN_USE_CUDA  //使用CUDA加速
int main(int argc, char *argv[])
{
    std::setlocale(LC_ALL, ".UTF-8");
    //初始化Log系统，防止log输出崩溃时无法记录日志
    LogHelper::DisplayInformation();

    if(argc == 1)
    {

        LogHelper::WriteLogO ( "Qahse Command Line Interface (CLI) - Version 1.0" );
        LogHelper::WriteLogO ( "Usage: Qahse [options]" );
        LogHelper::WriteLogO ( "Options:" );
        LogHelper::WriteLogO ( "  --test                 Run QFEM tests and exit" );
        LogHelper::WriteLogO ( "  --linearize <file.lin> Run linearization from .lin file, no GUI" );
        LogHelper::WriteLogO (  "  --qwd <file.qwd>      Run standalone wind mode from .qwd file" );
        LogHelper::WriteLogO (  "  --qod <file.qod>      Run standalone ocean mode from .qod file" );
        LogHelper::WriteLogO (  "  --pcsl <input_file>   Run PCSL cross-section analysis from input file" );
        LogHelper::WriteLogO ( "  --run <file.trb|file.sim> [options]  Run simulation from definition file, no GUI" );

        //对argc, char *argv进行赋值
        Console::Write(" >");
        auto cmds = StringHelper::Split(Console::ReadLine(),' ');
        argc = cmds.size()+1;
        argv = new char*[argc];
        for (size_t i = 1; i < argc; i++)
        {
            argv[i] = new char[cmds[i-1].size() + 1];
            std::strcpy(argv[i], cmds[i-1].c_str());       
        }
        
    }



    // --qwd <file.qwd>
    // 独立风场模式: 仅依据 qwd 生成/导入风场，不依赖风力机模型。
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--qwd") == 0) {
#ifdef _WIN32
            if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
                AllocConsole();
            }
            // FILE* fDummy;
            // freopen_s(&fDummy, "CONOUT$", "w", stdout);
            // freopen_s(&fDummy, "CONOUT$", "w", stderr);
            // freopen_s(&fDummy, "CONIN$",  "r", stdin);
            // std::cout.clear();
            // std::cerr.clear();
#endif
            if (i + 1 >= argc) {
                std::cerr << "Usage: Qahse --qwd <file.qwd>" << std::endl;
                return 1;
            }

            const std::string qwdFile = argv[i + 1];
            std::cout << "=== Qahse Standalone Wind Mode (.qwd) ===" << std::endl;
            std::cout << "Input: " << qwdFile << std::endl;

            //const int result = QahseEngine::RunWindFromQwd(qwdFile);

            std::cout.flush();
            std::cerr.flush();
            fflush(stdout);
            fflush(stderr);
        }
    }


    // --pcsl <input_filepath>
    // 独立截面分析模式: PCSL 截面刚度/质量矩阵计算
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--pcsl") == 0) {
#ifdef _WIN32
            if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
                AllocConsole();
            }
            // FILE* fDummy;
            // freopen_s(&fDummy, "CONOUT$", "w", stdout);
            // freopen_s(&fDummy, "CONOUT$", "w", stderr);
            // freopen_s(&fDummy, "CONIN$",  "r", stdin);
            // std::cout.clear();
            // std::cerr.clear();
#endif
            if (i + 1 >= argc) {
                std::cerr << "Usage: Qahse --pcsl <input_filepath>" << std::endl;
                return 1;
            }
            std::string filepath = argv[i + 1];
#ifdef _WIN32
            // argv uses ACP (e.g. GBK) but setlocale(".UTF-8") makes file APIs expect UTF-8.
            // Convert ACP → wide → UTF-8 using kernel32 APIs (always implicitly linked).
            {
                int wlen = MultiByteToWideChar(CP_ACP, 0, argv[i + 1], -1, nullptr, 0);
                if (wlen > 0) {
                    std::wstring ws(wlen - 1, L'\0');
                    MultiByteToWideChar(CP_ACP, 0, argv[i + 1], -1, &ws[0], wlen);
                    int u8len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
                    if (u8len > 0) {
                        filepath.resize(u8len - 1);
                        WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &filepath[0], u8len, nullptr, nullptr);
                    }
                }
            }
#endif
            int result = PCSL::RunPcslFromFile(filepath);
            std::cout.flush();
            std::cerr.flush();
            fflush(stdout);
            fflush(stderr);
        }
    }

    LogHelper::EndProgram();

   /* QApplication app(argc, argv);
    Qahse window;
    window.show();
    return app.exec();*/
}
