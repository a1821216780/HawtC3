
//**********************************************************************************************************************************
//LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,赵子祯
//
//    This file is part of HawtC3.IO.Math
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

#pragma once
#ifndef PhysicalParameters_H
#define PhysicalParameters_H

#include <complex>
#include <iostream>
#include <limits>
#include <cmath>



namespace HawtC3::IO::IO
{

	class PhysicalParameters
	{
	public:

		inline static   double Max(double a, double b)
		{
			return a > b ? a : b;
		}
		inline static   double Min(double a, double b)
		{
			return a > b ? b : a;
		}
		inline static   float Max(float a, float b)
		{
			return a > b ? a : b;
		}
		inline static   float Min(float a, float b)
		{
			return a > b ? b : a;
		}

		inline static constexpr  double PI = 3.141592653589793238462643383279;
		/// <summary>
		/// 弧度转换为角度
		/// </summary>
		inline static constexpr  double R2D = 180.0 / PI;
		/// <summary>
		/// 角度转换为弧度
		/// </summary>
		inline static constexpr  const double D2R = PI / 180.0;
		/// <summary>
		/// pi/2
		/// </summary>
		inline static constexpr  const double PiBy2 = PI / 2.0;
		/// <summary>
		/// 1/(2pi)
		/// </summary>
		inline static constexpr  const double Inv2Pi = 1.0 / (2.0 * PI);

		/// <summary>
		/// 2.0/pi
		/// </summary>
		inline static constexpr  const double _2byPi = 2.0 / PI;
		/// <summary>
		///  弧度每秒转换为转每分钟
		/// </summary>
		inline static constexpr  const double RPS2RPM = 60.0 / (2.0 * PI);
		/// <summary>
		/// 转每分钟转换为弧度每秒
		/// </summary>
		inline static constexpr  const double RPM2RPS = (2.0 * PI) / 60.0;
		/// <summary>
		/// 1/3
		/// </summary>
		inline static constexpr  const double Onebythree = 1.0 / 3.0;

		/// <summary>
		/// 2*PI
		/// </summary>
		inline static constexpr  const double _2MutiPI = 2.0 * PI;


		/// <summary>
		/// 弧度转换为角度
		/// </summary>
		inline static constexpr  const float R2Df = (float)R2D;
		/// <summary>
		/// 角度转换为弧度
		/// </summary>
		inline static constexpr  const float D2Rf = (float)D2R;
		/// <summary>
		/// pi/2
		/// </summary>
		inline static constexpr  const float PiBy2f = (float)PiBy2;
		/// <summary>
		/// 1/(2pi)
		/// </summary>
		inline static constexpr  const float Inv2Pif = (float)Inv2Pi;

		/// <summary>
		/// 2.0/pi
		/// </summary>
		inline static constexpr  const float _2byPif = (float)_2byPi;
		/// <summary>
		///  弧度每秒转换为转每分钟
		/// </summary>
		inline static constexpr  const float RPS2RPMf = (float)RPS2RPM;
		/// <summary>
		/// 转每分钟转换为弧度每秒
		/// </summary>
		inline static constexpr  const float RPM2RPSf = (float)RPM2RPS;
		/// <summary>
		/// 1/3
		/// </summary>
		inline static constexpr  const float Onebythreef = (float)(1.0 / 3.0);
		/// <summary>
		/// 2*PI
		/// </summary>
		inline static constexpr  const float _2MutiPIf = (float)_2MutiPI;



		/// <summary>
		/// float可以表示的最小数，并以double来呈现(为一个无穷小数)
		/// </summary>
		inline static constexpr const double MinFloatD = std::numeric_limits<float>::min();// 0x1.0p-126;

		/// <summary>
		/// float可以表示的最靠近0的正数，并以double来呈现(为一个无穷靠近0的正数)
		/// </summary>
		inline static constexpr const double MinFloatEpsilonD = (double)std::numeric_limits<float>::epsilon();// 0x1.0p-23;

		/// <summary>
		/// float可以表示的最小数(为一个无穷小数)
		/// </summary>
		inline static constexpr const float MinFloat = std::numeric_limits<float>::min();// 0x1.0p-126f;

		/// <summary>
		/// float可以表示的最大数(为一个无穷大数)
		/// </summary>
		inline static constexpr const float MaxFloat = std::numeric_limits<float>::max();// 0x1.fffffep+127f;

		/// <summary>
		/// float可以表示的最靠近0的正数
		/// </summary>
		inline static constexpr const float EpsilonF = std::numeric_limits<float>::epsilon();  //0x1.0p-23f;

		/// <summary>
		/// double 可以表示的最大数(为一个无穷大数)
		/// </summary>
		inline static constexpr double MaxDouble = std::numeric_limits<double>::max();// 0x1.fffffffffffffp+1023;

		/// <summary>
		/// double 可以表示的最小数(为一个无穷小数)
		/// </summary>
		inline static constexpr double MinDouble = std::numeric_limits<double>::min();// 0x1.0p-1022;


		/// <summary>
		/// Represents the smallest positive <see cref="double"/> value that is greater than zero.
		/// </summary>
		/// <remarks>This constant is equivalent to <see cref="double.Epsilon"/> and can be used to
		/// compare  floating-point numbers for precision or to identify the smallest possible difference  between two
		/// distinct <see cref="double"/> values.</remarks>
		inline static constexpr   double EpsilonD = std::numeric_limits<double>::epsilon();
		/// <summary>
		/// Represents the conversion factor from Newtons (N) to Kilonewtons (kN).
		/// </summary>
		/// <remarks>This constant defines the multiplier used to convert a value in Newtons (N) to Kilonewtons (kN). For
		/// example, multiplying a force value in Newtons by <see cref="NtoKn"/> will yield the equivalent value in
		/// Kilonewtons.</remarks>
		inline static constexpr   double NtoKn = 0.001;//修改为0.000001 则表示输出位MN

		/// <summary>
		/// Represents the conversion factor from Newtons (N) to Meganewtons (MN).
		/// </summary>
		/// <remarks>This constant defines the multiplier used to convert a value in Newtons (N) to
		/// Meganewtons (MN). For example, multiplying a force value in Newtons by this constant will yield the
		/// equivalent value in Meganewtons.</remarks>
		inline static constexpr   const double NtoMn = 0.000001;//修改为0.000001 则表示输出位MN

		/// <summary>
		/// Represents the conversion factor from kilonewtons (kN) to newtons (N).
		/// </summary>
		/// <remarks>This constant defines the multiplier used to convert a value in kilonewtons (kN) to
		/// newtons (N). For example, multiplying a value in kN by <see cref="KntoN"/> will yield the equivalent value
		/// in N.</remarks>
		inline static constexpr    const double KntoN = 1000;//修改为1000000 则表示输出位MN

		/// <summary>
		/// Represents the conversion factor from meganewtons (MN) to newtons (N).
		/// </summary>
		/// <remarks>This constant defines the number of newtons in one meganewton.  It can be used in
		/// calculations where a conversion between these units is required.</remarks>
		inline static constexpr    const double MNtoN = 1000000;
	};

} // namespace HawtC3::IO::Math

#endif // PhysicalParameters_H