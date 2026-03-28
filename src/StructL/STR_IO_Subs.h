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

#include "Str_Types.h"
#include "../IO/IO/CheckError.h"
#include "../IO/IO/StringHelper.h"
#include "../IO/IO/FileParser.h"

namespace Qahse::StrL
{
    using Qahse::IO::IO::CheckError;
    using Qahse::IO::IO::StringHelper;

    using namespace std;

    class STR_IO_Subs
    {

        static StrLInput ReadStrL_MainFile(std::string filePath)
        {
            // #先检查文件是否存在
            CheckError::CheckPath(filePath, ".str", false);

            // 读取文件字符串内容
            vector<string> lines = StringHelper::ReadAllLines(filePath);

            // 创建输入结构体实例并返回
            StrLInput input;

            input.Filepath = filePath;
            input.Lines = lines;

            // 返回输入结构体实例
            return input;
        }
    };
}