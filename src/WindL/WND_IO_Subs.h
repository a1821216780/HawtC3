#pragma once
//**********************************************************************************************************************************
// LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of Qahse.IO.Math
//
// Licensed under the Boost Software License - Version 1.0 - August 17th, 2003
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.HawtC.cn/licenses.txt
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//**********************************************************************************************************************************

// 该文件的作用主要是读取IO部分的内容，读取和赋值结构体

#include "WND_Types.h"
#include "../IO/IO/CheckError.h"
#include "../IO/IO/InputFileParser.h"
#include "../IO/IO/StringHelper.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Qahse::WindL
{
    using Qahse::IO::IO::CheckError;
    using Qahse::IO::IO::InputFileParser;
    using Qahse::IO::IO::StringHelper;

    inline RunMode ParseRunMode(const std::string &token)
    {
        const std::string t = InputFileParser::ToUpperCopy(token);
        if (t == "1" || t == "IMPORT")
            return RunMode::IMPORT;
        return RunMode::GENERATE;
    }

    inline WindModelType ParseWindModelType(const std::string &token)
    {
        const std::string t = InputFileParser::ToUpperCopy(token);
        if (t == "VONKARMAN" || t == "VON_KARMAN")
            return WindModelType::VONKARMAN;
        if (t == "ETM")
            return WindModelType::ETM;
        if (t == "EWM")
            return WindModelType::EWM;
        if (t == "EOG")
            return WindModelType::EOG;
        if (t == "EDC")
            return WindModelType::EDC;
        if (t == "ECD")
            return WindModelType::ECD;
        if (t == "EWS")
            return WindModelType::EWS;
        if (t == "UNIFORM")
            return WindModelType::UNIFORM;
        return WindModelType::KAIMAL;
    }

    inline IecStandard ParseIecStandard(const std::string &token)
    {
        const std::string t = InputFileParser::ToUpperCopy(token);
        if (t == "ED2")
            return IecStandard::ED2;
        if (t == "ED4")
            return IecStandard::ED4;
        return IecStandard::ED3;
    }

    inline TurbineClass ParseTurbineClass(const std::string &token)
    {
        const std::string t = InputFileParser::ToUpperCopy(token);
        if (t == "III" || t == "CLASS_III")
            return TurbineClass::Class_III;
        if (t == "II" || t == "CLASS_II")
            return TurbineClass::Class_II;
        return TurbineClass::Class_I;
    }

    inline TurbulenceClass ParseTurbulenceClass(const std::string &token)
    {
        const std::string t = InputFileParser::ToUpperCopy(token);
        if (t == "A" || t == "CLASS_A")
            return TurbulenceClass::Class_A;
        if (t == "C" || t == "CLASS_C")
            return TurbulenceClass::Class_C;
        return TurbulenceClass::Class_B;
    }

    inline ShearType ParseShearType(const std::string &token)
    {
        const std::string t = InputFileParser::ToUpperCopy(token);
        if (t == "LOG")
            return ShearType::LOG;
        if (t == "NONE")
            return ShearType::NONE;
        if (t == "TABULAR")
            return ShearType::TABULAR;
        return ShearType::PL;
    }

    inline GenMethod ParseGenMethod(const std::string &token)
    {
        const std::string t = InputFileParser::ToUpperCopy(token);
        if (t == "SRM")
            return GenMethod::SRM;
        if (t == "VEERS")
            return GenMethod::VEERS;
        return GenMethod::AUTO;
    }

    inline InterpMethod ParseInterpMethod(const std::string &token)
    {
        const std::string t = InputFileParser::ToUpperCopy(token);
        if (t == "CUBIC")
            return InterpMethod::CUBIC;
        return InterpMethod::TRILINEAR;
    }

    inline WindLInput ReadWindL_MainFile(const std::string &path)
    {
        CheckError::Filexists(path);

        const std::vector<std::string> lines = StringHelper::ReadAllLines(path);

        WindLInput in;
        


        in.mode = ParseRunMode(InputFileParser::ReadKeywordToken(lines, "MODE", path, "GENERATE", false));
        in.windModel = ParseWindModelType(InputFileParser::ReadKeywordToken(lines, "WINDMODEL", path, "KAIMAL", false));

        in.iecStandard = ParseIecStandard(InputFileParser::ReadKeywordToken(lines, "IECSTANDARD", path, "ED3", false));
        in.turbineClass = ParseTurbineClass(InputFileParser::ReadKeywordToken(lines, "TURBINECLASS", path, "I", false));
        in.turbClass = ParseTurbulenceClass(InputFileParser::ReadKeywordToken(lines, "TURBCLASS", path, "B", false));
        in.vRef = InputFileParser::ReadKeywordDouble(lines, "VREF", path, in.vRef, false);
        in.rotorDiameter = InputFileParser::ReadKeywordDouble(lines, "ROTORDIAMETER", path, in.rotorDiameter, false);

        in.meanWindSpeed = InputFileParser::ReadKeywordDouble(lines, "MEANWINDSPEED", path, in.meanWindSpeed, false);
        in.refHeight = InputFileParser::ReadKeywordDouble(lines, "REFHEIGHT", path, in.refHeight, false);
        in.shearType = ParseShearType(InputFileParser::ReadKeywordToken(lines, "SHEARTYPE", path, "PL", false));
        in.shearExp = InputFileParser::ReadKeywordDouble(lines, "SHEAREXP", path, in.shearExp, false);
        in.roughness = InputFileParser::ReadKeywordDouble(lines, "ROUGHNESS", path, in.roughness, false);
        in.horAngle = InputFileParser::ReadKeywordDouble(lines, "HORANGLE", path, in.horAngle, false);
        in.vertAngle = InputFileParser::ReadKeywordDouble(lines, "VERTANGLE", path, in.vertAngle, false);

        in.turbIntensity = InputFileParser::ReadKeywordDouble(lines, "TURBINTENSITY", path, in.turbIntensity, false);
        in.turbSeed = InputFileParser::ReadKeywordInt(lines, "TURBSEED", path, in.turbSeed, false);
        in.gridPtsY = InputFileParser::ReadKeywordInt(lines, "GRIDPTSY", path, in.gridPtsY, false);
        in.gridPtsZ = InputFileParser::ReadKeywordInt(lines, "GRIDPTSZ", path, in.gridPtsZ, false);
        in.fieldDimY = InputFileParser::ReadKeywordDouble(lines, "FIELDDIMY", path, in.fieldDimY, false);
        in.fieldDimZ = InputFileParser::ReadKeywordDouble(lines, "FIELDDIMZ", path, in.fieldDimZ, false);
        in.simTime = InputFileParser::ReadKeywordDouble(lines, "SIMTIME", path, in.simTime, false);
        in.timeStep = InputFileParser::ReadKeywordDouble(lines, "TIMESTEP", path, in.timeStep, false);
        in.cycleWind = InputFileParser::ReadKeywordBool(lines, "CYCLEWIND", path, in.cycleWind, false);
        in.threeComp = InputFileParser::ReadKeywordBool(lines, "THREECOMP", path, in.threeComp, false);

        in.gustStartTime = InputFileParser::ReadKeywordDouble(lines, "GUSTSTARTTIME", path, in.gustStartTime, false);
        in.gustDuration = InputFileParser::ReadKeywordDouble(lines, "GUSTDURATION", path, in.gustDuration, false);
        in.windVeer = InputFileParser::ReadKeywordDouble(lines, "WINDVEER", path, in.windVeer, false);

        in.genMethod = ParseGenMethod(InputFileParser::ReadKeywordToken(lines, "GENMETHOD", path, "AUTO", false));
        in.useFft = InputFileParser::ReadKeywordBool(lines, "USEFFT", path, in.useFft, false);
        in.interpMethod = ParseInterpMethod(InputFileParser::ReadKeywordToken(lines, "INTERPMETHOD", path, "TRILINEAR", false));

        const std::string savePathToken = InputFileParser::ReadKeywordToken(lines, "SAVEPATH", path, in.savePath, false);
        if (!savePathToken.empty())
        {
            in.savePath = InputFileParser::ResolvePath(path, savePathToken);
            std::filesystem::create_directories(in.savePath);
        }
        in.sumPrint = InputFileParser::ReadKeywordBool(lines, "SUMPRINT", path, in.sumPrint, false);

        return in;
    }

} // namespace Qahse::WindL
