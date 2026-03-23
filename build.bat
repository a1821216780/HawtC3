@echo off
cd /d E:\HawtC3
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1

:: Compile Windows resource (version info + icon)
rc.exe /nologo /fo build\HawtC3.res src\HawtC3.rc
if errorlevel 1 (
    echo [ERROR] Resource compilation failed.
    exit /b 1
)

cl.exe /EHsc /std:c++17 /Zc:__cplusplus /O2 /Ob2 /Ot /Oi /GL /Gy /MP /W3 /sdl /permissive- /openmp /arch:AVX /FS /utf-8 /bigobj /Zm999 /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /DEIGEN_USE_MKL_ALL /nologo ^
  /I "E:\Qahse\Qahse" ^
  /I "E:\Qahse\Qahse\Qahse\Core" ^
  /I "D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\include" ^
  /I "E:\Qahse\Qahse\include_headers\Eigen3" ^
  /I "E:\Qahse\Qahse\include_headers\Chrono" ^
  /I "E:\Qahse\Qahse\include_headers" ^
  /I "E:\Qahse\Qahse\include_headers\OpenBlas\include" ^
  /I "E:\Qahse\Qahse\src" ^
  /I "E:\Qahse\Qahse\Project\OpenXLSX-master\OpenXLSX" ^
  /I "E:\Qahse\Qahse\Project\OpenXLSX-master\build\OpenXLSX" ^
  /I "E:\Qahse\Qahse\Project\OpenXLSX-master\OpenXLSX\headers" ^
  /I "E:\Qahse\Qahse\Project\spectra-master\include" ^
  /I "C:\Program Files (x86)\Intel\oneAPI\mkl\latest\include" ^
  /Fe"build\HawtC3.exe" /Fobuild\ /Fdbuild\ ^
  @sources.rsp ^
  /link /SUBSYSTEM:CONSOLE /OPT:REF /OPT:ICF /LTCG /DEBUG:NONE ^
  /LIBPATH:"E:\Qahse\Qahse\include_headers\OpenBlas\lib" ^
  /LIBPATH:"C:\Program Files (x86)\Intel\oneAPI\mkl\2025.3\lib" ^
  /LIBPATH:"C:\Program Files (x86)\Intel\oneAPI\compiler\2025.3\lib" ^
  /LIBPATH:"E:\Qahse\Qahse\Project\chrono-9.0.1\build\lib\Release" ^
  mkl_intel_lp64_dll.lib mkl_intel_thread_dll.lib mkl_core_dll.lib libiomp5md.lib ChronoEngine.lib build\HawtC3.res
