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




#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <vector>
#include <functional>
#include <algorithm>
#include <random>

#include "../Log/LogHelper.h"
#include "../IO/PhysicalParameters.h"

/// @brief 物理参数类别名，简化对 Qahse::IO::IO::PhysicalParameters 的命名空间访问
using phy = Qahse::IO::IO::PhysicalParameters;

/// @brief 日志辅助类别名，简化对 Qahse::IO::Log::LogHelper 的命名空间访问
using LogHelper = Qahse::IO::Log::LogHelper;

/**
 * @brief Qahse 输入输出数学工具命名空间
 * @details 提供数学运算、随机数生成、向量/矩阵操作等通用工具，
 *          是 Qahse 仿真框架数值计算层的基础支持模块。
 */
namespace Qahse::IO::Math
{
	/**
	 * @brief 数学辅助工具类
	 * @details 集成以下功能模块：
	 *   - **随机数生成**：全局静态接口（共享引擎）及内部有种子类 RandomSeed
	 *   - **序列生成**：Range（整数/浮点等差数列）与 linspace（线性等间距序列）
	 *   - **矩阵/数组初始化**：zeros（支持 2D/3D/4D 常量填充）
	 *   - **向量元素映射**：FunAT / FunATExpr（逐元素函数应用）
	 *   - **快速整数幂**：Pow1（二进制指数算法，O(log n)）
	 *   - **有序向量区间查找**：FindIndex（线性扫描，适用于插值场景）
	 *   - **容器互转**：Eigen ↔ std::vector 双向转换
	 *   - **角度规范化**：Zero2TwoPi（将任意角规范化到 [0, 2π)）
	 *   - **精度四舍五入**：MirrorRound（指定小数位精度控制）
	 *
	 * @note 所有静态随机数接口共享同一个全局梅森旋转引擎（固定种子 1000）。
	 *       若需要可重现的随机序列，请使用内部类 MathHelper::RandomSeed。
	 */
	class MathHelper
	{

	private:
		/// @brief 全局共享梅森旋转伪随机数引擎（64位 MT19937），固定种子 1000。
		///        所有静态 Randomd/Randomi/Randomf 方法均使用此引擎共享状态。
		/// @warning 多线程并发调用静态随机接口时存在竞态条件，建议每线程单独使用 RandomSeed 实例。
		inline static std::mt19937_64 urbg{1000};

		/// @brief 全局双精度均匀分布对象，采样范围 [0.0, 1.0)，与全局 urbg 配合使用
		inline static std::uniform_real_distribution<double> randomdouble{0.0, 1.0};

		/// @brief 全局整数均匀分布对象，采样范围 [0, 100]，与全局 urbg 配合使用
		inline static std::uniform_int_distribution<int> randomint{0, 100};

	public:
		// ======================================================================
		// 内部类：有种子的随机数生成器
		// ======================================================================

		/**
		 * @brief 基于指定种子的随机数生成器（可重现随机序列）
		 * @details 通过构造时传入固定种子，保证相同种子下随机数序列完全可重现。
		 *          每个实例持有独立的引擎 urbg 和分布对象，互不干扰。
		 *
		 *          **支持的功能：**
		 *            - 单个整数 / 浮点数随机采样
		 *            - 一维随机整数/浮点数 std::vector 数组
		 *            - 二维随机整数/浮点数 Eigen 矩阵
		 *            - 指定 [low, high] 范围的单值采样
		 *
		 * @note 该类禁止拷贝构造，防止多个对象共享内部引擎状态导致序列错乱。
		 *
		 * @code
		 * // 示例：使用种子 12345 生成可重现的随机序列
		 * MathHelper::RandomSeed rng(12345);
		 * double  val  = rng.Randomd();            // 单个双精度随机数 ∈ [0, 1)
		 * int     ival = rng.Randomi();             // 单个整数随机数 ∈ [0, INT_MAX]
		 * auto    arr  = rng.Randomd(5, 0.0, 1.0); // 5 个随机 double ∈ [0, 1)
		 * auto    mat  = rng.Randomd(3, 3, 0.0, 10.0); // 3×3 随机 double 矩阵
		 * @endcode
		 */
		class RandomSeed
		{
		private:
			/// @brief 实例专属梅森旋转伪随机数引擎（64位），由构造函数种子初始化
			std::mt19937_64 urbg{1000};

			/// @brief 实例专属双精度均匀分布对象，采样范围 [0.0, 1.0)
			std::uniform_real_distribution<double> randomdouble{0.0, 1.0};

			/// @brief 实例专属整数均匀分布对象，采样范围 [0, 2147483647]（INT_MAX）
			std::uniform_int_distribution<int> randomint{0, 2147483647};

		public:
			/// @brief 禁用拷贝构造，防止多个实例共享同一引擎状态导致随机序列混乱
			RandomSeed(const RandomSeed &) = delete;

			/**
			 * @brief 使用指定整数种子构造随机数生成器
			 * @param[in] seed 随机数种子，相同种子产生完全相同的随机序列（可重现性保证）
			 *
			 * @code
			 * MathHelper::RandomSeed rng1(42);
			 * MathHelper::RandomSeed rng2(42);
			 * // rng1 与 rng2 将产生完全相同的随机序列
			 * assert(rng1.Randomd() == rng2.Randomd()); // true
			 * @endcode
			 */
			explicit RandomSeed(int seed)
			{
				// 将有符号整数种子隐式转换为无符号类型，确保引擎初始化一致性
				const unsigned p = seed;
				std::mt19937_64 urbg1{p};
				urbg = urbg1; // 用新引擎覆盖默认引擎
			}

			/**
			 * @brief 生成单个随机双精度浮点数，范围 [0.0, 1.0)
			 * @return 均匀分布在 [0.0, 1.0) 的 double 值
			 *
			 * @code
			 * MathHelper::RandomSeed rng(123);
			 * double val = rng.Randomd(); // 例如：0.7312456
			 * @endcode
			 */
			double Randomd()
			{
				return randomdouble(urbg); // 使用实例引擎从 [0.0, 1.0) 采样
			}

			/**
			 * @brief 生成单个随机整数，范围 [0, INT_MAX]
			 * @return 均匀分布在 [0, 2147483647] 的 int 值
			 *
			 * @code
			 * MathHelper::RandomSeed rng(456);
			 * int val = rng.Randomi(); // 例如：1847392658
			 * @endcode
			 */
			int Randomi()
			{
				return randomint(urbg); // 使用实例引擎从 [0, INT_MAX] 采样
			}

			/**
			 * @brief 生成指定长度的随机整数数组，元素均匀分布于 [minValue, maxValue]
			 * @param[in] arrayLength 数组长度，必须 >= 0
			 * @param[in] minValue    随机整数下界（包含）
			 * @param[in] maxValue    随机整数上界（包含），必须 >= minValue
			 * @return 长度为 arrayLength 的 std::vector<int>，每元素 ∈ [minValue, maxValue]
			 *
			 * @code
			 * MathHelper::RandomSeed rng(789);
			 * auto arr = rng.Randomi(5, 10, 20);
			 * // 结果示例：[12, 17, 11, 19, 15]
			 * @endcode
			 */
			std::vector<int> Randomi(int arrayLength, int minValue, int maxValue)
			{
				// 构造指定范围的整数均匀分布
				std::uniform_int_distribution<int> randomint{minValue, maxValue};

				// 预分配结果容器
				std::vector<int> randomArray(arrayLength);

				// 逐元素填充随机整数
				for (int i = 0; i < arrayLength; i++)
				{
					randomArray[i] = randomint(urbg); // 在 [minValue, maxValue] 内采样
				}
				return randomArray;
			}

			/**
			 * @brief 生成指定长度的随机整数数组，范围使用实例默认 [0, INT_MAX]
			 * @param[in] arrayLength 数组长度，必须 >= 0
			 * @return 长度为 arrayLength 的 std::vector<int>，每元素 ∈ [0, INT_MAX]
			 *
			 * @code
			 * MathHelper::RandomSeed rng(789);
			 * auto arr = rng.Randomi(5);
			 * // 结果示例：[1073741823, 429496729, ...]
			 * @endcode
			 */
			std::vector<int> Randomi(int arrayLength)
			{
				// 预分配结果容器
				std::vector<int> randomArray(arrayLength);

				// 逐元素填充（使用实例默认分布 [0, INT_MAX]）
				for (int i = 0; i < arrayLength; i++)
				{
					randomArray[i] = randomint(urbg); // 使用默认范围采样
				}
				return randomArray;
			}

			/**
			 * @brief 生成 rows×cols 的随机整数矩阵（Eigen），元素均匀分布于 [minValue, maxValue]
			 * @param[in] rows     矩阵行数，必须 > 0
			 * @param[in] cols     矩阵列数，必须 > 0
			 * @param[in] minValue 随机整数下界（包含）
			 * @param[in] maxValue 随机整数上界（包含），必须 >= minValue
			 * @return 大小为 rows×cols 的 Eigen::MatrixXi，每元素 ∈ [minValue, maxValue]
			 *
			 * @code
			 * MathHelper::RandomSeed rng(147);
			 * Eigen::MatrixXi m = rng.Randomi(2, 3, 5, 15);
			 * // 结果示例：
			 * // [[ 8, 12,  7],
			 * //  [11,  9, 14]]
			 * @endcode
			 */
			Eigen::MatrixXi Randomi(int rows, int cols, int minValue, int maxValue)
			{
				// 构造指定范围的整数均匀分布
				std::uniform_int_distribution<int> randomint{minValue, maxValue};

				// 预分配结果矩阵
				Eigen::MatrixXi randomArray(rows, cols);

				// 按行优先顺序逐元素填充
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						randomArray(i, j) = randomint(urbg); // 在 [minValue, maxValue] 内采样
					}
				}

				return randomArray;
			}

			/**
			 * @brief 生成指定长度的随机双精度浮点数数组，元素均匀分布于 [minValue, maxValue)
			 * @param[in] arrayLength 数组长度，必须 >= 0
			 * @param[in] minValue    随机数下界（包含），默认 0.0
			 * @param[in] maxValue    随机数上界（排除），默认 1.0，必须 > minValue
			 * @return 长度为 arrayLength 的 std::vector<double>，每元素 ∈ [minValue, maxValue)
			 *
			 * @code
			 * MathHelper::RandomSeed rng(321);
			 * auto arr = rng.Randomd(3, 0.0, 10.0);
			 * // 结果示例：[7.23, 2.89, 9.12]
			 * @endcode
			 */
			std::vector<double> Randomd(int arrayLength, double minValue = 0.0, double maxValue = 1.0)
			{
				// 构造指定范围的双精度均匀分布
				std::uniform_real_distribution<double> randomdouble{minValue, maxValue};

				// 预分配结果容器
				std::vector<double> randomArray(arrayLength);

				// 逐元素填充随机浮点数
				for (int i = 0; i < arrayLength; i++)
				{
					randomArray[i] = randomdouble(urbg); // 在 [minValue, maxValue) 内采样
				}

				return randomArray;
			}

			/**
			 * @brief 生成指定长度的随机单精度浮点数数组，元素均匀分布于 [minValue, maxValue)
			 * @param[in] arrayLength 数组长度，必须 >= 0
			 * @param[in] minValue    随机数下界（包含）
			 * @param[in] maxValue    随机数上界（排除），必须 > minValue
			 * @return 长度为 arrayLength 的 std::vector<float>，每元素 ∈ [minValue, maxValue)
			 *
			 * @code
			 * MathHelper::RandomSeed rng(654);
			 * auto arr = rng.Randomf(4, 0.0f, 5.0f);
			 * // 结果示例：[1.23f, 4.56f, 0.78f, 3.21f]
			 * @endcode
			 */
			std::vector<float> Randomf(int arrayLength, float minValue, float maxValue)
			{
				// 构造指定范围的单精度均匀分布
				std::uniform_real_distribution<float> randomdouble{minValue, maxValue};

				// 预分配结果容器
				std::vector<float> randomArray(arrayLength);

				// 逐元素填充随机单精度浮点数
				for (int i = 0; i < arrayLength; i++)
				{
					randomArray[i] = randomdouble(urbg); // 在 [minValue, maxValue) 内采样
				}

				return randomArray;
			}

			/**
			 * @brief 生成指定长度的随机单精度浮点数 Eigen 列向量，元素均匀分布于 [minValue, maxValue)
			 * @param[in] arrayLength 向量长度，必须 > 0
			 * @param[in] minValue    随机数下界（包含）
			 * @param[in] maxValue    随机数上界（排除），必须 > minValue
			 * @param[in] Vector      保留参数（用于与返回 std::vector 的重载区分签名），传 true 即可
			 * @return 长度为 arrayLength 的 Eigen::VectorXf，每元素 ∈ [minValue, maxValue)
			 *
			 * @note Vector 参数目前未参与逻辑，仅用于函数签名区分。
			 *
			 * @code
			 * MathHelper::RandomSeed rng(987);
			 * Eigen::VectorXf v = rng.Randomf(3, 1.0f, 10.0f, true);
			 * // 结果示例：[2.34, 8.67, 5.12]
			 * @endcode
			 */
			Eigen::VectorXf Randomf(int arrayLength, float minValue, float maxValue, bool Vector)
			{
				// 构造指定范围的单精度均匀分布
				std::uniform_real_distribution<float> randomfloat{minValue, maxValue};

				// 预分配 Eigen 列向量
				Eigen::VectorXf randomArray(arrayLength);

				// 逐元素填充随机单精度浮点数
				for (size_t i = 0; i < arrayLength; i++)
				{
					randomArray(i) = randomfloat(urbg); // 在 [minValue, maxValue) 内采样
				}

				return randomArray;
			}

			/**
			 * @brief 生成 rows×cols 的随机双精度浮点数矩阵（Eigen），元素均匀分布于 [minValue, maxValue)
			 * @param[in] rows     矩阵行数，必须 > 0
			 * @param[in] cols     矩阵列数，必须 > 0
			 * @param[in] minValue 随机数下界（包含），默认 0.0
			 * @param[in] maxValue 随机数上界（排除），默认 1.0
			 * @return 大小为 rows×cols 的 Eigen::MatrixXd，每元素 ∈ [minValue, maxValue)
			 *
			 * @code
			 * MathHelper::RandomSeed rng(258);
			 * Eigen::MatrixXd m = rng.Randomd(2, 2, 0.0, 1.0);
			 * // 结果示例：
			 * // [[0.234, 0.789],
			 * //  [0.567, 0.123]]
			 * @endcode
			 */
			Eigen::MatrixXd Randomd(int rows, int cols, double minValue = 0.0, double maxValue = 1.0)
			{
				// 构造指定范围的双精度均匀分布
				std::uniform_real_distribution<double> randomdouble{minValue, maxValue};

				// 预分配结果矩阵
				Eigen::MatrixXd randomArray(rows, cols);

				// 按行优先顺序逐元素填充
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						randomArray(i, j) = randomdouble(urbg); // 在 [minValue, maxValue) 内采样
					}
				}

				return randomArray;
			}

			/**
			 * @brief 生成 rows×cols 的随机单精度浮点数矩阵（Eigen），元素均匀分布于 [minValue, maxValue)
			 * @param[in] rows     矩阵行数，必须 > 0
			 * @param[in] cols     矩阵列数，必须 > 0
			 * @param[in] minValue 随机数下界（包含），默认 0.0f
			 * @param[in] maxValue 随机数上界（排除），默认 1.0f
			 * @return 大小为 rows×cols 的 Eigen::MatrixXf，每元素 ∈ [minValue, maxValue)
			 *
			 * @code
			 * MathHelper::RandomSeed rng(369);
			 * Eigen::MatrixXf m = rng.Randomf(3, 2, 0.0f, 10.0f);
			 * // 结果示例：
			 * // [[2.34f, 7.89f],
			 * //  [5.67f, 1.23f],
			 * //  [9.45f, 3.78f]]
			 * @endcode
			 */
			Eigen::MatrixXf Randomf(int rows, int cols, float minValue = 0.0f, float maxValue = 1.0f)
			{
				// 构造指定范围的单精度均匀分布
				std::uniform_real_distribution<float> randomfloat{minValue, maxValue};

				// 预分配结果矩阵
				Eigen::MatrixXf randomArray(rows, cols);

				// 按行优先顺序逐元素填充
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						randomArray(i, j) = randomfloat(urbg); // 在 [minValue, maxValue) 内采样
					}
				}

				return randomArray;
			}

			/**
			 * @brief 在 [low, high) 范围内生成单个随机双精度浮点数
			 * @param[in] low  下界（包含）
			 * @param[in] high 上界（排除），必须 > low
			 * @return 均匀分布在 [low, high) 的 double 值
			 *
			 * @code
			 * MathHelper::RandomSeed rng(741);
			 * double val = rng.Randomd(5.0, 15.0);
			 * // 结果示例：11.234
			 * @endcode
			 */
			double Randomd(double low, double high)
			{
				// 构造临时范围分布并采样
				std::uniform_real_distribution<double> randomdouble{low, high};
				return randomdouble(urbg);
			}

			/**
			 * @brief 在 [low, high) 范围内生成单个随机单精度浮点数
			 * @param[in] low  下界（包含）
			 * @param[in] high 上界（排除），必须 > low
			 * @return 均匀分布在 [low, high) 的 float 值
			 *
			 * @code
			 * MathHelper::RandomSeed rng(852);
			 * float val = rng.Randomf(1.0f, 5.0f);
			 * // 结果示例：3.14f
			 * @endcode
			 */
			float Randomf(float low, float high)
			{
				// 构造临时范围分布并采样
				std::uniform_real_distribution<float> randomdouble{low, high};
				return randomdouble(urbg);
			}
		}; // end class RandomSeed

		// ======================================================================
		// 全局静态随机数接口（共享引擎，固定种子 1000）
		// ======================================================================

		/**
		 * @brief 生成单个随机双精度浮点数，范围 [0.0, 1.0)（使用全局共享引擎）
		 * @return 均匀分布在 [0.0, 1.0) 的 double 值
		 * @note 全局引擎固定种子为 1000，不同次程序运行生成相同序列（种子不变时）。
		 *
		 * @code
		 * double v = MathHelper::Randomd();
		 * // 结果示例：0.7312456
		 * @endcode
		 */
		static double Randomd()
		{
			return randomdouble(urbg); // 使用全局引擎从 [0.0, 1.0) 采样
		}

		/**
		 * @brief 生成单个随机整数，范围 [0, 100]（使用全局共享引擎）
		 * @param[in] hh 保留参数，当前版本未使用，默认值 0
		 * @return 均匀分布在 [0, 100] 的 int 值
		 *
		 * @code
		 * int v = MathHelper::Randomi();
		 * // 结果示例：18
		 * @endcode
		 */
		static int Randomi(int hh = 0)
		{
			return randomint(urbg); // 使用全局引擎从 [0, 100] 采样
		}

		/**
		 * @brief 生成指定长度的随机整数数组，元素均匀分布于 [minValue, maxValue]（全局引擎）
		 * @param[in] arrayLength 数组长度，必须 >= 0
		 * @param[in] minValue    随机整数下界（包含），默认 0
		 * @param[in] maxValue    随机整数上界（包含），默认 100，必须 >= minValue
		 * @return 长度为 arrayLength 的 std::vector<int>，每元素 ∈ [minValue, maxValue]
		 *
		 * @code
		 * auto arr = MathHelper::Randomi(5, 1, 10);
		 * // 结果示例：[3, 7, 2, 9, 5]
		 * @endcode
		 */
		static std::vector<int> Randomi(int arrayLength, int minValue = 0, int maxValue = 100)
		{
			// 构造局部整数均匀分布，范围 [minValue, maxValue]
			std::uniform_int_distribution<int> randomintt{minValue, maxValue};

			// 预分配结果容器
			std::vector<int> randomArray(arrayLength);

			// 逐元素填充随机整数
			for (int i = 0; i < arrayLength; i++)
			{
				randomArray[i] = randomintt(urbg); // 使用全局引擎采样
			}
			return randomArray;
		}

		/**
		 * @brief 生成指定长度的随机双精度浮点数数组，元素均匀分布于 [minValue, maxValue)（全局引擎）
		 * @param[in] arrayLength 数组长度，必须 >= 0
		 * @param[in] minValue    随机数下界（包含），默认 0.0
		 * @param[in] maxValue    随机数上界（排除），默认 1.0，必须 > minValue
		 * @return 长度为 arrayLength 的 std::vector<double>，每元素 ∈ [minValue, maxValue)
		 *
		 * @code
		 * auto arr = MathHelper::Randomd(3, 0.0, 5.0);
		 * // 结果示例：[1.23, 4.56, 2.89]
		 * @endcode
		 */
		static std::vector<double> Randomd(int arrayLength, double minValue = 0.0, double maxValue = 1.0)
		{
			// 构造局部双精度均匀分布
			std::uniform_real_distribution<double> randomdouble{minValue, maxValue};

			// 预分配结果容器
			std::vector<double> randomArray(arrayLength);

			// 逐元素填充随机浮点数
			for (int i = 0; i < arrayLength; i++)
			{
				randomArray[i] = randomdouble(urbg); // 使用全局引擎采样
			}

			return randomArray;
		}

		/**
		 * @brief 生成指定长度的随机单精度浮点数数组，元素均匀分布于 [minValue, maxValue)（全局引擎）
		 * @param[in] arrayLength 数组长度，必须 >= 0
		 * @param[in] minValue    随机数下界（包含），默认 0.0f
		 * @param[in] maxValue    随机数上界（排除），默认 1.0f，必须 > minValue
		 * @return 长度为 arrayLength 的 std::vector<float>，每元素 ∈ [minValue, maxValue)
		 *
		 * @code
		 * auto arr = MathHelper::Randomf(4, 0.0f, 2.0f);
		 * // 结果示例：[0.31f, 1.87f, 0.54f, 1.22f]
		 * @endcode
		 */
		static std::vector<float> Randomf(int arrayLength, float minValue = 0.0, float maxValue = 1.0)
		{
			// 构造局部单精度均匀分布
			std::uniform_real_distribution<float> randomdouble{minValue, maxValue};

			// 预分配结果容器
			std::vector<float> randomArray(arrayLength);

			// 逐元素填充随机单精度浮点数
			for (int i = 0; i < arrayLength; i++)
			{
				randomArray[i] = randomdouble(urbg); // 使用全局引擎采样
			}

			return randomArray;
		}

		/**
		 * @brief 生成 rows×cols 的随机整数矩阵（Eigen），元素均匀分布于 [minValue, maxValue]（全局引擎）
		 * @param[in] rows     矩阵行数，必须 > 0
		 * @param[in] cols     矩阵列数，必须 > 0
		 * @param[in] minValue 随机整数下界（包含），默认 0
		 * @param[in] maxValue 随机整数上界（包含），默认 100
		 * @return 大小为 rows×cols 的 Eigen::MatrixXi，每元素 ∈ [minValue, maxValue]
		 *
		 * @code
		 * Eigen::MatrixXi m = MathHelper::Randomi(2, 3, 10, 20);
		 * // 结果示例：
		 * // [[12, 17, 11],
		 * //  [19, 15, 13]]
		 * @endcode
		 */
		static Eigen::MatrixXi Randomi(int rows, int cols, int minValue = 0, int maxValue = 100)
		{
			// 构造局部整数均匀分布
			std::uniform_int_distribution<int> randomintt{minValue, maxValue};

			// 预分配结果矩阵
			Eigen::MatrixXi randomArray(rows, cols);

			// 按行优先顺序逐元素填充
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					randomArray(i, j) = randomintt(urbg); // 使用全局引擎采样
				}
			}

			return randomArray;
		}

		/**
		 * @brief 生成 rows×cols 的随机双精度浮点数矩阵（Eigen），元素均匀分布于 [minValue, maxValue)（全局引擎）
		 * @param[in] rows     矩阵行数，必须 > 0
		 * @param[in] cols     矩阵列数，必须 > 0
		 * @param[in] minValue 随机数下界（包含），默认 0.0
		 * @param[in] maxValue 随机数上界（排除），默认 1.0
		 * @return 大小为 rows×cols 的 Eigen::MatrixXd，每元素 ∈ [minValue, maxValue)
		 *
		 * @code
		 * Eigen::MatrixXd m = MathHelper::Randomd(2, 2, 0.0, 10.0);
		 * // 结果示例：
		 * // [[2.34, 7.89],
		 * //  [5.67, 1.23]]
		 * @endcode
		 */
		static Eigen::MatrixXd Randomd(int rows, int cols, double minValue = 0.0, double maxValue = 1)
		{
			// 构造局部双精度均匀分布
			std::uniform_real_distribution<double> randomdouble{minValue, maxValue};

			// 预分配结果矩阵
			Eigen::MatrixXd randomArray(rows, cols);

			// 按行优先顺序逐元素填充
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					randomArray(i, j) = randomdouble(urbg); // 使用全局引擎采样
				}
			}

			return randomArray;
		}

		/**
		 * @brief 生成 rows×cols 的随机单精度浮点数矩阵（Eigen），元素均匀分布于 [minValue, maxValue)（全局引擎）
		 * @param[in] rows     矩阵行数，必须 > 0
		 * @param[in] cols     矩阵列数，必须 > 0
		 * @param[in] minValue 随机数下界（包含），默认 0.0f
		 * @param[in] maxValue 随机数上界（排除），默认 1.0f
		 * @return 大小为 rows×cols 的 Eigen::MatrixXf，每元素 ∈ [minValue, maxValue)
		 *
		 * @code
		 * Eigen::MatrixXf m = MathHelper::Randomf(2, 2, 0.0f, 10.0f);
		 * // 结果示例：
		 * // [[2.34f, 7.89f],
		 * //  [5.67f, 1.23f]]
		 * @endcode
		 */
		static Eigen::MatrixXf Randomf(int rows, int cols, float minValue = 0.0, float maxValue = 1)
		{
			// 构造局部单精度均匀分布
			std::uniform_real_distribution<float> randomdouble{minValue, maxValue};

			// 预分配结果矩阵
			Eigen::MatrixXf randomArray(rows, cols);

			// 按行优先顺序逐元素填充
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					randomArray(i, j) = randomdouble(urbg); // 使用全局引擎采样
				}
			}

			return randomArray;
		}

		/**
		 * @brief 在 [low, high) 范围内生成单个随机双精度浮点数（全局引擎）
		 * @param[in] low  下界（包含）
		 * @param[in] high 上界（排除），必须 > low
		 * @return 均匀分布在 [low, high) 的 double 值
		 *
		 * @code
		 * double v = MathHelper::Randomd(5.0, 15.0);
		 * // 结果示例：11.234
		 * @endcode
		 */
		static double Randomd(double low, double high)
		{
			// 构造临时范围分布并采样
			std::uniform_real_distribution<double> randomdouble{low, high};
			return randomdouble(urbg);
		}

		/**
		 * @brief 在 [low, high) 范围内生成单个随机单精度浮点数（全局引擎）
		 * @param[in] low  下界（包含）
		 * @param[in] high 上界（排除），必须 > low
		 * @return 均匀分布在 [low, high) 的 float 值
		 *
		 * @code
		 * float v = MathHelper::Randomf(0.0f, 5.0f);
		 * // 结果示例：3.14f
		 * @endcode
		 */
		static float Randomf(float low, float high)
		{
			// 构造临时范围分布并采样
			std::uniform_real_distribution<float> randomdouble{low, high};
			return randomdouble(urbg);
		}

		/**
		 * @brief 在 [low, high] 范围内生成单个随机整数（全局引擎）
		 * @param[in] low  下界（包含）
		 * @param[in] high 上界（包含），必须 >= low
		 * @return 均匀分布在 [low, high] 的 int 值
		 *
		 * @code
		 * int v = MathHelper::Randomi(10, 20);
		 * // 结果示例：15
		 * @endcode
		 */
		static int Randomi(int low, int high)
		{
			// 构造临时整数均匀分布并采样
			std::uniform_int_distribution<int> randomintt{low, high};
			return randomintt(urbg);
		}

		// ======================================================================
		// 角度与数值工具
		// ======================================================================

		/**
		 * @brief 将任意弧度角规范化到 [0, 2π) 区间
		 * @details 使用取模运算折叠角度到一个完整圆周范围内，负角度将被转换为等价正角。
		 *          常用于方向角处理、三角函数预处理及几何变换等场景。
		 *
		 *          **算法步骤：**
		 *            1. angle = fmod(angle, 2π)  →  结果 ∈ (-2π, 2π)
		 *            2. 若 < 0，加 2π 转为正角
		 *
		 * @param[in] angle 输入角度（弧度），可以是任意实数（包括负数和大角度）
		 * @return 等价角度（弧度），确保结果 ∈ [0, 2π)
		 *
		 * @code
		 * // 负角转正等价角
		 * double a1 = MathHelper::Zero2TwoPi(-phy::PI / 2);
		 * // 结果：3π/2 ≈ 4.712 rad
		 *
		 * // 超过 2π 的角映射回一圈内
		 * double a2 = MathHelper::Zero2TwoPi(3 * phy::PI);
		 * // 结果：π ≈ 3.142 rad
		 *
		 * // 已在范围内的角保持不变
		 * double a3 = MathHelper::Zero2TwoPi(phy::PI / 4);
		 * // 结果：π/4 ≈ 0.785 rad
		 * @endcode
		 */
		static double Zero2TwoPi(double angle)
		{
			// Step 1：取模将角度折叠到 (-2π, 2π) 范围内
			angle = fmod(angle, 2.0 * phy::PI);

			// Step 2：若结果为负，加 2π 转换为等价正角
			if (angle < 0)
			{
				angle += 2.0 * phy::PI;
			}
			return angle;
		}

		/**
		 * @brief 将双精度浮点数四舍五入到指定小数位数
		 * @details 通过缩放—取整—还原三步实现精确的十进制小数位控制：
		 *            1. factor = 10^decimalPlaces
		 *            2. rounded = round(value × factor)   （标准数学四舍五入）
		 *            3. result  = rounded / factor
		 *
		 * @param[in] value         待四舍五入的双精度浮点数
		 * @param[in] decimalPlaces 保留的小数位数，必须 >= 0（0 表示四舍五入到整数）
		 * @return 四舍五入后的 double 值
		 *
		 * @code
		 * double r1 = MathHelper::MirrorRound(3.14159, 2);
		 * // 结果：3.14
		 *
		 * double r2 = MathHelper::MirrorRound(2.71828, 4);
		 * // 结果：2.7183
		 *
		 * double r3 = MathHelper::MirrorRound(123.456789, 0);
		 * // 结果：123.0
		 * @endcode
		 */
		static double MirrorRound(double value, int decimalPlaces)
		{
			double factor = pow(10, decimalPlaces); // 缩放因子：10^decimalPlaces
			double rounded = round(value * factor); // 缩放后执行标准四舍五入
			double mirrored = rounded / factor;		// 还原到原数量级
			return mirrored;
		}

		// ======================================================================
		// 序列生成：Range / linspace
		// ======================================================================

		/**
		 * @brief 生成从 start 开始、长度为 count 的连续整数序列
		 * @details 类似 Python 的 range(start, start + count)。
		 * @param[in] start 序列起始值
		 * @param[in] count 元素个数，必须 >= 0
		 * @return 长度为 count 的 Eigen::VectorXi，值为 start, start+1, ..., start+count-1
		 *
		 * @code
		 * Eigen::VectorXi v = MathHelper::Range(5, 3);
		 * // 结果：[5, 6, 7]
		 *
		 * Eigen::VectorXi v2 = MathHelper::Range(0, 5);
		 * // 结果：[0, 1, 2, 3, 4]
		 * @endcode
		 */
		static Eigen::VectorXi Range(int start, int count)
		{
			Eigen::VectorXi result(count); // 分配长度为 count 的整数向量
			for (int i = 0; i < count; ++i)
			{
				result(i) = start + i; // 从 start 逐步递增
			}
			return result;
		}

		/**
		 * @brief 生成从 0 到 start-1 的连续索引序列（以 double 存储）
		 * @details 类似 Python 的 list(range(start))，生成 [0.0, 1.0, ..., (start-1).0]。
		 * @param[in] start 序列长度（排除上界），必须 >= 0；为 0 时返回空向量
		 * @return 长度为 start 的 Eigen::VectorXd，值为 0.0, 1.0, ..., (start-1).0
		 *
		 * @code
		 * Eigen::VectorXd v = MathHelper::Range(5);
		 * // 结果：[0.0, 1.0, 2.0, 3.0, 4.0]
		 * @endcode
		 */
		static Eigen::VectorXd Range(int start)
		{
			Eigen::VectorXd result(start); // 分配长度为 start 的 double 向量
			for (int i = 0; i < start; ++i)
			{
				result(i) = static_cast<double>(i); // 将整数索引转换为 double 存储
			}
			return result;
		}

		/**
		 * @brief 生成从 start 到 end（含）、步长为 step 的整数等差数列
		 * @details 类似 Python 的 list(range(start, end+1, step))。
		 *          序列元素数量 = (end - start) / step + 1（向下取整）。
		 * @param[in] start 序列起始值（包含）
		 * @param[in] end   序列终止值（包含，若刚好对齐则包括 end）
		 * @param[in] step  步长，必须 > 0（当前版本不支持负步长）
		 * @return Eigen::VectorXi，包含 start, start+step, start+2*step, ...，最大值 <= end
		 *
		 * @note 若 start > end，count 计算可能为负，行为未定义，请确保 start <= end。
		 *
		 * @code
		 * Eigen::VectorXi v = MathHelper::Range(0, 10, 2);
		 * // 结果：[0, 2, 4, 6, 8, 10]
		 *
		 * Eigen::VectorXi v2 = MathHelper::Range(1, 10, 3);
		 * // 结果：[1, 4, 7, 10]
		 * @endcode
		 */
		static Eigen::VectorXi Range(int start, int end, int step)
		{
			int count = (end - start) / step + 1; // 计算序列元素个数
			Eigen::VectorXi result(count);		  // 预分配结果向量
			int idx = 0;
			for (int i = start; i <= end; i += step) // 按步长逐步递增直到不超过 end
			{
				result(idx++) = i;
			}
			return result;
		}

		/**
		 * @brief 生成从 start 到 end、步长为 step 的双精度浮点数等差数列
		 * @details 适用于连续浮点数网格点生成。序列长度 = round((end-start)/step) + 1。
		 *          步长可以为正（递增）或负（递减）。
		 * @param[in] start 序列起始值
		 * @param[in] end   序列终止值（近似，取决于步长能否整除范围）
		 * @param[in] step  步长，不能为 0；正值生成递增序列，负值生成递减序列
		 * @return Eigen::VectorXd，包含等差分布的浮点数
		 * @throws std::invalid_argument 若 step == 0
		 *
		 * @code
		 * Eigen::VectorXd v = MathHelper::Range(0.0, 10.0, 2.5);
		 * // 结果：[0.0, 2.5, 5.0, 7.5, 10.0]
		 *
		 * Eigen::VectorXd v2 = MathHelper::Range(10.0, 0.0, -2.0);
		 * // 结果：[10.0, 8.0, 6.0, 4.0, 2.0, 0.0]
		 * @endcode
		 */
		static Eigen::VectorXd Range(double start, double end, double step)
		{
			if (step == 0)
			{
				LogHelper::ErrorLog("step不能为0"); // 记录错误日志
				throw std::invalid_argument("step cannot be zero");
			}

			// 计算所需元素个数（通过 round 处理浮点精度误差）
			int length = static_cast<int>(round((end - start) / step)) + 1;
			Eigen::VectorXd result(length);

			// 从 start 按 step 逐步填充
			for (int i = 0; i < length; ++i)
			{
				result(i) = start + i * step;
			}
			return result;
		}

		/**
		 * @brief 生成从 start 到 end、步长为 step 的单精度浮点数等差数列
		 * @details 内部委托 linspace(start, end, length) 实现，以保证端点精度。
		 *          序列长度 = round((end-start)/step) + 1。
		 * @param[in] start 序列起始值
		 * @param[in] end   序列终止值
		 * @param[in] step  步长，不能为 0
		 * @return Eigen::VectorXf，包含等差分布的单精度浮点数
		 * @throws std::invalid_argument 若 step == 0
		 *
		 * @code
		 * Eigen::VectorXf v = MathHelper::Range(0.0f, 5.0f, 1.0f);
		 * // 结果：[0.0, 1.0, 2.0, 3.0, 4.0, 5.0]
		 * @endcode
		 */
		static Eigen::VectorXf Range(float start, float end, float step)
		{
			if (step == 0)
			{
				LogHelper::ErrorLog("step不能为0"); // 记录错误日志
				throw std::invalid_argument("step cannot be zero");
			}

			// 计算元素总数，委托 linspace 确保端点精度
			int length = static_cast<int>(round((end - start) / step)) + 1;
			return linspace(start, end, length);
		}

		/**
		 * @brief 从 start 开始，以 step 为步长，生成 length 个单精度浮点数序列
		 * @details 通过指定步长和点数（而非终止值）构造等差数列，
		 *          适用于只知道起点、步长和点数的场景（如时间步序列生成）。
		 * @param[in] start  序列起始值
		 * @param[in] step   步长，不能为 0；负值生成递减序列
		 * @param[in] length 序列长度（元素个数），必须 >= 0
		 * @return Eigen::VectorXf，长度为 length，result[i] = start + step * i
		 * @throws std::invalid_argument 若 step == 0
		 *
		 * @code
		 * Eigen::VectorXf v = MathHelper::Range(1.0f, 0.5f, 5);
		 * // 结果：[1.0, 1.5, 2.0, 2.5, 3.0]
		 *
		 * Eigen::VectorXf v2 = MathHelper::Range(0.0f, -1.0f, 4);
		 * // 结果：[0.0, -1.0, -2.0, -3.0]
		 * @endcode
		 */
		static Eigen::VectorXf Range(float start, float step, int length)
		{
			if (step == 0)
			{
				LogHelper::ErrorLog("step不能为0"); // 记录错误日志
				throw std::invalid_argument("step cannot be zero");
			}

			Eigen::VectorXf result(length); // 预分配结果向量
			for (int i = 0; i < length; ++i)
			{
				result(i) = start + step * i; // result[i] = start + i × step
			}
			return result;
		}

		/**
		 * @brief 在 [start, end] 区间生成 length 个线性等间距双精度浮点数序列
		 * @details 等价于 MATLAB linspace / numpy.linspace，委托给 MathHelper::linspace。
		 * @param[in] start  序列起始值（包含端点）
		 * @param[in] end    序列终止值（包含端点）
		 * @param[in] length 序列元素个数，建议 > 1
		 * @return Eigen::VectorXd，长度为 length，含两端点
		 *
		 * @code
		 * Eigen::VectorXd v = MathHelper::Range(0.0, 10.0, 5);
		 * // 结果：[0.0, 2.5, 5.0, 7.5, 10.0]
		 * @endcode
		 */
		static Eigen::VectorXd Range(double start, double end, int length)
		{
			return linspace(start, end, length); // 委托 linspace 实现线性等间距
		}

		/**
		 * @brief 在 [start, end] 区间内生成 length 个线性等间距双精度浮点数序列
		 * @details 实现 MATLAB/numpy linspace 功能：
		 *            - step = (end - start) / (length - 1)
		 *            - result[i] = start + i × step
		 *          当 length <= 1 时，退化为只含 start 的单元素向量。
		 * @param[in] start  序列起始值（包含）
		 * @param[in] end    序列终止值（包含）
		 * @param[in] length 序列元素个数；<= 1 时返回 [start]
		 * @return Eigen::VectorXd，长度为 length，第 0 个元素为 start，最后一个为 end
		 *
		 * @code
		 * Eigen::VectorXd v1 = MathHelper::linspace(0.0, 10.0, 5);
		 * // 结果：[0.0, 2.5, 5.0, 7.5, 10.0]
		 *
		 * Eigen::VectorXd v2 = MathHelper::linspace(-5.0, 5.0, 11);
		 * // 结果：[-5.0, -4.0, ..., 4.0, 5.0]
		 *
		 * Eigen::VectorXd v3 = MathHelper::linspace(1.0, 1.0, 3);
		 * // 结果：[1.0, 1.0, 1.0]（起止相同时全为同值）
		 * @endcode
		 */
		static Eigen::VectorXd linspace(double start, double end, int length)
		{
			if (length <= 1)
			{
				// 退化情况：只包含起始值的单元素向量
				Eigen::VectorXd result(1);
				result(0) = start;
				return result;
			}

			Eigen::VectorXd result(length);
			double step = (end - start) / (length - 1); // 均匀步长

			for (int i = 0; i < length; ++i)
			{
				result(i) = start + i * step; // 线性插值填充
			}
			return result;
		}

		/**
		 * @brief 在 [start, end] 区间内生成 length 个线性等间距单精度浮点数序列
		 * @details 单精度版本，逻辑与 double 版本完全相同，精度为 float。
		 *          当 length <= 1 时，退化为只含 start 的单元素向量。
		 * @param[in] start  序列起始值（包含）
		 * @param[in] end    序列终止值（包含）
		 * @param[in] length 序列元素个数；<= 1 时返回 [start]
		 * @return Eigen::VectorXf，长度为 length，第 0 个元素为 start，最后一个为 end
		 *
		 * @code
		 * Eigen::VectorXf v = MathHelper::linspace(0.0f, 5.0f, 6);
		 * // 结果：[0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f]
		 * @endcode
		 */
		static Eigen::VectorXf linspace(float start, float end, int length)
		{
			if (length <= 1)
			{
				// 退化情况：只包含起始值的单元素向量
				Eigen::VectorXf result(1);
				result(0) = start;
				return result;
			}

			Eigen::VectorXf result(length);
			float step = (end - start) / (length - 1); // 均匀步长

			for (int i = 0; i < length; ++i)
			{
				result(i) = start + i * step; // 线性插值填充
			}
			return result;
		}

		// ======================================================================
		// 矩阵/数组初始化：zeros
		// ======================================================================

		/**
		 * @brief 创建 m×n 的二维矩阵，所有元素初始化为指定常量值（模板版本）
		 * @tparam T 矩阵元素类型（如 double、float、int 等）
		 * @param[in] value 所有元素的填充值
		 * @param[in] m     矩阵行数，必须 > 0
		 * @param[in] n     矩阵列数，必须 > 0
		 * @return Eigen::Matrix<T, Dynamic, Dynamic>，大小 m×n，所有元素等于 value
		 *
		 * @code
		 * // 创建 2×3 的全 π 矩阵
		 * auto m = MathHelper::zeros(3.14159, 2, 3);
		 * // m 每个元素均为 3.14159
		 *
		 * // 创建 3×3 的零矩阵（整数类型）
		 * auto iz = MathHelper::zeros(0, 3, 3);
		 * @endcode
		 */
		template <typename T>
		static Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> zeros(T value, int m, int n)
		{
			// 直接调用 Eigen Constant 快速构造常量矩阵
			return Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Constant(m, n, value);
		}

		/**
		 * @brief 创建三维数组（m 个 n×k 矩阵组成的向量），所有元素初始化为指定常量值
		 * @tparam T 元素类型（如 double、float、int 等）
		 * @param[in] input 所有元素的填充值
		 * @param[in] m     外层 vector 长度（第一维大小）
		 * @param[in] n     每个矩阵的行数（第二维大小）
		 * @param[in] k     每个矩阵的列数（第三维大小）
		 * @return std::vector<Eigen::Matrix<T, Dynamic, Dynamic>>，长度为 m，每个矩阵大小 n×k，全部为 input
		 *
		 * @code
		 * // 创建 2 个 3×4 的零矩阵三维数组
		 * auto arr = MathHelper::zeros(0.0, 2, 3, 4);
		 * // arr[0] 和 arr[1] 均为 3×4 的全零 double 矩阵
		 * @endcode
		 */
		template <typename T>
		static std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>> zeros(T input, int m, int n, int k)
		{
			std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>> result(m);
			for (int i = 0; i < m; ++i)
			{
				// 将每个矩阵的所有元素设置为 input
				result[i] = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Constant(n, k, input);
			}
			return result;
		}

		/**
		 * @brief 创建四维数组（m×n 个 k×mm 矩阵的嵌套向量），所有元素初始化为指定常量值
		 * @tparam T 元素类型（如 double、float、int 等）
		 * @param[in] input 所有元素的填充值
		 * @param[in] m     外层 vector 长度（第一维）
		 * @param[in] n     内层 vector 长度（第二维）
		 * @param[in] k     每个矩阵的行数（第三维）
		 * @param[in] mm    每个矩阵的列数（第四维）
		 * @return std::vector<std::vector<Eigen::Matrix<T, Dynamic, Dynamic>>>，结构为 [m][n]，矩阵大小 k×mm，全为 input
		 *
		 * @code
		 * // 创建 2×3 个 4×5 的全 1 单精度矩阵四维数组
		 * auto arr = MathHelper::zeros(1.0f, 2, 3, 4, 5);
		 * // arr[1][2] 是一个 4×5 的全 1 矩阵
		 * @endcode
		 */
		template <typename T>
		static std::vector<std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>> zeros(T input, int m, int n, int k, int mm)
		{
			std::vector<std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>> result(m);
			for (int i = 0; i < m; ++i)
			{
				result[i].resize(n); // 第二维预分配 n 个矩阵槽位
				for (int j = 0; j < n; ++j)
				{
					// 将 result[i][j] 初始化为 k×mm 的常量矩阵
					result[i][j] = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Constant(k, mm, input);
				}
			}
			return result;
		}

		/**
		 * @brief 创建 m×n 的双精度浮点数矩阵，所有元素初始化为指定值（默认创建零矩阵）
		 * @param[in] m     矩阵行数，必须 > 0
		 * @param[in] n     矩阵列数，必须 > 0
		 * @param[in] value 所有元素的填充值，默认 0.0（标准零矩阵）
		 * @return Eigen::MatrixXd，大小 m×n，所有元素等于 value
		 *
		 * @code
		 * Eigen::MatrixXd Z = MathHelper::zeros(3, 4);        // 3×4 零矩阵
		 * Eigen::MatrixXd O = MathHelper::zeros(2, 2, 1.0);   // 2×2 全一矩阵
		 * Eigen::MatrixXd P = MathHelper::zeros(2, 3, 3.14);  // 2×3 全 π 矩阵
		 * @endcode
		 */
		static Eigen::MatrixXd zeros(int m, int n, double value = 0.0)
		{
			return Eigen::MatrixXd::Constant(m, n, value); // 调用 Eigen Constant 快速构造
		}

		// ======================================================================
		// 向量元素映射
		// ======================================================================

		/**
		 * @brief 对 Eigen 列向量的每个元素应用单参数函数，返回映射后的新向量
		 * @details 提供函数式编程风格的逐元素映射，**不修改原向量**，始终返回新向量。
		 *          适用于将 sin、exp、log 等数学函数或自定义 lambda 批量应用到向量元素。
		 * @param[in] f     接受 double 并返回 double 的可调用对象（函数指针、lambda、std::function）
		 * @param[in] input 输入列向量（Eigen::VectorXd）
		 * @return 与 input 等长的 Eigen::VectorXd，result[i] = f(input[i])
		 *
		 * @code
		 * Eigen::VectorXd x(4);
		 * x << 1.0, 2.0, 3.0, 4.0;
		 *
		 * // 1. 计算每个元素的平方
		 * auto sq = MathHelper::FunAT([](double v){ return v * v; }, x);
		 * // sq = [1.0, 4.0, 9.0, 16.0]
		 *
		 * // 2. 批量计算正弦值
		 * auto s = MathHelper::FunAT(std::sin, x);
		 * // s = [sin(1), sin(2), sin(3), sin(4)]
		 *
		 * // 3. 自定义线性变换
		 * auto lin = MathHelper::FunAT([](double v){ return 2*v + 1; }, x);
		 * // lin = [3.0, 5.0, 7.0, 9.0]
		 * @endcode
		 */
		static Eigen::VectorXd FunAT(std::function<double(double)> f, const Eigen::VectorXd &input)
		{
			Eigen::VectorXd result(input.size()); // 分配与 input 等长的结果向量
			for (int i = 0; i < input.size(); ++i)
			{
				result(i) = f(input(i)); // 对第 i 个元素应用函数 f
			}
			return result;
		}

		/**
		 * @brief 使用 Eigen unaryExpr 对列向量每个元素应用函数（高性能模板版本）
		 * @details 利用 Eigen 表达式模板（Lazy Evaluation）机制，
		 *          性能通常优于 FunAT（支持 Eigen 自动向量化 SIMD 优化）。
		 *          函数 f 需满足 Eigen::DenseBase::unaryExpr 的签名约束。
		 * @tparam Func 可调用类型，签名需为 Scalar(Scalar)（接受并返回向量的标量类型）
		 * @param[in] f     应用于每个元素的函数对象
		 * @param[in] input 输入列向量（Eigen::VectorXd）
		 * @return Eigen::VectorXd，result[i] = f(input[i])
		 *
		 * @code
		 * Eigen::VectorXd x(3);
		 * x << 0.0, 1.0, 2.0;
		 *
		 * // 使用 lambda 进行元素级变换
		 * auto r = MathHelper::FunATExpr([](double v){ return v * 2.0; }, x);
		 * // r = [0.0, 2.0, 4.0]
		 * @endcode
		 */
		template <typename Func>
		static Eigen::VectorXd FunATExpr(Func f, const Eigen::VectorXd &input)
		{
			return input.unaryExpr(f); // 利用 Eigen 表达式模板逐元素映射
		}

		// ======================================================================
		// 快速整数幂
		// ======================================================================

		/**
		 * @brief 快速计算双精度浮点数的整数次幂（二进制指数法，O(log n)）
		 * @details 使用"二进制快速幂"（Binary Exponentiation）算法：
		 *          对指数的每个二进制位判断是否需要累乘，依次对底数平方，
		 *          整体复杂度为 O(log |n|)，远优于朴素 O(|n|) 循环。
		 *
		 *          **特殊情况：**
		 *            - n == 0：返回 1.0（任意数的零次幂）
		 *            - n == 1：返回 x（短路优化）
		 *            - n < 0 ：返回 1.0 / x^|n|（负指数取倒数）
		 *
		 * @param[in] x 底数（双精度浮点数）
		 * @param[in] n 指数（整数），可以是正数、负数或零
		 * @return x 的 n 次幂结果（double）
		 *
		 * @note 当前实现固定扫描 31 个二进制位，覆盖全部 int 正整数范围（最大 2^30）。
		 *
		 * @code
		 * double r1 = MathHelper::Pow1(2.0, 10);
		 * // 结果：1024.0（2^10）
		 *
		 * double r2 = MathHelper::Pow1(3.0, -2);
		 * // 结果：0.1111...（1/9 = 1/3^2）
		 *
		 * double r3 = MathHelper::Pow1(5.0, 0);
		 * // 结果：1.0（任意底数的零次幂）
		 *
		 * double r4 = MathHelper::Pow1(7.0, 1);
		 * // 结果：7.0（任意底数的一次幂）
		 * @endcode
		 */
		static double Pow1(double x, int n)
		{
			if (n == 0)
				return 1.0; // x^0 = 1（数学定义，短路返回）
			if (n == 1)
				return x; // x^1 = x（短路优化，避免循环）

			double result = 1.0;
			int absN = abs(n); // 取绝对值，负指数最后取倒数

			// 二进制快速幂：扫描 absN 的每一个二进制位（共 31 位）
			for (int i = 0; i < 31; ++i)
			{
				if (((1 << i) & absN) != 0) // 若第 i 位为 1，将当前 x^(2^i) 累乘入结果
				{
					result *= x;
				}
				x *= x; // x 依次变为 x, x^2, x^4, x^8, x^16, ...
			}

			// 若原指数为负，对结果取倒数
			return n < 0 ? 1.0 / result : result;
		}

		// ======================================================================
		// 有序向量区间查找：FindIndex
		// ======================================================================

		/**
		 * @brief 在有序双精度向量中查找目标值所在插值区间的左端点索引
		 * @details 该函数基于线性扫描，假设输入向量已排好序（升序或降序）。
		 *          返回的索引 i 满足：
		 *            - 升序：xvec[i] <= x < xvec[i+1]（即 x 落入 [i, i+1) 区间）
		 *            - 降序：xvec[i] >= x > xvec[i+1]
		 *
		 *          **算法步骤：**
		 *            1. 检查向量非空
		 *            2. 比较首尾元素判断升序/降序
		 *            3. 线性扫描找到第一个"越过"x 的位置 i
		 *            4. 返回 max(i-1, 0) 作为左端点
		 *            5. 若未找到，记录日志并返回 -1
		 *
		 * @param[in] xvec  已排序的参考向量（升序或降序排列），不能为空
		 * @param[in] x     待查找的目标值
		 * @param[in] error 未找到时的日志级别：true 输出 ErrorLog，false 输出 WarnLog，默认 true
		 * @return 插值区间左端点的零基索引；未找到时返回 -1
		 * @throws std::invalid_argument 若 xvec 为空
		 *
		 * @code
		 * Eigen::VectorXd v(5);
		 * v << 1.0, 3.0, 5.0, 7.0, 9.0;
		 *
		 * int i1 = MathHelper::FindIndex(v, 4.0);
		 * // 4.0 落在 [3.0, 5.0) 区间，返回 1（v[1]=3.0 是左端点）
		 *
		 * int i2 = MathHelper::FindIndex(v, 7.0);
		 * // 恰好等于 v[3]，返回 2（v[2]=5.0 是左端点）
		 *
		 * // 降序向量
		 * Eigen::VectorXd desc(4);
		 * desc << 9.0, 6.0, 3.0, 0.0;
		 * int i3 = MathHelper::FindIndex(desc, 4.0);
		 * // 4.0 落在 (6.0, 3.0] 区间，返回 1（desc[1]=6.0 是左端点）
		 * @endcode
		 */
		static int FindIndex(const Eigen::VectorXd &xvec, double x, bool error = true)
		{
			if (xvec.size() == 0)
			{
				throw std::invalid_argument("数组不能为空"); // 空向量无法进行区间查找
			}

			// 根据首尾元素大小关系判断排序方向
			bool isAscending = xvec(0) < xvec(xvec.size() - 1);

			// 线性扫描：找到第一个"越过" x 的位置
			for (int i = 0; i < xvec.size(); ++i)
			{
				// 升序：找第一个 >= x 的位置；降序：找第一个 <= x 的位置
				if (isAscending ? xvec(i) >= x : xvec(i) <= x)
				{
					return i != 0 ? i - 1 : i; // 返回左端点索引（在边界时返回 0）
				}
			}

			// 未找到合适区间，根据 error 参数记录对应级别日志
			if (error)
			{
				LogHelper::ErrorLog("未找到索引!"); // 严重错误级别
			}
			else
			{
				LogHelper::WarnLog("未找到索引!", " FindIndex"); // 警告级别
			}
			return -1; // 未找到时返回哨兵值 -1
		}

		/**
		 * @brief 在有序整数向量中查找目标值所在插值区间的左端点索引
		 * @details 与 double 版本逻辑完全相同，区别仅在于输入为 Eigen::VectorXi，
		 *          内部比较时将整数元素 static_cast 为 double 后进行比较。
		 * @param[in] xvec  已排序的整数参考向量（升序或降序），不能为空
		 * @param[in] x     待查找的目标值（double，支持与整数向量混合比较）
		 * @param[in] error 未找到时的日志级别：true 输出 ErrorLog，false 输出 WarnLog，默认 true
		 * @return 插值区间左端点的零基索引；未找到时返回 -1
		 * @throws std::invalid_argument 若 xvec 为空
		 *
		 * @code
		 * Eigen::VectorXi v(4);
		 * v << 0, 2, 4, 6;
		 *
		 * int idx = MathHelper::FindIndex(v, 3.0);
		 * // 3.0 落在 [2, 4) 区间，返回 1
		 * @endcode
		 */
		static int FindIndex(const Eigen::VectorXi &xvec, double x, bool error = true)
		{
			if (xvec.size() == 0)
			{
				throw std::invalid_argument("数组不能为空"); // 空向量无法查找
			}

			// 判断整数向量的排序方向
			bool isAscending = xvec(0) < xvec(xvec.size() - 1);

			// 线性扫描，整数元素转换为 double 后与 x 比较
			for (int i = 0; i < xvec.size(); ++i)
			{
				if (isAscending ? static_cast<double>(xvec(i)) >= x
								: static_cast<double>(xvec(i)) <= x)
				{
					return i != 0 ? i - 1 : i; // 返回左端点索引
				}
			}

			// 未找到，记录日志
			if (error)
			{
				LogHelper::ErrorLog("未找到索引!");
			}
			else
			{
				LogHelper::WarnLog("未找到索引!", " FindIndex");
			}
			return -1;
		}

		/**
		 * @brief 批量查找：对 x1 中每个值在 xvec 中查找插值区间左端点索引
		 * @details 逐元素对 x1 中的每个值调用单值版 FindIndex，将所有结果打包成索引向量。
		 *          适用于批量插值前的区间预定位（如同时对多个查询点插值）。
		 * @param[in] xvec  已排序的参考双精度向量（升序或降序）
		 * @param[in] x1    需要查找区间的目标值向量
		 * @param[in] error 同单值版，控制未找到时的日志级别，默认 true
		 * @return Eigen::VectorXi，与 x1 等长，result[i] = FindIndex(xvec, x1[i], error)
		 *
		 * @code
		 * Eigen::VectorXd ref(5);
		 * ref << 0.0, 1.0, 2.0, 3.0, 4.0;
		 *
		 * Eigen::VectorXd q(3);
		 * q << 0.5, 2.3, 3.9;
		 *
		 * Eigen::VectorXi idx = MathHelper::FindIndex(ref, q);
		 * // idx = [0, 2, 3]
		 * // 解释：0.5 ∈ [0,1) → idx=0；2.3 ∈ [2,3) → idx=2；3.9 ∈ [3,4) → idx=3
		 * @endcode
		 */
		static Eigen::VectorXi FindIndex(const Eigen::VectorXd &xvec, const Eigen::VectorXd &x1, bool error = true)
		{
			Eigen::VectorXi result(x1.size()); // 预分配与 x1 等长的整数索引向量

			// 对 x1 中每个元素逐一调用单值版 FindIndex
			for (int i = 0; i < x1.size(); ++i)
			{
				result(i) = FindIndex(xvec, x1(i), error);
			}

			return result;
		}

		// ======================================================================
		// Eigen ↔ STL 容器互转
		// ======================================================================

		/**
		 * @brief 将 Eigen::VectorXd 转换为 std::vector<double>，保持元素顺序
		 * @details 用于需要 STL 容器接口的场景（如 std::sort、范围 for 循环等）。
		 * @param[in] eigenVec 输入的 Eigen 双精度列向量
		 * @return 与 eigenVec 等长的 std::vector<double>，元素按顺序逐一复制
		 *
		 * @code
		 * Eigen::VectorXd v(3);
		 * v << 1.0, 2.0, 3.0;
		 * auto sv = MathHelper::ToStdVector(v);
		 * // sv = {1.0, 2.0, 3.0}（std::vector<double>）
		 * @endcode
		 */
		static std::vector<double> ToStdVector(const Eigen::VectorXd &eigenVec)
		{
			std::vector<double> result(eigenVec.size()); // 预分配等长 STL 容器
			for (int i = 0; i < eigenVec.size(); ++i)
			{
				result[i] = eigenVec(i); // 逐元素复制
			}
			return result;
		}

		/**
		 * @brief 将 Eigen::VectorXi 转换为 std::vector<int>，保持元素顺序
		 * @param[in] eigenVec 输入的 Eigen 整数列向量
		 * @return 与 eigenVec 等长的 std::vector<int>，元素按顺序逐一复制
		 *
		 * @code
		 * Eigen::VectorXi v(3);
		 * v << 1, 2, 3;
		 * auto sv = MathHelper::ToStdVector(v);
		 * // sv = {1, 2, 3}（std::vector<int>）
		 * @endcode
		 */
		static std::vector<int> ToStdVector(const Eigen::VectorXi &eigenVec)
		{
			std::vector<int> result(eigenVec.size()); // 预分配等长 STL 容器
			for (int i = 0; i < eigenVec.size(); ++i)
			{
				result[i] = eigenVec(i); // 逐元素复制
			}
			return result;
		}

		/**
		 * @brief 将 Eigen::VectorXf 转换为 std::vector<float>，保持元素顺序
		 * @param[in] eigenVec 输入的 Eigen 单精度列向量
		 * @return 与 eigenVec 等长的 std::vector<float>，元素按顺序逐一复制
		 *
		 * @code
		 * Eigen::VectorXf v(2);
		 * v << 1.5f, 2.5f;
		 * auto sv = MathHelper::ToStdVector(v);
		 * // sv = {1.5f, 2.5f}（std::vector<float>）
		 * @endcode
		 */
		static std::vector<float> ToStdVector(const Eigen::VectorXf &eigenVec)
		{
			std::vector<float> result(eigenVec.size()); // 预分配等长 STL 容器
			for (int i = 0; i < eigenVec.size(); ++i)
			{
				result[i] = eigenVec(i); // 逐元素复制
			}
			return result;
		}

		/**
		 * @brief 将 std::vector<double> 转换为 Eigen::VectorXd，保持元素顺序
		 * @details 用于将 STL 数据传入需要 Eigen 类型的计算函数。
		 * @param[in] stdVec 输入的 STL 双精度浮点数向量
		 * @return 与 stdVec 等长的 Eigen::VectorXd，元素按顺序逐一复制
		 *
		 * @code
		 * std::vector<double> sv = {1.0, 2.0, 3.0};
		 * Eigen::VectorXd v = MathHelper::FromStdVector(sv);
		 * // v = [1.0, 2.0, 3.0]（Eigen::VectorXd）
		 * @endcode
		 */
		static Eigen::VectorXd FromStdVector(const std::vector<double> &stdVec)
		{
			Eigen::VectorXd result(stdVec.size()); // 预分配等长 Eigen 向量
			for (size_t i = 0; i < stdVec.size(); ++i)
			{
				result(i) = stdVec[i]; // 逐元素复制
			}
			return result;
		}

		/**
		 * @brief 将 std::vector<int> 转换为 Eigen::VectorXi，保持元素顺序
		 * @param[in] stdVec 输入的 STL 整数向量
		 * @return 与 stdVec 等长的 Eigen::VectorXi，元素按顺序逐一复制
		 *
		 * @code
		 * std::vector<int> sv = {1, 2, 3};
		 * Eigen::VectorXi v = MathHelper::FromStdVector(sv);
		 * // v = [1, 2, 3]（Eigen::VectorXi）
		 * @endcode
		 */
		static Eigen::VectorXi FromStdVector(const std::vector<int> &stdVec)
		{
			Eigen::VectorXi result(stdVec.size()); // 预分配等长 Eigen 向量
			for (size_t i = 0; i < stdVec.size(); ++i)
			{
				result(i) = stdVec[i]; // 逐元素复制
			}
			return result;
		}

		/**
		 * @brief 将 std::vector<float> 转换为 Eigen::VectorXf，保持元素顺序
		 * @param[in] stdVec 输入的 STL 单精度浮点数向量
		 * @return 与 stdVec 等长的 Eigen::VectorXf，元素按顺序逐一复制
		 *
		 * @code
		 * std::vector<float> sv = {1.0f, 2.0f, 3.0f};
		 * Eigen::VectorXf v = MathHelper::FromStdVector(sv);
		 * // v = [1.0f, 2.0f, 3.0f]（Eigen::VectorXf）
		 * @endcode
		 */
		static Eigen::VectorXf FromStdVector(const std::vector<float> &stdVec)
		{
			Eigen::VectorXf result(stdVec.size()); // 预分配等长 Eigen 向量
			for (size_t i = 0; i < stdVec.size(); ++i)
			{
				result(i) = stdVec[i]; // 逐元素复制
			}
			return result;
		}
	};
} // namespace Qahse::IO::Math

