#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "../../src/WindL/WND_IO_Subs.h"

using namespace Qahse::WindL;
namespace fs = std::filesystem;

class TempQwdFile
{
public:
    explicit TempQwdFile(const std::string &content)
    {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        dirPath_ = fs::temp_directory_path() / ("Qahse_windl_test_" + std::to_string(stamp));
        fs::create_directories(dirPath_);

        filePath_ = dirPath_ / "input.qwd";
        std::ofstream ofs(filePath_.string(), std::ios::out | std::ios::trunc);
        ofs << content;
    }

    ~TempQwdFile()
    {
        std::error_code ec;
        fs::remove_all(dirPath_, ec);
    }

    const fs::path &filePath() const { return filePath_; }
    const fs::path &dirPath() const { return dirPath_; }

private:
    fs::path dirPath_;
    fs::path filePath_;
};

TEST(WindLIOTest, ReadWindLMainFile_ParseCompleteConfig)
{
    const std::string qwd =
        "GENERATE MODE\n"
        "KAIMAL WINDMODEL\n"
        "ED3 IECSTANDARD\n"
        "I TURBINECLASS\n"
        "B TURBCLASS\n"
        "50.0 VREF\n"
        "126.0 ROTORDIAMETER\n"
        "11.4 MEANWINDSPEED\n"
        "87.6 REFHEIGHT\n"
        "PL SHEARTYPE\n"
        "0.14 SHEAREXP\n"
        "0.01 ROUGHNESS\n"
        "0.0 HORANGLE\n"
        "0.0 VERTANGLE\n"
        "0 TURBINTENSITY\n"
        "1 TURBSEED\n"
        "36 GRIDPTSY\n"
        "36 GRIDPTSZ\n"
        "150.0 FIELDDIMY\n"
        "150.0 FIELDDIMZ\n"
        "660.0 SIMTIME\n"
        "0.05 TIMESTEP\n"
        "true CYCLEWIND\n"
        "true THREECOMP\n"
        "5.0 GUSTSTARTTIME\n"
        "20.0 GUSTDURATION\n"
        "0.15 WINDVEER\n"
        "AUTO GENMETHOD\n"
        "true USEFFT\n"
        "TRILINEAR INTERPMETHOD\n"
        "./result SAVEPATH\n"
        "true SUMPRINT\n";

    TempQwdFile tmp(qwd);
    const fs::path expectedSavePath = fs::absolute(tmp.dirPath() / "result").lexically_normal();

    const WindLInput in = ReadWindL_MainFile(tmp.filePath().string());

    EXPECT_EQ(in.mode, RunMode::GENERATE);
    EXPECT_EQ(in.windModel, WindModelType::KAIMAL);
    EXPECT_EQ(in.iecStandard, IecStandard::ED3);
    EXPECT_EQ(in.turbineClass, TurbineClass::Class_I);
    EXPECT_EQ(in.turbClass, TurbulenceClass::Class_B);

    EXPECT_NEAR(in.vRef, 50.0, 1e-12);
    EXPECT_NEAR(in.rotorDiameter, 126.0, 1e-12);
    EXPECT_NEAR(in.meanWindSpeed, 11.4, 1e-12);
    EXPECT_NEAR(in.refHeight, 87.6, 1e-12);
    EXPECT_EQ(in.shearType, ShearType::PL);
    EXPECT_NEAR(in.shearExp, 0.14, 1e-12);
    EXPECT_NEAR(in.roughness, 0.01, 1e-12);

    EXPECT_EQ(in.gridPtsY, 36);
    EXPECT_EQ(in.gridPtsZ, 36);
    EXPECT_NEAR(in.simTime, 660.0, 1e-12);
    EXPECT_NEAR(in.timeStep, 0.05, 1e-12);
    EXPECT_TRUE(in.cycleWind);
    EXPECT_TRUE(in.threeComp);

    EXPECT_NEAR(in.gustStartTime, 5.0, 1e-12);
    EXPECT_NEAR(in.gustDuration, 20.0, 1e-12);
    EXPECT_NEAR(in.windVeer, 0.15, 1e-12);

    EXPECT_EQ(in.genMethod, GenMethod::AUTO);
    EXPECT_TRUE(in.useFft);
    EXPECT_EQ(in.interpMethod, InterpMethod::TRILINEAR);
    EXPECT_TRUE(in.sumPrint);

    EXPECT_EQ(fs::path(in.savePath).lexically_normal(), expectedSavePath);
    EXPECT_TRUE(fs::exists(in.savePath));
}

TEST(WindLIOTest, ReadWindLMainFile_MissingKeysKeepDefaults)
{
    const std::string qwd =
        "GENERATE MODE\n"
        "KAIMAL WINDMODEL\n"
        "./out SAVEPATH\n";

    TempQwdFile tmp(qwd);
    const WindLInput in = ReadWindL_MainFile(tmp.filePath().string());

    EXPECT_EQ(in.mode, RunMode::GENERATE);
    EXPECT_EQ(in.windModel, WindModelType::KAIMAL);

    EXPECT_EQ(in.iecStandard, IecStandard::ED3);
    EXPECT_EQ(in.turbineClass, TurbineClass::Class_I);
    EXPECT_EQ(in.turbClass, TurbulenceClass::Class_B);
    EXPECT_NEAR(in.vRef, 50.0, 1e-12);
    EXPECT_NEAR(in.rotorDiameter, 126.0, 1e-12);

    EXPECT_NEAR(in.meanWindSpeed, 11.4, 1e-12);
    EXPECT_NEAR(in.refHeight, 87.6, 1e-12);
    EXPECT_EQ(in.shearType, ShearType::PL);
    EXPECT_NEAR(in.shearExp, 0.14, 1e-12);
    EXPECT_NEAR(in.roughness, 0.01, 1e-12);

    EXPECT_EQ(in.turbSeed, 1);
    EXPECT_EQ(in.gridPtsY, 36);
    EXPECT_EQ(in.gridPtsZ, 36);
    EXPECT_TRUE(in.cycleWind);
    EXPECT_TRUE(in.threeComp);

    EXPECT_EQ(in.genMethod, GenMethod::AUTO);
    EXPECT_TRUE(in.useFft);
    EXPECT_EQ(in.interpMethod, InterpMethod::TRILINEAR);
    EXPECT_TRUE(in.sumPrint);

    EXPECT_TRUE(fs::path(in.savePath).is_absolute());
    EXPECT_TRUE(fs::exists(in.savePath));
}

TEST(WindLIOTest, ReadWindLMainFile_ParseAliasesAndNumericFlags)
{
    const std::string qwd =
        "1 MODE\n"
        "von_karman WINDMODEL\n"
        "ED4 IECSTANDARD\n"
        "III TURBINECLASS\n"
        "A TURBCLASS\n"
        "LOG SHEARTYPE\n"
        "VEERS GENMETHOD\n"
        "CUBIC INTERPMETHOD\n"
        "false USEFFT\n"
        "1 CYCLEWIND\n"
        "0 THREECOMP\n"
        "false SUMPRINT\n"
        "./alias_out SAVEPATH\n";

    TempQwdFile tmp(qwd);
    const fs::path expectedSavePath = fs::absolute(tmp.dirPath() / "alias_out").lexically_normal();

    const WindLInput in = ReadWindL_MainFile(tmp.filePath().string());

    EXPECT_EQ(in.mode, RunMode::IMPORT);
    EXPECT_EQ(in.windModel, WindModelType::VONKARMAN);
    EXPECT_EQ(in.iecStandard, IecStandard::ED4);
    EXPECT_EQ(in.turbineClass, TurbineClass::Class_III);
    EXPECT_EQ(in.turbClass, TurbulenceClass::Class_A);
    EXPECT_EQ(in.shearType, ShearType::LOG);
    EXPECT_EQ(in.genMethod, GenMethod::VEERS);
    EXPECT_EQ(in.interpMethod, InterpMethod::CUBIC);

    EXPECT_FALSE(in.useFft);
    EXPECT_TRUE(in.cycleWind);
    EXPECT_FALSE(in.threeComp);
    EXPECT_FALSE(in.sumPrint);

    EXPECT_EQ(fs::path(in.savePath).lexically_normal(), expectedSavePath);
    EXPECT_TRUE(fs::exists(in.savePath));
}

TEST(WindLIOTest, ReadWindLMainFile_InvalidEnumTokensFallbackToDefaults)
{
    const std::string qwd =
        "INVALID MODE\n"
        "UNKNOWN_MODEL WINDMODEL\n"
        "EDX IECSTANDARD\n"
        "CLASS_X TURBINECLASS\n"
        "Z TURBCLASS\n"
        "SHEAR_X SHEARTYPE\n"
        "GEN_X GENMETHOD\n"
        "INTERP_X INTERPMETHOD\n"
        "./fallback_out SAVEPATH\n";

    TempQwdFile tmp(qwd);
    const WindLInput in = ReadWindL_MainFile(tmp.filePath().string());

    EXPECT_EQ(in.mode, RunMode::GENERATE);
    EXPECT_EQ(in.windModel, WindModelType::KAIMAL);
    EXPECT_EQ(in.iecStandard, IecStandard::ED3);
    EXPECT_EQ(in.turbineClass, TurbineClass::Class_I);
    EXPECT_EQ(in.turbClass, TurbulenceClass::Class_B);
    EXPECT_EQ(in.shearType, ShearType::PL);
    EXPECT_EQ(in.genMethod, GenMethod::AUTO);
    EXPECT_EQ(in.interpMethod, InterpMethod::TRILINEAR);
}

TEST(WindLIOTest, ReadWindLMainFile_EmptyFileKeepsDefaults)
{
    TempQwdFile tmp("");
    const WindLInput in = ReadWindL_MainFile(tmp.filePath().string());

    EXPECT_EQ(in.mode, RunMode::GENERATE);
    EXPECT_EQ(in.windModel, WindModelType::KAIMAL);
    EXPECT_EQ(in.iecStandard, IecStandard::ED3);
    EXPECT_EQ(in.turbineClass, TurbineClass::Class_I);
    EXPECT_EQ(in.turbClass, TurbulenceClass::Class_B);
    EXPECT_NEAR(in.vRef, 50.0, 1e-12);
    EXPECT_NEAR(in.rotorDiameter, 126.0, 1e-12);
    EXPECT_TRUE(fs::path(in.savePath).is_absolute());
    EXPECT_TRUE(fs::exists(in.savePath));
}

TEST(WindLIOTest, ReadWindLMainFile_IgnoreCommentLinesWithKeywords)
{
    const std::string qwd =
        "-- fake line 999.0 VREF\n"
        "# fake line false SUMPRINT\n"
        "// fake line IMPORT MODE\n"
        "GENERATE MODE\n"
        "42.5 VREF\n"
        "true SUMPRINT\n"
        "./comment_out SAVEPATH\n";

    TempQwdFile tmp(qwd);
    const WindLInput in = ReadWindL_MainFile(tmp.filePath().string());

    EXPECT_EQ(in.mode, RunMode::GENERATE);
    EXPECT_NEAR(in.vRef, 42.5, 1e-12);
    EXPECT_TRUE(in.sumPrint);
    EXPECT_TRUE(fs::exists(in.savePath));
}
