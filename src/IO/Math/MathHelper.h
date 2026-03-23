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

#ifndef MATHHELPER_H
#define MATHHELPER_H


#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <vector>
#include <functional>
#include <algorithm>
#include <random>


#include "../Log/LogHelper.h"
#include "../IO/PhysicalParameters.h"

using phy = HawtC3::IO::IO::PhysicalParameters;
using LogHelper = HawtC3::IO::Log::LogHelper;

namespace HawtC3::IO::Math
{
	class MathHelper
	{

	private:

		inline static  std::mt19937_64 urbg{ 1000 };//Mersenne Twister random engine(梅森旋转随机引擎):
		inline static  std::uniform_real_distribution<double> randomdouble{ 0.0, 1.0 };
		inline static  std::uniform_int_distribution<int> randomint{ 0,100 };

	public:
		/// <summary>
	   /// 基于种子的随机数生成器类，提供可重现的随机数序列
	   /// </summary>
	   /// <remarks>
	   /// 此类允许创建具有指定种子值的随机数生成器，支持生成各种数据类型的随机数和随机数组。
	   /// 使用相同的种子值可以确保生成相同的随机数序列，这对于测试和调试非常有用。
	   /// 
	   /// 支持的功能：
	   /// - 整数、浮点数随机数生成
	   /// - 一维和多维随机数组
	   /// - 随机矩阵和向量
	   /// - 指定范围的随机数生成
	   /// 
	   /// 示例用法：
	   /// <code>
	   /// var seededRandom = new MathHelper.RandomSeed(12345);
	   /// double value = seededRandom.Random();
	   /// int[] array = seededRandom.Random(10, 0, 100);
	   /// var matrix = seededRandom.Random(3, 3, 0.0, 1.0);
	   /// </code>
	   /// </remarks>
		class RandomSeed
		{
		private:
			/// <summary>
		  /// 内部随机数生成器实例
		  /// </summary>
			std::mt19937_64 urbg{ 1000 };//Mersenne Twister random engine(梅森旋转随机引擎):
			std::uniform_real_distribution<double> randomdouble{ 0.0, 1.0 };
			std::uniform_int_distribution<int> randomint{ 0, 2147483647 };
		public:
			RandomSeed(const RandomSeed&) = delete;
			explicit RandomSeed(int seed)
			{
				const unsigned p = seed;
				std::mt19937_64 urbg1{ p };
				urbg = urbg1;
			}

			/// <summary>
		  /// 生成一个大于等于 0.0 且小于 1.0 的随机双精度浮点数
		  /// </summary>
		  /// <returns>范围在 [0.0, 1.0) 的双精度浮点数</returns>
		  /// <remarks>
		  /// 生成的值均匀分布，意味着范围内的所有值出现的概率相等。
		  /// 
		  /// 示例：
		  /// <code>
		  /// var seededRandom = new RandomSeed(123);
		  /// double value = seededRandom.Random(); // 例如：0.7312456
		  /// </code>
		  /// </remarks>
			double Randomd()
			{
				return randomdouble(urbg);
			}

			/// <summary>
		 /// 使用底层随机数生成器生成一个随机整数
		 /// </summary>
		 /// <param name="hh">此参数当前未使用，对方法行为没有影响</param>
		 /// <returns>随机生成的整数</returns>
		 /// <remarks>
		 /// 该方法使用 <see cref="System.Random"/> 类的默认实现来生成随机数。
		 /// 
		 /// 示例：
		 /// <code>
		 /// var seededRandom = new RandomSeed(456);
		 /// int value = seededRandom.Random(0); // 例如：1847392658
		 /// </code>
		 /// </remarks>
			int Randomi()
			{
				return randomint(urbg);
			}



			/// <summary>
		 /// 在指定范围内生成随机整数数组
		 /// </summary>
		 /// <param name="arrayLength">要生成的随机整数数量。必须大于等于 0</param>
		 /// <param name="minValue">随机数的包含下界。默认为 0</param>
		 /// <param name="maxValue">随机数的排除上界。默认为 100。必须大于 <paramref name="minValue"/></param>
		 /// <returns>长度为 <paramref name="arrayLength"/> 的随机整数数组。每个整数都大于等于 <paramref name="minValue"/> 且小于 <paramref name="maxValue"/></returns>
		 /// <remarks>
		 /// 生成的每个整数都在指定范围内均匀分布。
		 /// 
		 /// 示例：
		 /// <code>
		 /// var seededRandom = new RandomSeed(789);
		 /// int[] array = seededRandom.Random(5, 10, 20);
		 /// // 结果示例：[12, 17, 11, 19, 15]
		 /// </code>
		 /// </remarks>
			std::vector<int> Randomi(int arrayLength, int minValue, int maxValue)
			{
				std::uniform_int_distribution<int> randomint{ minValue , maxValue };
				// 初始化随机整数数组
				std::vector<int> randomArray(arrayLength);

				// 填充数组
				for (int i = 0; i < arrayLength; i++)
				{
					randomArray[i] = randomint(urbg); // 生成范围内的随机整数
				}
				return randomArray;
			}


			/// <summary>
/// 在指定范围内生成随机整数数组
/// </summary>
/// <param name="arrayLength">要生成的随机整数数量。必须大于等于 0</param>
/// <param name="minValue">随机数的包含下界。默认为 0</param>
/// <param name="maxValue">随机数的排除上界。默认为 100。必须大于 <paramref name="minValue"/></param>
/// <returns>长度为 <paramref name="arrayLength"/> 的随机整数数组。每个整数都大于等于 <paramref name="minValue"/> 且小于 <paramref name="maxValue"/></returns>
/// <remarks>
/// 生成的每个整数都在指定范围内均匀分布。
/// 
/// 示例：
/// <code>
/// var seededRandom = new RandomSeed(789);
/// int[] array = seededRandom.Random(5, 10, 20);
/// // 结果示例：[12, 17, 11, 19, 15]
/// </code>
/// </remarks>
			std::vector<int> Randomi(int arrayLength)
			{
				// 初始化随机整数数组
				std::vector<int> randomArray(arrayLength);

				// 填充数组
				for (int i = 0; i < arrayLength; i++)
				{
					randomArray[i] = randomint(urbg); // 生成范围内的随机整数
				}
				return randomArray;
			}





			/// <summary>
/// 生成填充有指定范围内随机整数的二维数组
/// </summary>
/// <param name="rows">生成数组的行数。必须大于 0</param>
/// <param name="cols">生成数组的列数。必须大于 0</param>
/// <param name="minValue">随机数的包含下界。默认为 0</param>
/// <param name="maxValue">随机数的排除上界。默认为 100。必须大于 <paramref name="minValue"/></param>
/// <returns>维度为 <paramref name="rows"/> × <paramref name="cols"/> 的整数二维数组，其中每个元素都是范围 [<paramref name="minValue"/>, <paramref name="maxValue"/>) 内的随机整数</returns>
/// <remarks>
/// 生成的每个整数都在指定范围内均匀分布。
/// 
/// 示例：
/// <code>
/// var seededRandom = new RandomSeed(147);
/// int[,] matrix = seededRandom.Random(2, 3, 5, 15);
/// // 结果示例：
/// // [[ 8, 12,  7],
/// //  [11,  9, 14]]
/// </code>
/// </remarks>
			Eigen::MatrixXi Randomi(int rows, int cols, int minValue, int maxValue)
			{
				std::uniform_int_distribution<int> randomint{ minValue , maxValue };
				// 初始化随机二维数组
				Eigen::MatrixXi randomArray(rows, cols);

				// 填充数组
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						randomArray(i, j) = randomint(urbg);; // 生成范围内的随机整数
					}
				}

				return randomArray;
			}


			/// <summary>
			/// 在指定范围内生成随机双精度浮点数数组
			/// </summary>
			/// <param name="arrayLength">要生成的随机数数量。必须是非负整数</param>
			/// <param name="minValue">随机数范围的包含下界。默认为 0.0</param>
			/// <param name="maxValue">随机数范围的排除上界。默认为 1.0</param>
			/// <returns><see cref="double"/> 值数组，每个值都在范围 [<paramref name="minValue"/>, <paramref name="maxValue"/>) 内随机生成</returns>
			/// <remarks>
			/// 该方法使用均匀分布生成随机数。<paramref name="minValue"/> 是包含的，而 <paramref name="maxValue"/> 是排除的。
			/// 如果 <paramref name="arrayLength"/> 为 0，则返回空数组。
			/// 
			/// 示例：
			/// <code>
			/// var seededRandom = new RandomSeed(321);
			/// double[] array = seededRandom.Random(3, 0.0, 10.0);
			/// // 结果示例：[7.23, 2.89, 9.12]
			/// </code>
			/// </remarks>
			std::vector<double> Randomd(int arrayLength, double minValue = 0.0, double maxValue = 1.0)
			{
				std::uniform_real_distribution<double> randomdouble{ minValue, maxValue };
				// 初始化随机浮点数数组
				std::vector<double> randomArray(arrayLength);

				// 填充数组
				for (int i = 0; i < arrayLength; i++)
				{
					randomArray[i] = randomdouble(urbg); // 生成范围内的随机浮点数
				}

				return randomArray;
			}

			/// <summary>
			/// 在指定范围内生成随机浮点数数组
			/// </summary>
			/// <param name="arrayLength">要生成的随机数数量。必须大于等于 0</param>
			/// <param name="minValue">随机数范围的包含下界。默认为 0.0</param>
			/// <param name="maxValue">随机数范围的排除上界。默认为 1.0</param>
			/// <returns>随机浮点数数组，其中每个值都大于等于 <paramref name="minValue"/> 且小于 <paramref name="maxValue"/></returns>
			/// <remarks>
			/// 生成的随机数使用均匀分布。
			/// 
			/// 示例：
			/// <code>
			/// var seededRandom = new RandomSeed(654);
			/// float[] array = seededRandom.Random(4, 0.0f, 5.0f);
			/// // 结果示例：[1.23f, 4.56f, 0.78f, 3.21f]
			/// </code>
			/// </remarks>
			std::vector<float> Randomf(int arrayLength, float minValue, float maxValue)
			{
				std::uniform_real_distribution<float> randomdouble{ minValue, maxValue };
				// 初始化随机浮点数数组
				std::vector<float> randomArray(arrayLength);

				// 填充数组
				for (int i = 0; i < arrayLength; i++)
				{
					randomArray[i] = randomdouble(urbg); ; // 生成范围内的随机浮点数
				}

				return randomArray;
			}

			/// <summary>
			/// 在指定范围内生成随机浮点数向量
			/// </summary>
			/// <param name="arrayLength">结果向量中的元素数量。必须大于 0</param>
			/// <param name="minValue">随机数范围的包含下界。默认为 0.0</param>
			/// <param name="maxValue">随机数范围的排除上界。默认为 1.0</param>
			/// <param name="Vector">指示结果是否应作为向量返回。此参数当前未使用</param>
			/// <returns>包含指定范围内随机数的 <see cref="MathNet.Numerics.LinearAlgebra.Vector{T}"/> 类型为 <see langword="float"/> 的向量。向量长度等于 <paramref name="arrayLength"/></returns>
			/// <remarks>
			/// 随机数使用均匀分布生成。<paramref name="minValue"/> 是包含的，而 <paramref name="maxValue"/> 是排除的。
			/// 如果 <paramref name="minValue"/> 大于等于 <paramref name="maxValue"/>，行为未定义。
			/// 
			/// 示例：
			/// <code>
			/// var seededRandom = new RandomSeed(987);
			/// var vector = seededRandom.Random(3, 1.0f, 10.0f, true);
			/// // 结果示例：Vector [2.34, 8.67, 5.12]
			/// </code>
			/// </remarks>
			Eigen::VectorXf Randomf(int arrayLength, float minValue, float maxValue, bool Vector)
			{
				std::uniform_real_distribution<float> randomfloat{ minValue, maxValue };
				// 初始化随机浮点数数组
				Eigen::VectorXf randomArray(arrayLength);
				for (size_t i = 0; i < arrayLength; i++)
				{
					randomArray(i) = randomfloat(urbg);
				}

				return randomArray;
			}



			/// <summary>
			/// 生成具有指定范围内随机双精度值的矩阵
			/// </summary>
			/// <param name="rows">矩阵的行数。必须大于 0</param>
			/// <param name="cols">矩阵的列数。必须大于 0</param>
			/// <param name="minValue">随机值的包含下界。默认为 0.0</param>
			/// <param name="maxValue">随机值的排除上界。默认为 1.0</param>
			/// <returns>类型为 <see cref="double"/> 的 <see cref="Matrix{T}"/>，包含 <paramref name="minValue"/>（包含）和 <paramref name="maxValue"/>（排除）之间的随机值</returns>
			/// <remarks>
			/// 生成的随机值使用均匀分布。
			/// 
			/// 示例：
			/// <code>
			/// var seededRandom = new RandomSeed(258);
			/// var matrix = seededRandom.Random(2, 2, 0.0, 1.0);
			/// // 结果示例：
			/// // Matrix [[0.234, 0.789],
			/// //         [0.567, 0.123]]
			/// </code>
			/// </remarks>
			Eigen::MatrixXd Randomd(int rows, int cols, double minValue = 0.0, double maxValue = 1.0)
			{
				std::uniform_real_distribution<double> randomdouble{ minValue, maxValue };
				// 初始化随机二维数组
				Eigen::MatrixXd randomArray(rows, cols);

				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						randomArray(i, j) = randomdouble(urbg); // 生成范围内的随机浮点数
					}
				}

				return randomArray;
			}

			/// <summary>
			/// 在指定范围内生成随机浮点数矩阵
			/// </summary>
			/// <param name="rows">矩阵的行数。必须大于 0</param>
			/// <param name="cols">矩阵的列数。必须大于 0</param>
			/// <param name="minValue">随机值的包含下界。默认为 0.0f</param>
			/// <param name="maxValue">随机值的排除上界。默认为 1.0f</param>
			/// <returns>类型为 <see cref="float"/> 的 <see cref="Matrix{T}"/>，包含 <paramref name="minValue"/>（包含）和 <paramref name="maxValue"/>（排除）之间的随机值</returns>
			/// <remarks>
			/// 该方法初始化指定维度的矩阵，并用在指定范围内均匀生成的随机浮点数填充它。默认范围是 [0.0, 1.0)。
			/// 
			/// 示例：
			/// <code>
			/// var seededRandom = new RandomSeed(369);
			/// var matrix = seededRandom.frandom(3, 2, 0.0f, 10.0f);
			/// // 结果示例：
			/// // Matrix [[2.34f, 7.89f],
			/// //         [5.67f, 1.23f],
			/// //         [9.45f, 3.78f]]
			/// </code>
			/// </remarks>
			Eigen::MatrixXf Randomf(int rows, int cols, float minValue = 0.0f, float maxValue = 1.0f)
			{
				std::uniform_real_distribution<float> randomfloat{ minValue, maxValue };
				// 初始化随机二维数组
				Eigen::MatrixXf randomArray(rows, cols);

				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						randomArray(i, j) = randomfloat(urbg); // 生成范围内的随机浮点数
					}
				}

				return randomArray;
			}


			/// <summary>
			/// 在指定范围内生成随机双精度浮点数
			/// </summary>
			/// <param name="low">范围的包含下界</param>
			/// <param name="high">范围的排除上界。必须大于 <paramref name="low"/></param>
			/// <returns>大于等于 <paramref name="low"/> 且小于 <paramref name="high"/> 的随机双精度值</returns>
			/// <remarks>
			/// 生成的随机数使用均匀分布。
			/// 
			/// 示例：
			/// <code>
			/// var seededRandom = new RandomSeed(741);
			/// double value = seededRandom.Random(5.0, 15.0);
			/// // 结果示例：11.234
			/// </code>
			/// </remarks>
			double Randomd(double low, double high)
			{
				std::uniform_real_distribution<double> randomdouble{ low, high };
				return randomdouble(urbg);
			}

			/// <summary>
			/// 在指定范围内生成随机双精度浮点数
			/// </summary>
			/// <param name="low">要生成的随机数的包含下界</param>
			/// <param name="high">要生成的随机数的排除上界</param>
			/// <returns>大于等于 <paramref name="low"/> 且小于 <paramref name="high"/> 的随机双精度浮点数</returns>
			/// <remarks>
			/// 该方法使用均匀分布生成随机数。
			/// 
			/// 示例：
			/// <code>
			/// var seededRandom = new RandomSeed(852);
			/// double value = seededRandom.Random(10, 20);
			/// // 结果示例：15.678 (实际上是整数范围的随机数)
			/// </code>
			/// </remarks>
			float Randomf(float low, float high)
			{
				std::uniform_real_distribution<float> randomdouble{ low, high };
				return randomdouble(urbg);
			}

		};


		/// <summary>
		/// 生成一个大于等于 0.0 且小于 1.0 的随机双精度浮点数
		/// </summary>
		/// <returns>范围在 [0.0, 1.0) 的双精度浮点数</returns>
		/// <remarks>
		/// 生成的值均匀分布，意味着范围内的所有值出现的概率相等。
		/// 
		/// 示例：
		/// <code>
		/// double randomValue = MathHelper.Random();
		/// // 结果示例：0.7312456
		/// </code>
		/// </remarks>
		static double Randomd()
		{
			return randomdouble(urbg);
		}

		/// <summary>
		/// 使用默认随机数生成器生成一个[0,100]的随机整数
		/// </summary>
		/// <param name="hh">此参数当前未使用，对方法行为没有影响</param>
		/// <returns>随机生成的整数</returns>
		/// <remarks>
		/// 该方法使用全局随机数生成器实例。
		/// 
		/// 示例：
		/// <code>
		/// int randomInt = MathHelper.Random(0);
		/// // 结果示例：18
		/// </code>
		/// </remarks>
		static int Randomi(int hh = 0)
		{
			return randomint(urbg);
		}

		/// <summary>
		/// 在指定范围内生成随机整数数组
		/// </summary>
		/// <param name="arrayLength">结果数组中的元素数量。必须大于等于 0</param>
		/// <param name="minValue">要生成的随机数的包含下界。默认为 0</param>
		/// <param name="maxValue">要生成的随机数的排除上界。默认为 100。必须大于 <paramref name="minValue"/></param>
		/// <returns>长度为 <paramref name="arrayLength"/> 的随机整数数组。每个整数都大于等于 <paramref name="minValue"/> 且小于 <paramref name="maxValue"/></returns>
		/// <remarks>
		/// 生成的每个整数都在指定范围内均匀分布。
		/// 
		/// 示例：
		/// <code>
		/// auto randomArray = MathHelper.Random(5, 1, 10);
		/// // 结果示例：[3, 7, 2, 9, 5]
		/// </code>
		/// </remarks>
		static std::vector<int> Randomi(int arrayLength, int minValue = 0, int maxValue = 100)
		{
			std::uniform_int_distribution<int> randomintt{ minValue, maxValue }; //
			// 初始化随机整数数组
			std::vector<int> randomArray(arrayLength);

			// 填充数组
			for (int i = 0; i < arrayLength; i++)
			{
				randomArray[i] = randomintt(urbg); // 生成范围内的随机整数
			}
			return randomArray;
		}

		/// <summary>
		/// 在指定范围内生成随机双精度浮点数数组
		/// </summary>
		/// <param name="arrayLength">要生成的随机值数量。必须是非负整数</param>
		/// <param name="minValue">随机值的包含下界。默认为 0.0</param>
		/// <param name="maxValue">随机值的排除上界。默认为 1.0。必须大于 <paramref name="minValue"/></param>
		/// <returns><see cref="double"/> 值数组，每个值都在范围 [<paramref name="minValue"/>, <paramref name="maxValue"/>) 内随机生成</returns>
		/// <remarks>
		/// 该方法使用均匀分布生成随机值。生成的值包含 <paramref name="minValue"/> 且排除 <paramref name="maxValue"/>。
		/// 
		/// 示例：
		/// <code>
		/// double[] randomArray = MathHelper.Random(3, 0.0, 5.0);
		/// // 结果示例：[1.23, 4.56, 2.89]
		/// </code>
		/// </remarks>
		static std::vector<double> Randomd(int arrayLength, double minValue = 0.0, double maxValue = 1.0)
		{
			std::uniform_real_distribution<double> randomdouble{ minValue, maxValue };
			// 初始化随机浮点数数组
			std::vector<double> randomArray(arrayLength);

			// 填充数组
			for (int i = 0; i < arrayLength; i++)
			{
				randomArray[i] = randomdouble(urbg); // 生成范围内的随机浮点数
			}

			return randomArray;
		}





		/// <summary>
/// 在指定范围内生成随机单精度浮点数数组
/// </summary>
/// <param name="arrayLength">要生成的随机值数量。必须是非负整数</param>
/// <param name="minValue">随机值的包含下界。默认为 0.0</param>
/// <param name="maxValue">随机值的排除上界。默认为 1.0。必须大于 <paramref name="minValue"/></param>
/// <returns><see cref="double"/> 值数组，每个值都在范围 [<paramref name="minValue"/>, <paramref name="maxValue"/>) 内随机生成</returns>
/// <remarks>
/// 该方法使用均匀分布生成随机值。生成的值包含 <paramref name="minValue"/> 且排除 <paramref name="maxValue"/>。
/// 
/// 示例：
/// <code>
/// double[] randomArray = MathHelper.Random(3, 0.0, 5.0);
/// // 结果示例：[1.23, 4.56, 2.89]
/// </code>
/// </remarks>
		static std::vector<float> Randomf(int arrayLength, float minValue = 0.0, float maxValue = 1.0)
		{
			std::uniform_real_distribution<float> randomdouble{ minValue, maxValue };
			// 初始化随机浮点数数组
			std::vector<float> randomArray(arrayLength);

			// 填充数组
			for (int i = 0; i < arrayLength; i++)
			{
				randomArray[i] = randomdouble(urbg); // 生成范围内的随机浮点数
			}

			return randomArray;
		}








		/// <summary>
		/// 生成指定范围内随机整数的二维数组
		/// </summary>
		/// <param name="rows">生成数组的行数。必须大于 0</param>
		/// <param name="cols">生成数组的列数。必须大于 0</param>
		/// <param name="minValue">随机数的包含下界。默认为 0</param>
		/// <param name="maxValue">随机数的排除上界。默认为 100。必须大于 <paramref name="minValue"/></param>
		/// <returns>维度为 <paramref name="rows"/> × <paramref name="cols"/> 的整数二维数组。每个元素都是范围 [<paramref name="minValue"/>, <paramref name="maxValue"/>) 内的随机整数</returns>
		/// <remarks>
		/// 生成的每个元素都在指定范围内均匀分布。
		/// 
		/// 示例：
		/// <code>
		/// int[,] matrix = MathHelper.Random(2, 3, 10, 20);
		/// // 结果示例：
		/// // [[12, 17, 11],
		/// //  [19, 15, 13]]
		/// </code>
		/// </remarks>
		static Eigen::MatrixXi Randomi(int rows, int cols, int minValue = 0, int maxValue = 100)
		{
			std::uniform_int_distribution<int> randomintt{ minValue, maxValue }; //
			// 初始化随机二维数组
			Eigen::MatrixXi randomArray(rows, cols);

			// 填充数组
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					randomArray(i, j) = randomintt(urbg); // 生成范围内的随机整数
				}
			}

			return randomArray;
		}

		/// <summary>
		/// 生成具有指定维度且填充有指定范围内随机双精度值的矩阵
		/// </summary>
		/// <param name="rows">矩阵的行数。必须大于 0</param>
		/// <param name="cols">矩阵的列数。必须大于 0</param>
		/// <param name="minValue">随机值的包含下界。默认为 0.0</param>
		/// <param name="maxValue">随机值的排除上界。默认为 1.0</param>
		/// <returns>大小为 <paramref name="rows"/> × <paramref name="cols"/> 的矩阵，包含范围 [<paramref name="minValue"/>, <paramref name="maxValue"/>) 内的随机双精度值</returns>
		/// <remarks>
		/// 生成的随机值使用均匀分布。
		/// 
		/// 示例：
		/// <code>
		/// var matrix = MathHelper.Random(2, 2, 0.0, 10.0);
		/// // 结果示例：
		/// // Matrix [[2.34, 7.89],
		/// //         [5.67, 1.23]]
		/// </code>
		/// </remarks>
		static Eigen::MatrixXd Randomd(int rows, int cols, double minValue = 0.0, double maxValue = 1)
		{
			std::uniform_real_distribution<double> randomdouble{ minValue, maxValue };
			// 初始化随机二维数组
			Eigen::MatrixXd randomArray(rows, cols);

			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					randomArray(i, j) = randomdouble(urbg); // 生成范围内的随机浮点数
				}
			}

			return randomArray;
		}



		/// <summary>
/// 生成具有指定维度且填充有指定范围内随机单精度值的矩阵
/// </summary>
/// <param name="rows">矩阵的行数。必须大于 0</param>
/// <param name="cols">矩阵的列数。必须大于 0</param>
/// <param name="minValue">随机值的包含下界。默认为 0.0</param>
/// <param name="maxValue">随机值的排除上界。默认为 1.0</param>
/// <returns>大小为 <paramref name="rows"/> × <paramref name="cols"/> 的矩阵，包含范围 [<paramref name="minValue"/>, <paramref name="maxValue"/>) 内的随机单精度值</returns>
/// <remarks>
/// 生成的随机值使用均匀分布。
/// 
/// 示例：
/// <code>
/// var matrix = MathHelper.Random(2, 2, 0.0, 10.0);
/// // 结果示例：
/// // Matrix [[2.34, 7.89],
/// //         [5.67, 1.23]]
/// </code>
/// </remarks>
		static Eigen::MatrixXf Randomf(int rows, int cols, float minValue = 0.0, float maxValue = 1)
		{
			std::uniform_real_distribution<float> randomdouble{ minValue, maxValue };
			// 初始化随机二维数组
			Eigen::MatrixXf randomArray(rows, cols);

			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					randomArray(i, j) = randomdouble(urbg); // 生成范围内的随机浮点数
				}
			}

			return randomArray;
		}





		/// <summary>
		/// 在指定范围内生成随机双精度浮点数
		/// </summary>
		/// <param name="low">随机数范围的包含下界</param>
		/// <param name="high">随机数范围的排除上界</param>
		/// <returns>大于等于 <paramref name="low"/> 且小于 <paramref name="high"/> 的双精度浮点数</returns>
		/// <remarks>
		/// 该方法使用均匀分布生成随机数。
		/// 
		/// 示例：
		/// <code>
		/// double value = MathHelper.Random(5.0, 15.0);
		/// // 结果示例：11.234
		/// </code>
		/// </remarks>
		static double Randomd(double low, double high)
		{
			std::uniform_real_distribution<double> randomdouble{ low, high };
			return randomdouble(urbg);
		}

		/// <summary>
/// 在指定范围内生成随机单精度浮点数
/// </summary>
/// <param name="low">随机数范围的包含下界</param>
/// <param name="high">随机数范围的排除上界</param>
/// <returns>大于等于 <paramref name="low"/> 且小于 <paramref name="high"/> 的单精度浮点数</returns>
/// <remarks>
/// 该方法使用均匀分布生成随机数。
/// 
/// 示例：
/// <code>
/// double value = MathHelper.Random(5.0, 15.0);
/// // 结果示例：11.234
/// </code>
/// </remarks>
		static float Randomf(float low, float high)
		{
			std::uniform_real_distribution<float> randomdouble{ low, high };
			return randomdouble(urbg);
		}

		/// <summary>
		/// 在指定范围内生成随机整数
		/// </summary>
		/// <param name="low">随机数范围的包含下界</param>
		/// <param name="high">随机数范围的排除上界</param>
		/// <returns>大于等于 <paramref name="low"/> 且小于 <paramref name="high"/> 的随机整数</returns>
		/// <remarks>
		/// 该方法使用均匀分布生成随机数。注意此重载实际上调用整数范围的随机数生成方法。
		/// 
		/// 示例：
		/// <code>
		/// double value = MathHelper.Random(10, 20);
		/// // 结果示例：15.0 (整数范围内的随机数)
		/// </code>
		/// </remarks>
		static int Randomi(int low, int high)
		{
			std::uniform_int_distribution<int> randomintt{ low, high }; //
			return randomintt(urbg);
		}

		/// <summary>
		/// 将角度规范化到范围 [0, 2π) 内
		/// </summary>
		/// <param name="angle">要规范化的角度，以弧度为单位。可以是任何实数</param>
		/// <returns>表示范围 [0, 2π) 内等效角度的双精度数</returns>
		/// <remarks>
		/// 此方法确保返回的角度始终为非负且小于 2π。对于需要角度在标准范围内的应用程序很有用，
		/// 例如三角函数计算或几何变换。
		/// 
		/// 算法使用模运算和条件加法来处理负角度和大于 2π 的角度。
		/// 
		/// 示例：
		/// <code>
		/// double angle1 = MathHelper.Zero2TwoPi(-phy::PI / 2);
		/// // 结果：3π/2 (约 4.712)
		/// 
		/// double angle2 = MathHelper.Zero2TwoPi(3 * phy::PI);
		/// // 结果：π (约 3.142)
		/// 
		/// double angle3 = MathHelper.Zero2TwoPi(phy::PI / 4);
		/// // 结果：π/4 (约 0.785)
		/// </code>
		/// </remarks>
		static double Zero2TwoPi(double angle)
		{

			angle = fmod(angle, 2.0 * phy::PI);
			if (angle < 0)
			{
				angle += 2.0 * phy::PI;
			}
			return angle;
		}

		/// <summary>
		/// 将双精度浮点值四舍五入到指定的小数位数，并确保结果反映原始值的精度
		/// </summary>
		/// <param name="value">要四舍五入的双精度浮点值</param>
		/// <param name="decimalPlaces">要四舍五入到的小数位数。必须是非负整数</param>
		/// <returns>四舍五入到指定小数位数的双精度浮点值</returns>
		/// <remarks>
		/// 此方法使用 <see cref="Math.Round(double)"/> 执行四舍五入操作，并调整结果以匹配指定的精度。
		/// 该方法通过乘以适当的 10 的幂、四舍五入，然后除以相同的因子来工作。
		/// 
		/// 示例：
		/// <code>
		/// double result1 = MathHelper.MirrorRound(3.14159, 2);
		/// // 结果：3.14
		/// 
		/// double result2 = MathHelper.MirrorRound(2.71828, 4);
		/// // 结果：2.7183
		/// 
		/// double result3 = MathHelper.MirrorRound(123.456789, 0);
		/// // 结果：123.0
		/// </code>
		/// </remarks>
		static double MirrorRound(double value, int decimalPlaces)
		{
			double factor = pow(10, decimalPlaces);
			double rounded = round(value * factor);
			double mirrored = rounded / factor;
			return mirrored;
		}

		/// <summary>
		/// 在指定范围内生成整数序列
		/// </summary>
		/// <param name="start">序列中第一个整数的值</param>
		/// <param name="count">要生成的连续整数数量。必须是非负数</param>
		/// <returns>从 <paramref name="start"/> 开始且包含 <paramref name="count"/> 个元素的整数数组</returns>
		/// <remarks>
		/// 此方法生成从指定起始值开始的连续整数序列。
		/// 
		/// 示例：
		/// <code>
		/// int[] range1 = MathHelper.Range(5, 3);
		/// // 结果：[5, 6, 7]
		/// 
		/// int[] range2 = MathHelper.Range(0, 5);
		/// // 结果：[0, 1, 2, 3, 4]
		/// 
		/// int[] range3 = MathHelper.Range(-2, 4);
		/// // 结果：[-2, -1, 0, 1]
		/// </code>
		/// </remarks>
		static Eigen::VectorXi Range(int start, int count) {
			Eigen::VectorXi result(count);
			for (int i = 0; i < count; ++i) {
				result(i) = start + i;
			}
			return result;
		}


		/// <summary>
		/// 创建包含从零开始到指定值（不包括）的整数序列的向量
		/// </summary>
		/// <param name="start">序列的排除上界。必须是非负整数</param>
		/// <returns>类型为 <see cref="MathNet.Numerics.LinearAlgebra.Vector{T}"/> 的向量，包含从 0 到 <paramref name="start"/> - 1 的整数序列。如果 <paramref name="start"/> 为 0，向量将为空</returns>
		/// <remarks>
		/// 此方法创建一个从 0 开始的整数序列向量，类似于 Python 的 range() 函数。
		/// 
		/// 示例：
		/// <code>
		/// var vector1 = MathHelper.Range(5);
		/// // 结果：Vector [0, 1, 2, 3, 4]
		/// 
		/// var vector2 = MathHelper.Range(3);
		/// // 结果：Vector [0, 1, 2]
		/// 
		/// var vector3 = MathHelper.Range(0);
		/// // 结果：Vector [] (空向量)
		/// </code>
		/// </remarks>
		static Eigen::VectorXd Range(int start) {
			Eigen::VectorXd result(start);
			for (int i = 0; i < start; ++i) {
				result(i) = static_cast<double>(i);
			}
			return result;
		}


		/// <summary>
		/// 使用定义的步长值在指定范围内生成整数序列
		/// </summary>
		/// <param name="start">序列的起始值</param>
		/// <param name="end">序列的包含上界</param>
		/// <param name="step">序列中连续值之间的增量。必须大于 0</param>
		/// <returns>表示从 <paramref name="start"/> 到 <paramref name="end"/> 且增量为 <paramref name="step"/> 的序列的整数数组</returns>
		/// <remarks>
		/// 如果 <paramref name="start"/> 大于 <paramref name="end"/>，该方法将返回空数组。
		/// 
		/// 示例：
		/// <code>
		/// int[] range1 = MathHelper.Range(0, 10, 2);
		/// // 结果：[0, 2, 4, 6, 8, 10]
		/// 
		/// int[] range2 = MathHelper.Range(1, 10, 3);
		/// // 结果：[1, 4, 7, 10]
		/// 
		/// int[] range3 = MathHelper.Range(5, 20, 5);
		/// // 结果：[5, 10, 15, 20]
		/// </code>
		/// </remarks>
		static Eigen::VectorXi Range(int start, int end, int step) {
			int count = (end - start) / step + 1;
			Eigen::VectorXi result(count);
			int idx = 0;
			for (int i = start; i <= end; i += step) {
				result(idx++) = i;
			}
			return result;
		}

		/// <summary>
		/// 在指定范围内创建均匀间隔值的向量
		/// </summary>
		/// <param name="start">范围的起始值</param>
		/// <param name="end">范围的结束值。如果与步长大小对齐，范围包括此值</param>
		/// <param name="step">范围中连续值之间的增量。必须非零。正值生成递增序列，负值生成递减序列</param>
		/// <returns>包含从 <paramref name="start"/> 开始并按 <paramref name="step"/> 递增直到值超过 <paramref name="end"/>（对于负步长则小于 <paramref name="end"/>）的值的向量</returns>
		/// <remarks>
		/// 结果向量的长度由范围和步长大小确定。如果范围不能被步长大小整除，
		/// 向量中的最后一个值将是与步长大小对齐的范围内的最大值。
		/// 
		/// 示例：
		/// <code>
		/// var vector1 = MathHelper.Range(0.0, 10.0, 2.5);
		/// // 结果：Vector [0.0, 2.5, 5.0, 7.5, 10.0]
		/// 
		/// var vector2 = MathHelper.Range(1.0, 5.0, 1.0);
		/// // 结果：Vector [1.0, 2.0, 3.0, 4.0, 5.0]
		/// 
		/// var vector3 = MathHelper.Range(10.0, 0.0, -2.0);
		/// // 结果：Vector [10.0, 8.0, 6.0, 4.0, 2.0, 0.0]
		/// </code>
		/// </remarks>
		static Eigen::VectorXd Range(double start, double end, double step) {
			if (step == 0) {
				LogHelper::ErrorLog("step不能为0");
				throw std::invalid_argument("step cannot be zero");
			}

			int length = static_cast<int>(round((end - start) / step)) + 1;
			Eigen::VectorXd result(length);

			for (int i = 0; i < length; ++i) {
				result(i) = start + i * step;
			}
			return result;
		}

		/// <summary>
		/// 在指定范围内创建均匀间隔值的向量
		/// </summary>
		/// <param name="start">范围的起始值</param>
		/// <param name="end">范围的结束值</param>
		/// <param name="step">范围中连续值之间的增量。必须非零。正值生成递增序列，负值生成递减序列</param>
		/// <returns>包含从 <paramref name="start"/> 开始到 <paramref name="end"/> 结束、步长为 <paramref name="step"/> 的值的向量。如果与步长大小对齐，向量包括 <paramref name="end"/> 值</returns>
		/// <remarks>
		/// 此方法使用指定的步长生成值序列并将其转换为向量。结果向量的长度由范围和步长确定。
		/// 
		/// 示例：
		/// <code>
		/// var vector1 = MathHelper.Range(0.0f, 5.0f, 1.0f);
		/// // 结果：Vector [0.0, 1.0, 2.0, 3.0, 4.0, 5.0]
		/// 
		/// var vector2 = MathHelper.Range(1.5f, 4.0f, 0.5f);
		/// // 结果：Vector [1.5, 2.0, 2.5, 3.0, 3.5, 4.0]
		/// </code>
		/// </remarks>
		static Eigen::VectorXf Range(float start, float end, float step) {
			if (step == 0) {
				LogHelper::ErrorLog("step不能为0");
				throw std::invalid_argument("step cannot be zero");
			}

			int length = static_cast<int>(round((end - start) / step)) + 1;
			return linspace(start, end, length);
		}

		/// <summary>
		/// 从指定值开始，使用定义的步长和长度生成均匀间隔值的向量
		/// </summary>
		/// <param name="start">序列的起始值</param>
		/// <param name="step">序列中连续值之间的差值。不能为零</param>
		/// <param name="length">要生成的值数量。必须是非负整数</param>
		/// <returns>包含 <paramref name="length"/> 个均匀间隔值的向量，从 <paramref name="start"/> 开始，每个后续值按 <paramref name="step"/> 递增</returns>
		/// <remarks>
		/// 此方法对于生成数字序列很有用，例如用于数值计算或数据分析。
		/// 
		/// 示例：
		/// <code>
		/// var vector1 = MathHelper.Range(1.0f, 0.5f, 5);
		/// // 结果：Vector [1.0, 1.5, 2.0, 2.5, 3.0]
		/// 
		/// var vector2 = MathHelper.Range(0.0f, -1.0f, 4);
		/// // 结果：Vector [0.0, -1.0, -2.0, -3.0]
		/// 
		/// var vector3 = MathHelper.Range(10.0f, 2.0f, 3);
		/// // 结果：Vector [10.0, 12.0, 14.0]
		/// </code>
		/// </remarks>
		static Eigen::VectorXf Range(float start, float step, int length) {
			if (step == 0) {
				LogHelper::ErrorLog("step不能为0");
				throw std::invalid_argument("step cannot be zero");
			}

			Eigen::VectorXf result(length);
			for (int i = 0; i < length; ++i) {
				result(i) = start + step * i;
			}
			return result;
		}

		/// <summary>
		/// 创建包含指定起始点和结束点之间线性间隔值序列的向量
		/// </summary>
		/// <param name="start">序列的起始值</param>
		/// <param name="end">序列的结束值</param>
		/// <param name="length">序列中值的数量。必须大于零</param>
		/// <returns>类型为 <see cref="MathNet.Numerics.LinearAlgebra.Vector{double}"/> 的向量，包含 <paramref name="start"/> 和 <paramref name="end"/> 之间的 <paramref name="length"/> 个线性间隔值</returns>
		/// <remarks>
		/// 此方法生成均匀分布的数值序列，包括起始值和结束值。
		/// 
		/// 示例：
		/// <code>
		/// var vector1 = MathHelper.Range(0.0, 10.0, 5);
		/// // 结果：Vector [0.0, 2.5, 5.0, 7.5, 10.0]
		/// 
		/// var vector2 = MathHelper.Range(-1.0, 1.0, 3);
		/// // 结果：Vector [-1.0, 0.0, 1.0]
		/// 
		/// var vector3 = MathHelper.Range(1.0, 2.0, 11);
		/// // 结果：Vector [1.0, 1.1, 1.2, ..., 1.9, 2.0]
		/// </code>
		/// </remarks>
		static Eigen::VectorXd Range(double start, double end, int length) {
			return linspace(start, end, length);
		}

		/// <summary>
		/// 在指定范围内生成均匀间隔的双精度数序列
		/// </summary>
		/// <param name="start">序列的起始值</param>
		/// <param name="end">序列的结束值</param>
		/// <param name="length">序列中要生成的值数量。必须大于等于 2</param>
		/// <returns>表示均匀间隔序列的 <see cref="double"/> 值数组。数组将包含 <paramref name="length"/> 个元素，第一个元素等于 <paramref name="start"/>，最后一个元素等于 <paramref name="end"/></returns>
		/// <remarks>
		/// 此方法对于生成用于绘图、插值或数值分析等任务的数值范围很有用。
		/// 连续值之间的间距经过计算以确保整个范围内的均匀分布。
		/// 
		/// 示例：
		/// <code>
		/// double[] array1 = MathHelper.linspace(0.0, 10.0, 5);
		/// // 结果：[0.0, 2.5, 5.0, 7.5, 10.0]
		/// 
		/// double[] array2 = MathHelper.linspace(-5.0, 5.0, 11);
		/// // 结果：[-5.0, -4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0]
		/// 
		/// double[] array3 = MathHelper.linspace(1.0, 1.0, 3);
		/// // 结果：[1.0, 1.0, 1.0]
		/// </code>
		/// </remarks>
		static Eigen::VectorXd linspace(double start, double end, int length) {
			if (length <= 1) {
				Eigen::VectorXd result(1);
				result(0) = start;
				return result;
			}

			Eigen::VectorXd result(length);
			double step = (end - start) / (length - 1);

			for (int i = 0; i < length; ++i) {
				result(i) = start + i * step;
			}
			return result;
		}

		/// <summary>
		/// 在指定范围内生成均匀间隔的双精度数序列
		/// </summary>
		/// <param name="start">序列的起始值</param>
		/// <param name="end">序列的结束值</param>
		/// <param name="length">序列中要生成的值数量。必须大于等于 2</param>
		/// <returns>表示均匀间隔序列的 <see cref="double"/> 值数组。数组将包含 <paramref name="length"/> 个元素，第一个元素等于 <paramref name="start"/>，最后一个元素等于 <paramref name="end"/></returns>
		/// <remarks>
		/// 此方法对于生成用于绘图、插值或数值分析等任务的数值范围很有用。
		/// 连续值之间的间距经过计算以确保整个范围内的均匀分布。
		/// 
		/// 示例：
		/// <code>
		/// double[] array1 = MathHelper.linspace(0.0, 10.0, 5);
		/// // 结果：[0.0, 2.5, 5.0, 7.5, 10.0]
		/// 
		/// double[] array2 = MathHelper.linspace(-5.0, 5.0, 11);
		/// // 结果：[-5.0, -4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0]
		/// 
		/// double[] array3 = MathHelper.linspace(1.0, 1.0, 3);
		/// // 结果：[1.0, 1.0, 1.0]
		/// </code>
		/// </remarks>
		static Eigen::VectorXf linspace(float start, float end, int length) {
			if (length <= 1) {
				Eigen::VectorXf result(1);
				result(0) = start;
				return result;
			}

			Eigen::VectorXf result(length);
			float step = (end - start) / (length - 1);

			for (int i = 0; i < length; ++i) {
				result(i) = start + i * step;
			}
			return result;
		}


		/// <summary>
		   /// 创建指定维度的二维矩阵，所有元素都初始化为指定值
		   /// </summary>
		template<typename T>
		static Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> zeros(T value, int m, int n) {
			return Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Constant(m, n, value);
		}

		/// <summary>
		/// 创建指定维度的三维数组，所有元素都初始化为指定值
		/// </summary>
		template<typename T>
		static std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>> zeros(T input, int m, int n, int k) {
			std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>> result(m);
			for (int i = 0; i < m; ++i) {
				result[i] = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Constant(n, k, input);
			}
			return result;
		}

		/// <summary>
		/// 创建指定维度的四维数组，所有元素都初始化为给定值
		/// </summary>
		template<typename T>
		static std::vector<std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>> zeros(T input, int m, int n, int k, int mm) {
			std::vector<std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>> result(m);
			for (int i = 0; i < m; ++i) {
				result[i].resize(n);
				for (int j = 0; j < n; ++j) {
					result[i][j] = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Constant(k, mm, input);
				}
			}
			return result;
		}

		/// <summary>
		/// 创建具有指定维度且初始化为给定值的矩阵
		/// </summary>
		static Eigen::MatrixXd zeros(int m, int n, double value = 0.0) {
			return Eigen::MatrixXd::Constant(m, n, value);
		}


		/// <summary>
		/// 对输入向量的每个元素应用指定函数，并返回包含结果的新向量
		/// </summary>
		/// <param name="f">接受双精度值作为输入并返回双精度值作为输出的函数</param>
		/// <param name="input">要应用函数 <paramref name="f"/> 的输入向量</param>
		/// <returns>包含对 <paramref name="input"/> 向量的每个元素应用函数 <paramref name="f"/> 结果的新向量</returns>
		/// <remarks>
		/// 此方法提供了一种函数式编程方式来变换向量中的每个元素。
		/// 它创建一个新向量，而不是修改原始向量。
		/// 
		/// 该方法在数值计算中特别有用，可以应用数学函数、变换或任何自定义操作到向量的所有元素。
		/// 
		/// 示例：
		/// <code>
		/// var input = LinearAlgebraHelper.zeros(new double[] { 1.0, 2.0, 3.0, 4.0 });
		/// 
		/// // 计算每个元素的平方
		/// var squares = MathHelper.FunAT(x => x * x, input);
		/// // 结果：Vector [1.0, 4.0, 9.0, 16.0]
		/// 
		/// // 应用正弦函数
		/// var sines = MathHelper.FunAT(Math.Sin, input);
		/// // 结果：Vector [sin(1), sin(2), sin(3), sin(4)]
		/// 
		/// // 应用自定义函数
		/// var custom = MathHelper.FunAT(x => 2 * x + 1, input);
		/// // 结果：Vector [3.0, 5.0, 7.0, 9.0]
		/// </code>
		/// </remarks>
		static Eigen::VectorXd FunAT(std::function<double(double)> f, const Eigen::VectorXd& input) {
			Eigen::VectorXd result(input.size());
			for (int i = 0; i < input.size(); ++i) {
				result(i) = f(input(i));
			}
			return result;
		}


		/// <summary>
	/// 对输入向量的每个元素应用指定函数（使用Eigen的unaryExpr）
	/// </summary>
		template<typename Func>
		static Eigen::VectorXd FunATExpr(Func f, const Eigen::VectorXd& input) {
			return input.unaryExpr(f);
		}

		/// <summary>
		/// 将双精度浮点数提升到指定整数指数的幂
		/// </summary>
		/// <param name="x">要提升到幂的底数</param>
		/// <param name="n">底数要提升到的指数。可以是正数、负数或零</param>
		/// <returns>将 <paramref name="x"/> 提升到 <paramref name="n"/> 次幂的结果。如果 <paramref name="n"/> 为 0，方法返回 1.0。如果 <paramref name="n"/> 为负数，方法返回结果的倒数</returns>
		/// <remarks>
		/// 此方法使用高效算法通过基于指数的二进制表示，迭代平方底数并仅在必要时相乘来计算幂。
		/// 该方法处理正指数和负指数，以及指数为 0 的特殊情况。
		/// 
		/// 算法基于快速幂算法（也称为二进制指数），其时间复杂度为 O(log n)，
		/// 比朴素的 O(n) 算法更高效。
		/// 
		/// 特殊情况：
		/// - 如果 n = 0，返回 1.0（任何数的0次幂等于1）
		/// - 如果 n = 1，返回 x（任何数的1次幂等于自身）
		/// - 如果 n < 0，返回 1 / (x^|n|)（负指数的处理）
		/// 
		/// 示例：
		/// <code>
		/// double result1 = MathHelper.Pow1(2.0, 10);
		/// // 结果：1024.0 (2的10次幂)
		/// 
		/// double result2 = MathHelper.Pow1(3.0, -2);
		/// // 结果：0.1111... (1/(3^2) = 1/9)
		/// 
		/// double result3 = MathHelper.Pow1(5.0, 0);
		/// // 结果：1.0 (任何数的0次幂)
		/// 
		/// double result4 = MathHelper.Pow1(7.0, 1);
		/// // 结果：7.0 (任何数的1次幂)
		/// </code>
		/// </remarks>
		static double Pow1(double x, int n)
		{
			if (n == 0) return 1.0;
			if (n == 1) return x;

			double result = 1.0;
			int absN = abs(n);

			for (int i = 0; i < 31; ++i) {
				if (((1 << i) & absN) != 0) {
					result *= x;
				}
				x *= x;
			}

			return n < 0 ? 1.0 / result : result;
		}

		/// <summary>
		/// 在指定集合中查找满足与给定值比较条件的第一个元素的索引
		/// </summary>
		/// <typeparam name="T">集合中元素的类型。必须实现 <see cref="IComparable{T}"/></typeparam>
		/// <param name="xvec">要搜索的元素集合。不能为 null 或空</param>
		/// <param name="x">要与集合中元素比较的值</param>
		/// <param name="error">布尔值，指示如果未找到匹配索引是否记录错误。如果为 <see langword="true"/>，记录错误；否则记录警告</param>
		/// <returns>满足比较条件的第一个元素的从零开始的索引。如果集合按升序排序，方法返回第一个大于等于 <paramref name="x"/> 的元素索引。如果集合按降序排序，方法返回第一个小于等于 <paramref name="x"/> 的元素索引。如果未找到此类元素，方法返回 -1</returns>
		/// <exception cref="ArgumentException">如果 <paramref name="xvec"/> 为 null 或空，则抛出此异常</exception>
		/// <remarks>
		/// 该方法假设集合按升序或降序排序。如果集合未排序，行为未定义。
		/// 如果未找到匹配索引，方法会记录有关搜索过程的附加信息。
		/// 
		/// 算法工作原理：
		/// 1. 首先检查集合是否为空
		/// 2. 确定集合是升序还是降序排序
		/// 3. 根据排序方向查找合适的索引
		/// 4. 如果未找到，根据 error 参数记录相应的日志
		/// 5. 返回找到的索引减1（如果索引不为0）
		/// 
		/// 应用场景：
		/// - 在排序数组中进行二分查找的前置步骤
		/// - 插值算法中查找插值点
		/// - 数值分析中的区间定位
		/// 
		/// 示例：
		/// <code>
		/// var sortedArray = new double[] { 1.0, 3.0, 5.0, 7.0, 9.0 };
		/// 
		/// int index1 = MathHelper.FindIndex(sortedArray, 4.0);
		/// // 结果：1 (值4.0应该插入到索引1和2之间，返回1)
		/// 
		/// int index2 = MathHelper.FindIndex(sortedArray, 7.0);
		/// // 结果：2 (找到确切匹配值7.0在索引3，返回2)
		/// 
		/// var descendingArray = new double[] { 9.0, 7.0, 5.0, 3.0, 1.0 };
		/// int index3 = MathHelper.FindIndex(descendingArray, 6.0);
		/// // 结果：1 (在降序数组中找到第一个小于等于6.0的值)
		/// </code>
		/// </remarks>
	/// <summary>
	/// 在指定向量中查找满足与给定值比较条件的第一个元素的索引
	/// </summary>
		static int FindIndex(const Eigen::VectorXd& xvec, double x, bool error = true) {
			if (xvec.size() == 0) {
				throw std::invalid_argument("数组不能为空");
			}

			bool isAscending = xvec(0) < xvec(xvec.size() - 1);

			for (int i = 0; i < xvec.size(); ++i) {
				if (isAscending ? xvec(i) >= x : xvec(i) <= x) {
					return i != 0 ? i - 1 : i;
				}
			}

			if (error) {
				LogHelper::ErrorLog("未找到索引!");
			}
			else {
				LogHelper::WarnLog("未找到索引!", " FindIndex");
			}
			return -1;
		}

		/// <summary>
		/// 在指定向量中查找满足与给定值比较条件的第一个元素的索引（整数版本）
		/// </summary>
		static int FindIndex(const Eigen::VectorXi& xvec, double x, bool error = true) {
			if (xvec.size() == 0) {
				throw std::invalid_argument("数组不能为空");
			}

			bool isAscending = xvec(0) < xvec(xvec.size() - 1);

			for (int i = 0; i < xvec.size(); ++i) {
				if (isAscending ? static_cast<double>(xvec(i)) >= x : static_cast<double>(xvec(i)) <= x) {
					return i != 0 ? i - 1 : i;
				}
			}

			if (error) {
				LogHelper::ErrorLog("未找到索引!");
			}
			else {
				LogHelper::WarnLog("未找到索引!", " FindIndex");
			}
			return -1;
		}

		/// <summary>
		/// 在给定向量中查找指定值的索引
		/// </summary>
		static Eigen::VectorXi FindIndex(const Eigen::VectorXd& xvec, const Eigen::VectorXd& x1, bool error = true) {
			Eigen::VectorXi result(x1.size());

			for (int i = 0; i < x1.size(); ++i) {
				result(i) = FindIndex(xvec, x1(i), error);
			}

			return result;
		}
		/// <summary>
   /// 从Eigen向量创建标准向量（用于与STL兼容）
   /// </summary>
		static std::vector<double> ToStdVector(const Eigen::VectorXd& eigenVec) {
			std::vector<double> result(eigenVec.size());
			for (int i = 0; i < eigenVec.size(); ++i) {
				result[i] = eigenVec(i);
			}
			return result;
		}

		static std::vector<int> ToStdVector(const Eigen::VectorXi& eigenVec) {
			std::vector<int> result(eigenVec.size());
			for (int i = 0; i < eigenVec.size(); ++i) {
				result[i] = eigenVec(i);
			}
			return result;
		}

		static std::vector<float> ToStdVector(const Eigen::VectorXf& eigenVec) {
			std::vector<float> result(eigenVec.size());
			for (int i = 0; i < eigenVec.size(); ++i) {
				result[i] = eigenVec(i);
			}
			return result;
		}

		/// <summary>
		/// 从标准向量创建Eigen向量
		/// </summary>
		static Eigen::VectorXd FromStdVector(const std::vector<double>& stdVec) {
			Eigen::VectorXd result(stdVec.size());
			for (size_t i = 0; i < stdVec.size(); ++i) {
				result(i) = stdVec[i];
			}
			return result;
		}

		static Eigen::VectorXi FromStdVector(const std::vector<int>& stdVec) {
			Eigen::VectorXi result(stdVec.size());
			for (size_t i = 0; i < stdVec.size(); ++i) {
				result(i) = stdVec[i];
			}
			return result;
		}

		static Eigen::VectorXf FromStdVector(const std::vector<float>& stdVec) {
			Eigen::VectorXf result(stdVec.size());
			for (size_t i = 0; i < stdVec.size(); ++i) {
				result(i) = stdVec[i];
			}
			return result;
		}

	};
}

#endif // INTERPOLATEHELPER_H
