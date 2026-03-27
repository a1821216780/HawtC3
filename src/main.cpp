
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


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
using namespace Qahse::IO::System;
using namespace Qahse::IO::IO;
using namespace Qahse::IO::Log;


// #define  EIGEN_USE_MKL_ALL// 已移至项目预处理器定义 (vcxproj)

// Forward-declare CLI runner (defined in ImportExport.cpp, namespace Qahse)
namespace QahseEngine {
    int RunSimulationFromTrb(const std::string& trbFileName,
                             double windSpeed, int numSteps, double dt, double rpm,
                             double turbulenceIntensity = 0.0, int seed = 1,
                             int gridPts = 8,
                             const std::string& mode = "prescribed",
                             double waveHs = 0.0, double waveTp = 0.0,
                             double waveDir = 0.0,
                             double initPitchDeg = 0.0,
                             int waveStretch = 0,
                             const std::string& qwdFile = "",
                             bool enableVTK = false,
                             double dtVTK = 0.5,
                             int vtkType = 2);
    int RunSimulationFromSim(const std::string& simFileName);
    int RunWindFromQwd(const std::string& qwdFileName);
    int RunOceanFromQod(const std::string& qodFileName);
}

// Forward-declare linearization entry (defined in Linearization.cpp)
namespace QahseEngine {
    int RunLinearizationFromFile(const std::string& linFilePath);
}

// PCSL cross-section analysis (header-only)
#include "PCSL/PcslRunner.h"

int RunQFEMTestsOnDemand();

// Global crash log helper
static void writeCrashLog(const char* msg) {
    std::ofstream cl("E:\\Qahse\\Qahse\\crash_log.txt", std::ios::app);
    cl << msg << std::endl;
    cl.flush();
    std::cerr << msg << std::endl;
    std::cerr.flush();
}

#ifdef _WIN32
// Windows Structured Exception filter
static LONG WINAPI CrashFilter(EXCEPTION_POINTERS* ep) {
    char buf[256];
    snprintf(buf, sizeof(buf), "[CRASH] Windows exception code=0x%08X at address=%p",
             ep->ExceptionRecord->ExceptionCode,
             ep->ExceptionRecord->ExceptionAddress);
    writeCrashLog(buf);
    std::cout.flush();
    std::cerr.flush();
    fflush(stdout);
    fflush(stderr);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

// Signal handler for SIGSEGV etc
static void signalHandler(int sig) {
    char buf[128];
    snprintf(buf, sizeof(buf), "[CRASH] Signal %d received", sig);
    writeCrashLog(buf);
    std::cout.flush();
    std::cerr.flush();
    fflush(stdout);
    fflush(stderr);
    _Exit(99);
}

#ifdef _WIN32
// Note: SetUnhandledExceptionFilter (installed in main) catches SEH exceptions.
// No need for __try/__except here.
#endif

//#define EIGEN_USE_BLAS  //使用BLAS加速
//#define EIGEN_USE_CUDA  //使用CUDA加速
int main(int argc, char *argv[])
{


#ifdef _WIN32
    // Force UTF-8 console code page so UTF-8 literals print correctly on Windows.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    SetUnhandledExceptionFilter(CrashFilter);
#endif
    std::setlocale(LC_ALL, ".UTF-8");
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGABRT, signalHandler);
    std::signal(SIGFPE, signalHandler);

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


    // If --test is passed, run QFEM tests and exit without starting the GUI.
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--test") == 0) {
            int testResult = RunQFEMTestsOnDemand();
            std::cout.flush();
            std::cerr.flush();
            fflush(stdout);
            fflush(stderr);
            _Exit(testResult == 0 ? 0 : 1);
        }
    }

    // --linearize <file.lin>
    // CLI linearization mode: run linearization from .lin file, no GUI.
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--linearize") == 0) {
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
                std::cerr << "Usage: Qahse --linearize <file.lin>" << std::endl;
                return 1;
            }
            std::string linFile = argv[i + 1];
            std::cout << "=== Qahse Linearization ===" << std::endl;
            std::cout << "Input: " << linFile << std::endl;
            int result = QahseEngine::RunLinearizationFromFile(linFile);
            std::cout.flush();
            std::cerr.flush();
            fflush(stdout);
            fflush(stderr);
            _Exit(result);
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

            const int result = QahseEngine::RunWindFromQwd(qwdFile);

            std::cout.flush();
            std::cerr.flush();
            fflush(stdout);
            fflush(stderr);
            _Exit(result);
        }
    }

    // --qod <file.qod>
    // 独立波浪场模式: 仅依据 qod 生成/导入波浪场，不依赖风力机模型。
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--qod") == 0) {
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
                std::cerr << "Usage: Qahse --qod <file.qod>" << std::endl;
                return 1;
            }

            const std::string qodFile = argv[i + 1];
            std::cout << "=== Qahse Standalone Ocean Mode (.qod) ===" << std::endl;
            std::cout << "Input: " << qodFile << std::endl;

            const int result = QahseEngine::RunOceanFromQod(qodFile);

            std::cout.flush();
            std::cerr.flush();
            fflush(stdout);
            fflush(stderr);
            _Exit(result);
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

    // --run <file.trb|file.sim> [options]
    // CLI simulation mode: run simulation from definition file, no GUI.
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--run") == 0) {
#ifdef _WIN32
            // Windows subsystem app: attach to parent console or allocate one
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
                std::cerr << "Usage: Qahse --run <file.trb|file.sim> [--wind <m/s>] [--steps <N>] [--dt <s>] [--rpm <rpm>]"
                          << " [--ti <%>] [--seed <N>] [--gridpts <N>] [--mode prescribed|torque|freewheel]"
                          << " [--qwd <file.qwd>]"
                          << " [--hs <m>] [--tp <s>] [--wavedir <deg>] [--wavestretch <0-3>]" << std::endl;
                return 1;
            }
            std::string inputFile = argv[i + 1];

            // Parse optional parameters
            double windSpeed = 10.0;
            int    numSteps  = 100;
            double dt        = 0.05;
            double rpm       = -1.0;
            double ti        = 0.0;   // Turbulence intensity [%], 0 = uniform wind
            int    seed      = 1;     // Random seed for wind field
            int    gridPts   = 8;     // Grid points per side (Y and Z)
            std::string mode = "prescribed"; // prescribed | torque | freewheel
            double waveHs    = 0.0;   // Significant wave height [m], 0 = no waves
            double waveTp    = 0.0;   // Peak wave period [s]
            double waveDir   = 0.0;   // Wave propagation direction [deg]
            int    waveStretch = 0;   // Wave stretching: 0=None, 1=Vertical, 2=Wheeler, 3=Extrapolation
            double initPitch = 0.0;   // Initial collective pitch [deg]
            std::string qwdFile;      // .qwd 风场定义文件路径
            bool   enableVTK = false;
            double dtVTK     = 0.5;
            int    vtkType   = 2;     // 1=Surface, 2=Wireframe

            for (int j = i + 2; j < argc - 1; j++) {
                if (std::strcmp(argv[j], "--wind") == 0) {
                    windSpeed = std::atof(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--steps") == 0) {
                    numSteps = std::atoi(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--dt") == 0) {
                    dt = std::atof(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--rpm") == 0) {
                    rpm = std::atof(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--ti") == 0) {
                    ti = std::atof(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--seed") == 0) {
                    seed = std::atoi(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--gridpts") == 0) {
                    gridPts = std::atoi(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--mode") == 0) {
                    mode = argv[j + 1];
                    j++;
                } else if (std::strcmp(argv[j], "--hs") == 0) {
                    waveHs = std::atof(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--tp") == 0) {
                    waveTp = std::atof(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--wavedir") == 0) {
                    waveDir = std::atof(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--wavestretch") == 0) {
                    waveStretch = std::atoi(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--pitch") == 0) {
                    initPitch = std::atof(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--qwd") == 0) {
                    qwdFile = argv[j + 1];
                    j++;
                } else if (std::strcmp(argv[j], "--vtk") == 0) {
                    enableVTK = true;
                } else if (std::strcmp(argv[j], "--dtvtk") == 0) {
                    dtVTK = std::atof(argv[j + 1]);
                    j++;
                } else if (std::strcmp(argv[j], "--vtktype") == 0) {
                    vtkType = std::atoi(argv[j + 1]);
                    j++;
                }
            }

            int result = 0;
            // Determine file type by extension
            std::string ext;
            {
                size_t dot = inputFile.rfind('.');
                if (dot != std::string::npos)
                    ext = inputFile.substr(dot);
            }

            if (ext == ".sim") {
                result = QahseEngine::RunSimulationFromSim(inputFile);
            } else {
                // Default: treat as .trb
                {
                    std::ofstream ml("E:\\Qahse\\Qahse\\main_debug.log", std::ios::app);
                    ml << "About to call RunSimulationFromTrb..." << std::endl;
                    ml.flush();
                }
                result = QahseEngine::RunSimulationFromTrb(inputFile, windSpeed, numSteps, dt, rpm, ti, seed, gridPts, mode, waveHs, waveTp, waveDir, initPitch, waveStretch, qwdFile, enableVTK, dtVTK, vtkType);
                {
                    std::ofstream ml("E:\\Qahse\\Qahse\\main_debug.log", std::ios::app);
                    ml << "RunSimulationFromTrb returned: " << result << std::endl;
                    ml.flush();
                }
            }

            std::cout.flush();
            std::cerr.flush();
            fflush(stdout);
            fflush(stderr);
            _Exit(result);
        }
    }

    LogHelper::EndProgram();

   /* QApplication app(argc, argv);
    Qahse window;
    window.show();
    return app.exec();*/
}
