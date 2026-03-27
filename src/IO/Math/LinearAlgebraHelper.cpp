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

#include <limits>
#include <sstream>
#include <stdexcept>
#include <complex>
#include <cmath>
#include <execution>

#include "LinearAlgebraHelper.h"

namespace Qahse::IO::Math
{
	// Helper functions for special mathematical functions
	namespace
	{
		// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

		// Simplified implementations for special functions
		// These are basic approximations - for production use, consider proper implementations

		/**
		 * @brief 修正贝塞尔函数 K_nu(z) 的近似计算
		 *
		 * 对修正贝塞尔函数 \f$ K_\nu(z) \f$ 进行简化近似：
		 * - 当 z < 1 时，使用幂律近似 \f$ 2^{\nu-1}\Gamma(\nu)z^{-\nu} \f$；
		 * - 当 z >= 1 时，使用渐近近似 \f$ \sqrt{\pi/(2z)}\,e^{-z} \f$。
		 *
		 * @param nu  贝塞尔函数的阶数 \f$ \nu \f$
		 * @param z   自变量（需满足 z > 0）
		 * @return    近似的 \f$ K_\nu(z) \f$ 值；若 z <= 0 则返回正无穷大
		 *
		 * @note 本实现为快速近似版本，生产环境中建议使用专业数学库（如 Boost.Math）。
		 *
		 * @code
		 * double val = besselk_approx(0.5, 2.0); // 计算 K_0.5(2.0)
		 * @endcode
		 */
		double besselk_approx(double nu, double z)
		{
			if (z <= 0)
				return std::numeric_limits<double>::infinity();

			// Very basic approximation for demonstration
			// In production, use proper Bessel function implementation
			if (z < 1.0)
			{
				return std::pow(2.0, nu - 1.0) * std::tgamma(nu) * std::pow(z, -nu);
			}
			else
			{
				return std::sqrt(M_PI / (2.0 * z)) * std::exp(-z);
			}
		}

		/**
		 * @brief Gamma 函数的委托实现，直接调用 std::tgamma
		 *
		 * @param x  输入参数（x > 0 时结果有意义）
		 * @return   \f$ \Gamma(x) \f$ 的值
		 *
		 * @code
		 * double g = gamma_approx(5.0); // 返回 24.0 (即 4!)
		 * @endcode
		 */
		double gamma_approx(double x)
		{
			return std::tgamma(x);
		}
	}

#pragma region Array Expansion and Processing

	/**
	 * @brief 将数组扩展到目标长度，并尽可能保留原始值在对应计算位置的精确性
	 *
	 * @details
	 * 在新数组中，原始数组的每个元素会映射到一个按比例计算的整数索引位置，
	 * 其他位置通过线性插值填充。`low` 和 `up` 参数（若非 NaN）将作为边界值
	 * 插入到原始数组的两端参与扩展运算。
	 *
	 * @param originalArray  原始数组（如风速剖面等离散序列）
	 * @param targetLength   扩展后目标数组长度
	 * @param low            数组左边界扩充值，NaN 表示不添加左边界
	 * @param up             数组右边界扩充值，NaN 表示不添加右边界
	 * @return               扩展后的 VectorXd，长度为 `targetLength`
	 *
	 * @note 当 low 或 up 为 NaN 时，边界不添加额外点；否则工作数组在两端各添加一个边界值。
	 *
	 * @code
	 * VectorXd arr(3); arr << 1.0, 3.0, 5.0;
	 * VectorXd expanded = LinearAlgebraHelper::ExpandArrayPreserveExact(arr, 9,
	 *     std::numeric_limits<double>::quiet_NaN(),
	 *     std::numeric_limits<double>::quiet_NaN());
	 * // expanded 长度为 9，原始值在对应的整数索引处保持精确
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::ExpandArrayPreserveExact(const VectorXd &originalArray, int targetLength,
																				double low, double up)
	{

		if (originalArray.size() == 0)
		{
			throw std::runtime_error("原始数组不能为空");
		}

		if (targetLength <= originalArray.size())
		{
			throw std::runtime_error("目标长度必须大于原数组长度");
		}

		// Handle NaN values for bounds
		if (std::isnan(low))
		{
			low = originalArray.minCoeff();
		}
		if (std::isnan(up))
		{
			up = originalArray.maxCoeff();
		}

		// Find boundary indices
		int lowIdx = -1, upIdx = -1;
		for (int i = 0; i < originalArray.size(); ++i)
		{
			if (std::abs(originalArray[i] - low) < 1e-10)
			{
				lowIdx = i;
			}
			if (std::abs(originalArray[i] - up) < 1e-10)
			{
				upIdx = i;
			}
		}

		VectorXd workingArray = originalArray;

		// Add boundary points if not found
		if (lowIdx == -1)
		{
			VectorXd temp(originalArray.size() + 1);
			temp[0] = low;
			temp.segment(1, originalArray.size()) = originalArray;
			workingArray = temp;
			lowIdx = 0;
			if (upIdx >= 0)
				upIdx++;
		}

		if (upIdx == -1)
		{
			VectorXd temp(workingArray.size() + 1);
			temp.head(workingArray.size()) = workingArray;
			temp[workingArray.size()] = up;
			workingArray = temp;
			upIdx = workingArray.size() - 1;
		}

		// Extract the working segment
		VectorXd leftPart = workingArray.head(lowIdx);
		VectorXd rightPart = workingArray.tail(workingArray.size() - upIdx - 1);
		VectorXd centerPart = workingArray.segment(lowIdx, upIdx - lowIdx + 1);

		int adjustedTargetLength = targetLength - leftPart.size() - rightPart.size();
		VectorXd result(adjustedTargetLength);

		// Calculate exact indices for original array points in new array
		std::vector<int> exactIndices(centerPart.size());
		for (int i = 0; i < centerPart.size(); ++i)
		{
			exactIndices[i] = static_cast<int>(std::round(static_cast<double>(i) * (adjustedTargetLength - 1) / (centerPart.size() - 1)));
		}

		// Set exact points
		for (int i = 0; i < centerPart.size(); ++i)
		{
			result[exactIndices[i]] = centerPart[i];
		}

		// Interpolate other positions
		for (int i = 0; i < adjustedTargetLength; ++i)
		{
			bool isExactPoint = false;
			for (int exactIdx : exactIndices)
			{
				if (i == exactIdx)
				{
					isExactPoint = true;
					break;
				}
			}

			if (!isExactPoint)
			{
				// Find left and right exact points
				int leftIndex = -1, rightIndex = -1;

				for (int j = 0; j < exactIndices.size() - 1; ++j)
				{
					if (i > exactIndices[j] && i < exactIndices[j + 1])
					{
						leftIndex = j;
						rightIndex = j + 1;
						break;
					}
				}

				if (leftIndex >= 0 && rightIndex >= 0)
				{
					// Linear interpolation
					double leftPos = exactIndices[leftIndex];
					double rightPos = exactIndices[rightIndex];
					double t = (i - leftPos) / (rightPos - leftPos);
					result[i] = centerPart[leftIndex] + t * (centerPart[rightIndex] - centerPart[leftIndex]);
				}
				else if (i < exactIndices[0])
				{
					// Left boundary extrapolation
					result[i] = centerPart[0];
				}
				else if (i > exactIndices[exactIndices.size() - 1])
				{
					// Right boundary extrapolation
					result[i] = centerPart[centerPart.size() - 1];
				}
			}
		}

		// Concatenate all parts
		VectorXd finalResult(targetLength);
		finalResult.head(leftPart.size()) = leftPart;
		finalResult.segment(leftPart.size(), result.size()) = result;
		finalResult.tail(rightPart.size()) = rightPart;

		return finalResult;
	}

	/**
	 * @brief 将值插入到已排序向量的适当位置，并保持升序顺序
	 *
	 * @details
	 * 若 sort=true，则先对输入向量排序；然后利用二分查找找到 value 的插入位置。
	 *
	 * @param v1    原始序列
	 * @param value 要插入的标量值
	 * @param sort  true = 插入前先对 v1 进行排序
	 * @return      插入后长度为 (v1.size()+1) 的新向量
	 *
	 * @throws std::runtime_error 当输入向量为空时抛出
	 *
	 * @code
	 * VectorXd v(3); v << 1, 3, 5;
	 * VectorXd r = LinearAlgebraHelper::AddSortedValue(v, 4.0, false); // [1,3,4,5]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::AddSortedValue(const VectorXd &v1, double value, bool sort)
	{
		VectorXd v = v1;

		if (sort)
		{
			std::vector<double> temp(v.data(), v.data() + v.size());
			std::sort(temp.begin(), temp.end());
			v = Eigen::Map<VectorXd>(temp.data(), temp.size());
		}

		if (v.size() == 0)
		{
			throw std::runtime_error("The input vector must not be null or empty.");
		}

		VectorXd result(v.size() + 1);
		int index = FindSortedFirst(v, value);

		result.head(index) = v.head(index);
		result[index] = value;
		result.tail(result.size() - index - 1) = v.tail(v.size() - index);

		return result;
	}

#pragma endregion

#pragma region Cumulative Operations

	/**
	 * @brief 计算 VectorXd 的累积和
	 *
	 * @details
	 * result[0] = v[0]；result[i] = result[i-1] + v[i]（i >= 1）。
	 *
	 * @param v  输入向量
	 * @return   长度相同的累积和向量
	 *
	 * @code
	 * VectorXd v(4); v << 1, 2, 3, 4;
	 * VectorXd cs = LinearAlgebraHelper::Cumsum(v); // [1, 3, 6, 10]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Cumsum(const VectorXd &v)
	{
		VectorXd result(v.size());
		result[0] = v[0];
		for (int i = 1; i < v.size(); ++i)
		{
			result[i] = result[i - 1] + v[i];
		}
		return result;
	}

	/**
	 * @brief 计算 std::vector<double> 的累积和
	 *
	 * @param v  输入数组
	 * @return   长度相同的累积和数组
	 *
	 * @code
	 * std::vector<double> v = {1, 2, 3, 4};
	 * auto cs = LinearAlgebraHelper::Cumsum(v); // {1, 3, 6, 10}
	 * @endcode
	 */
	std::vector<double> LinearAlgebraHelper::Cumsum(const std::vector<double> &v)
	{
		std::vector<double> result(v.size());
		result[0] = v[0];
		for (size_t i = 1; i < v.size(); ++i)
		{
			result[i] = result[i - 1] + v[i];
		}
		return result;
	}

	/**
	 * @brief 计算 VectorXd 的反向累积和（将累积和还原为差分）
	 *
	 * @details
	 * result[0] = v[0]；result[i] = v[i] - v[i-1]（i >= 1）。
	 * 即将累积和序列还原为离散差分序列。
	 *
	 * @param v  输入向量
	 * @return   差分向量，和输入长度相同
	 *
	 * @code
	 * VectorXd cs(4); cs << 1, 3, 6, 10;
	 * VectorXd d = LinearAlgebraHelper::ReCumsum(cs); // [1, 2, 3, 4]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::ReCumsum(const VectorXd &v)
	{
		VectorXd result(v.size());
		result[0] = v[0];
		for (int i = 1; i < v.size(); ++i)
		{
			result[i] = v[i] - v[i - 1];
		}
		return result;
	}

	/**
	 * @brief 计算 std::vector<double> 的反向累积和（差分）
	 *
	 * @param v  输入数组
	 * @return   差分数组
	 *
	 * @code
	 * std::vector<double> cs = {1, 3, 6, 10};
	 * auto d = LinearAlgebraHelper::ReCumsum(cs); // {1, 2, 3, 4}
	 * @endcode
	 */
	std::vector<double> LinearAlgebraHelper::ReCumsum(const std::vector<double> &v)
	{
		std::vector<double> result(v.size());
		result[0] = v[0];
		for (size_t i = 1; i < v.size(); ++i)
		{
			result[i] = v[i] - v[i - 1];
		}
		return result;
	}

#pragma endregion

#pragma region Search and Find Operations

	/**
	 * @brief 查找向量中最接近指定目标值的元素的索引和值（double 版本）
	 *
	 * @details
	 * 线性扫描，时间复杂度 O(n)。
	 *
	 * @param v      输入向量（非空）
	 * @param target 目标值
	 * @return       std::tuple<索引, 最近值>
	 *
	 * @throws std::runtime_error 当输入向量为空时抛出
	 *
	 * @code
	 * std::vector<double> v = {1.0, 3.0, 5.0, 7.0};
	 * auto [idx, val] = LinearAlgebraHelper::FindClosestIndexAndValue<double>(v, 4.2);
	 * // idx = 1, val = 3.0 (最接近 4.2 的元素为 5.0？不，3|4.2-3|=1.2 < |4.2-5|=0.8, 所以实际为 5.0)
	 * @endcode
	 */
	template <>
	std::tuple<int, double> LinearAlgebraHelper::FindClosestIndexAndValue<double>(const std::vector<double> &v, double target)
	{
		if (v.empty())
		{
			throw std::runtime_error("The input vector must not be null or empty.");
		}

		int closestIndex = 0;
		double closestValue = static_cast<double>(v[0]);
		double minDiff = std::abs(closestValue - target);

		for (size_t i = 1; i < v.size(); ++i)
		{
			double value = static_cast<double>(v[i]);
			double diff = std::abs(value - target);
			if (diff < minDiff)
			{
				minDiff = diff;
				closestIndex = static_cast<int>(i);
				closestValue = value;
			}
		}

		return std::make_tuple(closestIndex, closestValue);
	}

	/**
	 * @brief 查找向量中最接近指定目标值的元素的索引和值（float 版本）
	 *
	 * @param v      输入单精度向量
	 * @param target 目标值
	 * @return       std::tuple<索引, 最近单精度值>
	 *
	 * @throws std::runtime_error 当输入向量为空时抛出
	 *
	 * @code
	 * std::vector<float> v = {1.0f, 3.0f, 5.0f};
	 * auto [idx, val] = LinearAlgebraHelper::FindClosestIndexAndValue<float>(v, 3.8f);
	 * @endcode
	 */
	template <>
	std::tuple<int, float> LinearAlgebraHelper::FindClosestIndexAndValue<float>(const std::vector<float> &v, float target)
	{
		if (v.empty())
		{
			throw std::runtime_error("The input vector must not be null or empty.");
		}

		int closestIndex = 0;
		float closestValue = static_cast<float>(v[0]);
		float minDiff = std::abs(closestValue - target);

		for (size_t i = 1; i < v.size(); ++i)
		{
			float value = static_cast<float>(v[i]);
			float diff = std::abs(value - target);
			if (diff < minDiff)
			{
				minDiff = diff;
				closestIndex = static_cast<int>(i);
				closestValue = value;
			}
		}

		return std::make_tuple(closestIndex, closestValue);
	}

	template <>
	std::tuple<int, int> LinearAlgebraHelper::FindClosestIndexAndValue<int>(const std::vector<int> &v, int target)
	{
		if (v.empty())
		{
			throw std::runtime_error("The input vector must not be null or empty.");
		}

		int closestIndex = 0;
		int closestValue = static_cast<int>(v[0]);
		int minDiff = std::abs(closestValue - target);

		for (size_t i = 1; i < v.size(); ++i)
		{
			int value = static_cast<int>(v[i]);
			int diff = std::abs(value - target);
			if (diff < minDiff)
			{
				minDiff = diff;
				closestIndex = static_cast<int>(i);
				closestValue = value;
			}
		}

		return std::make_tuple(closestIndex, closestValue);
	}

	/**
	 * @brief 将容器中所有元素按升序排列
	 *
	 * @tparam T  元素类型，需支持 operator< 比较
	 * @param values  输入序列
	 * @return        排序后的新序列（不修改原始输入）
	 *
	 * @code
	 * std::vector<double> v = {5, 2, 8, 1};
	 * auto sv = LinearAlgebraHelper::Sort(v); // {1, 2, 5, 8}
	 * @endcode
	 */
	template <typename T>
	std::vector<T> LinearAlgebraHelper::Sort(const std::vector<T> &values)
	{
		std::vector<T> result = values;
		std::sort(result.begin(), result.end());
		return result;
	}

	/**
	 * @brief 在已排序向量中二分查找第一个 >= target 的元素索引
	 *
	 * @param v      已按升序排列的 VectorXd
	 * @param target 目标值
	 * @return       第一个满足 v[i] >= target 的索引；
	 *              若所有元素都 < target 则返回 v.size()
	 *
	 * @code
	 * VectorXd v(5); v << 1, 3, 5, 7, 9;
	 * int idx = LinearAlgebraHelper::FindSortedFirst(v, 6); // 3 (即 v[3]=7 是第一个 >= 6 的元素)
	 * @endcode
	 */
	int LinearAlgebraHelper::FindSortedFirst(const VectorXd &v, double target)
	{
		int left = 0;
		int right = static_cast<int>(v.size()) - 1;

		while (left <= right)
		{
			int mid = (left + right) / 2;
			if (v[mid] < target)
			{
				left = mid + 1;
			}
			else
			{
				right = mid - 1;
			}
		}

		return left; // 返回第一个大于等于 target 的索引
	}

#pragma endregion

#pragma region Comparison and Equality

	/**
	 * @brief 判断两个实数是否在指定容差内近似相等
	 *
	 * @param a        第一个实数
	 * @param b        第二个实数
	 * @param epsilon  容差阈值（默认建议使用 1e-10）
	 * @return         |a - b| < epsilon 则返回 true
	 *
	 * @code
	 * bool eq = LinearAlgebraHelper::EqualRealNos(1.0000001, 1.0, 1e-5); // true
	 * @endcode
	 */
	bool LinearAlgebraHelper::EqualRealNos(double a, double b, double epsilon)
	{
		return std::abs(a - b) < epsilon;
	}

#pragma endregion

#pragma region Matrix Information

	/**
	 * @brief 获取矩阵的行数与列数
	 *
	 * @param matrix  输入双精度矩阵
	 * @return        std::tuple<行数, 列数>
	 *
	 * @code
	 * MatrixXd A(3, 4); // 3行 4列
	 * auto [rows, cols] = LinearAlgebraHelper::Size(A); // (3, 4)
	 * @endcode
	 */
	std::tuple<int, int> LinearAlgebraHelper::Size(const MatrixXd &matrix)
	{
		return std::make_tuple(static_cast<int>(matrix.rows()), static_cast<int>(matrix.cols()));
	}

	/**
	 * @brief 获取二维 std::vector<vector<double>> 的行数与列数
	 *
	 * @param matrix  二维数组（外层 = 行，内层 = 列）
	 * @return        std::tuple<行数, 列数>；若为空返回 (0, 0)
	 *
	 * @code
	 * std::vector<std::vector<double>> m(3, std::vector<double>(5));
	 * auto [r, c] = LinearAlgebraHelper::Size(m); // (3, 5)
	 * @endcode
	 */
	std::tuple<int, int> LinearAlgebraHelper::Size(const std::vector<std::vector<double>> &matrix)
	{
		if (matrix.empty())
			return std::make_tuple(0, 0);
		return std::make_tuple(static_cast<int>(matrix.size()), static_cast<int>(matrix[0].size()));
	}

	/**
	 * @brief 获取三维 std::vector 的各维大小
	 *
	 * @param matrix  三维数组
	 * @return        std::tuple<dim0, dim1, dim2>
	 *
	 * @code
	 * std::vector<std::vector<std::vector<double>>> m(2, {{}, {}}); // 示意
	 * auto [d0, d1, d2] = LinearAlgebraHelper::Size(m);
	 * @endcode
	 */
	std::tuple<int, int, int> LinearAlgebraHelper::Size(const std::vector<std::vector<std::vector<double>>> &matrix)
	{
		if (matrix.empty())
			return std::make_tuple(0, 0, 0);
		if (matrix[0].empty())
			return std::make_tuple(static_cast<int>(matrix.size()), 0, 0);
		return std::make_tuple(static_cast<int>(matrix.size()),
							   static_cast<int>(matrix[0].size()),
							   static_cast<int>(matrix[0][0].size()));
	}

	/**
	 * @brief 获取矩阵指定维度的大小（与 MATLAB size(A,1)/size(A,2) 完全一致）
	 *
	 * @param matrix  输入双精度矩阵
	 * @param a       1 = 行数，2 = 列数
	 * @return        对应维度的大小（int）
	 *
	 * @throws std::runtime_error a 不为 1 或 2 时抛出
	 *
	 * @code
	 * MatrixXd A(3, 5);
	 * int r = LinearAlgebraHelper::Size(A, 1); // 3
	 * int c = LinearAlgebraHelper::Size(A, 2); // 5
	 * @endcode
	 */
	int LinearAlgebraHelper::Size(const MatrixXd &matrix, int a)
	{
		if (a == 1)
		{
			return static_cast<int>(matrix.rows());
		}
		else if (a == 2)
		{
			return static_cast<int>(matrix.cols());
		}
		else
		{
			throw std::runtime_error("错误使用size a=1 or a=2");
		}
	}

#pragma endregion

#pragma region Matrix Computations

	/**
	 * @brief 基于一维坐标向量生成二维坐标网格
	 *
	 * @details
	 * 类似 MATLAB `meshgrid(x, y)` 函数。当 `f=true` 时，将 x 方向倒序排列。
	 *
	 * @param x  x 方向坐标向量（长度 m）
	 * @param y  y 方向坐标向量（长度 n）
	 * @param f  true = 将 x 按逆序排列；false = 按正序排列
	 * @return   std::tuple<xx(m×n), yy(m×n)>，其中 xx(i,j) = x[i](或逆序), yy(i,j) = y[j]
	 *
	 * @code
	 * VectorXd x(3); x << 1, 2, 3;
	 * VectorXd y(2); y << 10, 20;
	 * auto [xx, yy] = LinearAlgebraHelper::meshgrid(x, y, false);
	 * // xx = [[1,1],[2,2],[3,3]], yy = [[10,20],[10,20],[10,20]]
	 * @endcode
	 */
	std::tuple<LinearAlgebraHelper::MatrixXd, LinearAlgebraHelper::MatrixXd>
	LinearAlgebraHelper::meshgrid(const VectorXd &x, const VectorXd &y, bool f)
	{

		MatrixXd xx, yy;

		if (f)
		{
			xx = MatrixXd(x.size(), y.size());
			yy = MatrixXd(x.size(), y.size());
			for (int i = 0; i < x.size(); ++i)
			{
				for (int j = 0; j < y.size(); ++j)
				{
					xx(i, j) = x[x.size() - 1 - i];
					yy(i, j) = y[j];
				}
			}
		}
		else
		{
			xx = MatrixXd(x.size(), y.size());
			yy = MatrixXd(x.size(), y.size());
			for (int i = 0; i < x.size(); ++i)
			{
				for (int j = 0; j < y.size(); ++j)
				{
					xx(i, j) = x[i];
					yy(i, j) = y[j];
				}
			}
		}

		return std::make_tuple(xx, yy);
	}

	/**
	 * @brief 执行带缩放的矩阵乘法：\f$ C = \alpha (A \cdot B) + \beta C \f$
	 *
	 * @param[in,out] C    输入/输出矩阵，操作后被更新
	 * @param A             左矩阵
	 * @param B             右矩阵
	 * @param alpha         A*B 的缩放因子
	 * @param beta          C 的缩放因子
	 * @return              更新后的 C
	 *
	 * @code
	 * MatrixXd A(2,2); A << 1,2,3,4;
	 * MatrixXd B(2,2); B << 1,0,0,1;
	 * MatrixXd C = MatrixXd::Zero(2,2);
	 * LinearAlgebraHelper::Mul(C, A, B, 2.0, 0.0); // C = 2*A
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Mul(MatrixXd &C, const MatrixXd &A, const MatrixXd &B, double alpha, double beta)
	{
		C = alpha * (A * B) + beta * C;
		return C;
	}

	/**
	 * @brief 执行矩阵乘法：\f$ C = A \cdot B \f$
	 *
	 * @param[in,out] C    输入/输出矩阵
	 * @param A             左矩阵
	 * @param B             右矩阵
	 * @return              更新后的 C
	 *
	 * @code
	 * MatrixXd A(2,2); A << 1,0,0,1;
	 * MatrixXd B(2,2); B << 4,3,2,1;
	 * MatrixXd C;
	 * LinearAlgebraHelper::Mul(C, A, B); // C = A*B = B
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Mul(MatrixXd &C, const MatrixXd &A, const MatrixXd &B)
	{
		C = A * B;
		return C;
	}

	/**
	 * @brief 执行对称正定矩阵的 LDL^T 分解（双精度）
	 *
	 * @details
	 * 分解兹拉斯基 关系：\f$ A = L D L^T \f$，其中:
	 * - L 为单位下三角矩阵（对角线全为 1）
	 * - D 为对角矩阵
	 *
	 * @param A  输入对称正定矩阵（n×n 双精度）
	 * @return   std::tuple<L(MatrixXd), D(MatrixXd)>
	 *
	 * @note 此实现为普通串行版本；对于大型矩阵可使用 PLDL 并行版本。
	 *
	 * @code
	 * MatrixXd A(3,3); A << 4,2,2, 2,5,3, 2,3,6;
	 * auto [L, D] = LinearAlgebraHelper::LDL(A);
	 * // 验证：L * D * L.transpose() 应恒等于 A
	 * @endcode
	 */
	std::tuple<LinearAlgebraHelper::MatrixXd, LinearAlgebraHelper::MatrixXd>
	LinearAlgebraHelper::LDL(const MatrixXd &A)
	{

		int n = static_cast<int>(A.rows());
		MatrixXd L = Eye(n);
		MatrixXd D = MatrixXd::Zero(n, n); // Use Eigen directly instead of zeros function

		for (int j = 0; j < n; ++j)
		{
			double sumD = 0.0;
			for (int m = 0; m < j; ++m)
			{
				sumD += L(j, m) * L(j, m) * D(m, m);
			}
			D(j, j) = A(j, j) - sumD;

			for (int i = j + 1; i < n; ++i)
			{
				double sumL = 0.0;
				for (int m = 0; m < j; ++m)
				{
					sumL += L(i, m) * L(j, m) * D(m, m);
				}
				L(i, j) = (A(i, j) - sumL) / D(j, j);
			}
		}

		return std::make_tuple(L, D);
	}

	/**
	 * @brief 执行对称矩阵的 LDL^T 分解（单精度版本）
	 *
	 * @param A  输入对称正定矩阵（n×n 单精度）
	 * @return   std::tuple<L(MatrixXf), D(MatrixXf)>
	 *
	 * @code
	 * MatrixXf A(2,2); A << 4.0f, 2.0f, 2.0f, 5.0f;
	 * auto [L, D] = LinearAlgebraHelper::LDL(A);
	 * @endcode
	 */
	std::tuple<LinearAlgebraHelper::MatrixXf, LinearAlgebraHelper::MatrixXf>
	LinearAlgebraHelper::LDL(const MatrixXf &A)
	{

		int n = static_cast<int>(A.rows());
		MatrixXf L = fEye(n);
		MatrixXf D = MatrixXf::Zero(n, n); // Use Eigen directly instead of fzeros function

		for (int j = 0; j < n; ++j)
		{
			float sumD = 0.0f;
			for (int m = 0; m < j; ++m)
			{
				sumD += L(j, m) * L(j, m) * D(m, m);
			}
			D(j, j) = A(j, j) - sumD;

			for (int i = j + 1; i < n; ++i)
			{
				float sumL = 0.0f;
				for (int m = 0; m < j; ++m)
				{
					sumL += L(i, m) * L(j, m) * D(m, m);
				}
				L(i, j) = (A(i, j) - sumL) / D(j, j);
			}
		}

		return std::make_tuple(L, D);
	}

	/**
	 * @brief 执行对称矩阵的 PLDL^T 分解（并行版本，单精度）
	 *
	 * @details
	 * 与 LDL 相比，本函数采用外层串行循环、内层并行化结构，适合大矩阵加速。
	 * 分解关系：\f$ A = L D L^T \f$，其中 L 为单位下三角矩阵，D 为对角矩阵。
	 *
	 * @param A  输入的对称正定矩阵（单精度 MatrixXf）
	 * @return   std::tuple<L, D>，L 为单位下三角矩阵，D 为对角矩阵
	 *
	 * @note 当前实现使用普通串行循环以保证兼容性，生产环境中可启用 OpenMP 并行。
	 *
	 * @code
	 * MatrixXf A(3,3); A << 4,2,2, 2,5,3, 2,3,6;
	 * auto [L, D] = LinearAlgebraHelper::PLDL(A);
	 * @endcode
	 */
	std::tuple<LinearAlgebraHelper::MatrixXf, LinearAlgebraHelper::MatrixXf>
	LinearAlgebraHelper::PLDL(const MatrixXf &A)
	{

		int n = static_cast<int>(A.rows());
		MatrixXf L = MatrixXf::Zero(n, n); // Use Eigen directly
		MatrixXf D = MatrixXf::Zero(n, n); // Use Eigen directly

		// 并行计算 LDLT 分解
		for (int k = 0; k < n; ++k)
		{
			// 计算 D 的对角元素
			D(k, k) = A(k, k);
			for (int j = 0; j < k; ++j)
			{
				D(k, k) -= L(k, j) * L(k, j) * D(j, j);
			}

			// 填充 L 的下三角部分
			L(k, k) = 1.0f; // L 的对角元素设置为 1

			// Simple loop instead of OpenMP for compatibility
			for (int i = k + 1; i < n; ++i)
			{
				L(i, k) = A(i, k);
				for (int j = 0; j < k; ++j)
				{
					L(i, k) -= L(i, j) * L(k, j) * D(j, j);
				}
				L(i, k) /= D(k, k); // 更新 L 的元素
			}
		}

		return std::make_tuple(L, D);
	}

#pragma endregion

#pragma region File I/O Operations

	/**
	 * @brief 从文本文件读取矩阵数据（支持空格、逗号、Tab 分隔）
	 *
	 * @details
	 * 逐行读取文件，对每行字符串按 ' '、','、'\t' 三种分隔符进行分割，
	 * 将有效浮点数收集到行向量；所有行向量组合为 MatrixXd 返回。
	 * 若文件无法打开或内容为空，会抛出 std::runtime_error。
	 *
	 * @param filePath  文本文件路径（绝对路径或相对可执行文件目录的路径）
	 * @return          从文件解析得到的 MatrixXd 矩阵
	 *
	 * @throws std::runtime_error 文件打开失败或文件内容无有效数据时抛出
	 *
	 * @code
	 * MatrixXd mat = LinearAlgebraHelper::ReadMatrixFromFile("e:/data/matrix.txt");
	 * std::cout << mat.rows() << " x " << mat.cols() << std::endl;
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::ReadMatrixFromFile(const std::string &filePath)
	{
		std::ifstream file(filePath);
		if (!file.is_open())
		{
			throw std::runtime_error("Cannot open file: " + filePath);
		}

		std::vector<std::vector<double>> data;
		std::string line;

		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::vector<double> row;
			std::string token;

			while (std::getline(iss, token, ' ') || std::getline(iss, token, ',') || std::getline(iss, token, '\t'))
			{
				if (!token.empty())
				{
					try
					{
						row.push_back(std::stod(token));
					}
					catch (const std::exception &)
					{
						// Skip invalid tokens
					}
				}
			}

			if (!row.empty())
			{
				data.push_back(row);
			}
		}

		if (data.empty())
		{
			throw std::runtime_error("No valid data found in file");
		}

		int rows = static_cast<int>(data.size());
		int cols = static_cast<int>(data[0].size());
		MatrixXd matrix(rows, cols);

		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols && j < static_cast<int>(data[i].size()); ++j)
			{
				matrix(i, j) = data[i][j];
			}
		}

		return matrix;
	}

	/**
	 * @brief 从 MATLAB 风格的文本文件读取复数矩阵
	 *
	 * @details
	 * 按 Tab 分隔符逐列解析复数（格式如 "a+bi" 或 "a-bi"）。
	 * 当前版本为占位实现（TODO），解析器尚未完整处理，返回元素均为零值。
	 *
	 * @param filePath  MATLAB 风格复数矩阵文本文件路径
	 * @return          解析得到的 Eigen::MatrixXcd 复数矩阵
	 *
	 * @throws std::runtime_error 文件打开失败或内容为空时抛出
	 *
	 * @warning 当前复数解析部分为 TODO 占位实现，实际元素值全为零，待完善。
	 *
	 * @code
	 * Eigen::MatrixXcd cmat = LinearAlgebraHelper::ReadMatlabMatrixFromFile("e:/data/complex.txt");
	 * @endcode
	 */
	Eigen::MatrixXcd LinearAlgebraHelper::ReadMatlabMatrixFromFile(const std::string &filePath)
	{
		std::ifstream file(filePath);
		if (!file.is_open())
		{
			throw std::runtime_error("Cannot open file: " + filePath);
		}

		std::vector<std::vector<std::complex<double>>> data;
		std::string line;

		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::vector<std::complex<double>> row;
			std::string token;

			while (std::getline(iss, token, '\t'))
			{
				if (!token.empty())
				{
					// Parse complex number in format "a+bi" or "a-bi"
					try
					{
						// This is a simplified parser - would need more robust implementation
						std::complex<double> value(0, 0);
						// TODO: Implement proper complex number parsing
						row.push_back(value);
					}
					catch (const std::exception &)
					{
						// Skip invalid tokens
					}
				}
			}

			if (!row.empty())
			{
				data.push_back(row);
			}
		}

		if (data.empty())
		{
			throw std::runtime_error("No valid data found in file");
		}

		int rows = static_cast<int>(data.size());
		int cols = static_cast<int>(data[0].size());
		Eigen::MatrixXcd matrix(rows, cols);

		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols && j < static_cast<int>(data[i].size()); ++j)
			{
				matrix(i, j) = data[i][j];
			}
		}

		return matrix;
	}

#pragma endregion

#pragma region Eigenvalue and Eigenvector Operations

	/**
	 * @brief 计算方阵的特征值与特征向量（双精度标准特征值问题）
	 *
	 * @details
	 * 使用 Eigen::EigenSolver 计算非对称方阵的实部特征值与特征向量。
	 * 若 `sort=true`，则按特征值升序对特征向量列重新排列。
	 *
	 * @param M     输入的方阵（n×n 双精度）
	 * @param sort  是否按特征值升序排序（默认 false）
	 * @return      std::tuple<eigenValues(VectorXd), eigenVectors(MatrixXd)>，
	 *              eigenValues 为特征值列向量，eigenVectors 每列对应一个特征向量
	 *
	 * @note 仅提取实部，若矩阵存在复数特征值则结果为实部近似。
	 *
	 * @code
	 * MatrixXd M(2,2); M << 3,1, 1,3;
	 * auto [vals, vecs] = LinearAlgebraHelper::Eig(M, true);
	 * // vals = [2, 4], vecs 对应升序排列的特征向量
	 * @endcode
	 */
	std::tuple<LinearAlgebraHelper::VectorXd, LinearAlgebraHelper::MatrixXd>
	LinearAlgebraHelper::Eig(const MatrixXd &M, bool sort)
	{

		Eigen::EigenSolver<MatrixXd> solver(M);
		VectorXd eigenValues = solver.eigenvalues().real();
		MatrixXd eigenVectors = solver.eigenvectors().real();

		if (!sort)
		{
			return std::make_tuple(eigenValues, eigenVectors);
		}

		// Sort eigenvalues and eigenvectors
		std::vector<std::pair<double, int>> eigenPairs;
		for (int i = 0; i < eigenValues.size(); ++i)
		{
			eigenPairs.push_back(std::make_pair(eigenValues[i], i));
		}

		std::sort(eigenPairs.begin(), eigenPairs.end());

		VectorXd sortedEigenValues(eigenValues.size());
		MatrixXd sortedEigenVectors(eigenVectors.rows(), eigenVectors.cols());

		for (size_t i = 0; i < eigenPairs.size(); ++i)
		{
			sortedEigenValues[i] = eigenPairs[i].first;
			sortedEigenVectors.col(i) = eigenVectors.col(eigenPairs[i].second);
		}

		return std::make_tuple(sortedEigenValues, sortedEigenVectors);
	}

	/**
	 * @brief 计算方阵的特征值与特征向量（单精度标准特征值问题）
	 *
	 * @details
	 * 使用 Eigen::EigenSolver 计算单精度方阵的实部特征值与特征向量。
	 * 若 `sort=true`，则按特征值升序对特征向量列重新排列。
	 *
	 * @param M     输入的方阵（n×n 单精度）
	 * @param sort  是否按特征值升序排序
	 * @return      std::tuple<eigenValues(VectorXf), eigenVectors(MatrixXf)>
	 *
	 * @code
	 * MatrixXf M(2,2); M << 3.0f,1.0f, 1.0f,3.0f;
	 * auto [vals, vecs] = LinearAlgebraHelper::Eig(M, false);
	 * @endcode
	 */
	std::tuple<LinearAlgebraHelper::VectorXf, LinearAlgebraHelper::MatrixXf>
	LinearAlgebraHelper::Eig(const MatrixXf &M, bool sort)
	{

		Eigen::EigenSolver<MatrixXf> solver(M);
		VectorXf eigenValues = solver.eigenvalues().real();
		MatrixXf eigenVectors = solver.eigenvectors().real();

		if (!sort)
		{
			return std::make_tuple(eigenValues, eigenVectors);
		}

		// Sort eigenvalues and eigenvectors
		std::vector<std::pair<float, int>> eigenPairs;
		for (int i = 0; i < eigenValues.size(); ++i)
		{
			eigenPairs.push_back(std::make_pair(eigenValues[i], i));
		}

		std::sort(eigenPairs.begin(), eigenPairs.end());

		VectorXf sortedEigenValues(eigenValues.size());
		MatrixXf sortedEigenVectors(eigenVectors.rows(), eigenVectors.cols());

		for (int i = 0; i < eigenPairs.size(); ++i)
		{
			sortedEigenValues[i] = eigenPairs[i].first;
			sortedEigenVectors.col(i) = eigenVectors.col(eigenPairs[i].second);
		}

		return std::make_tuple(sortedEigenValues, sortedEigenVectors);
	}

	/**
	 * @brief 计算广义特征值问题 \f$ K v = \lambda M v \f$ 的特征值与特征向量
	 *
	 * @details
	 * 使用 Eigen::GeneralizedEigenSolver 求解广义特征值问题，常见于结构动力学的固有频率计算。
	 * 若 `sort=true`，按特征值升序对结果重新排列。
	 *
	 * @param K     刚度矩阵（或左侧矩阵），n×n 双精度
	 * @param M     质量矩阵（或右侧矩阵），n×n 双精度
	 * @param sort  是否按特征值升序排序
	 * @return      std::tuple<eigenValues(VectorXd), eigenVectors(MatrixXd)>
	 *
	 * @note 广义特征值的物理意义：\f$ \omega^2 = \lambda \f$ 为固有频率的平方。
	 *
	 * @code
	 * MatrixXd K(2,2); K << 4,0, 0,9;
	 * MatrixXd M(2,2); M << 1,0, 0,1;
	 * auto [omega2, modes] = LinearAlgebraHelper::Eig(K, M, true);
	 * @endcode
	 */
	std::tuple<LinearAlgebraHelper::VectorXd, LinearAlgebraHelper::MatrixXd>
	LinearAlgebraHelper::Eig(const MatrixXd &K, const MatrixXd &M, bool sort)
	{

		Eigen::GeneralizedEigenSolver<MatrixXd> solver(K, M);
		VectorXd eigenValues = solver.eigenvalues().real();
		MatrixXd eigenVectors = solver.eigenvectors().real();

		if (!sort)
		{
			return std::make_tuple(eigenValues, eigenVectors);
		}

		// Sort eigenvalues and eigenvectors
		std::vector<std::pair<double, int>> eigenPairs;
		for (int i = 0; i < eigenValues.size(); ++i)
		{
			eigenPairs.push_back(std::make_pair(eigenValues[i], i));
		}

		std::sort(eigenPairs.begin(), eigenPairs.end());

		VectorXd sortedEigenValues(eigenValues.size());
		MatrixXd sortedEigenVectors(eigenVectors.rows(), eigenVectors.cols());

		for (size_t i = 0; i < eigenPairs.size(); ++i)
		{
			sortedEigenValues[i] = eigenPairs[i].first;
			sortedEigenVectors.col(i) = eigenVectors.col(eigenPairs[i].second);
		}

		return std::make_tuple(sortedEigenValues, sortedEigenVectors);
	}

#pragma endregion

#pragma region Vector Products

	/**
	 * @brief 计算两个列向量的外积（头尾积）
	 *
	 * @param a  左列向量（n 维）
	 * @param b  右列向量（m 维）
	 * @return   n×m 双精度矩阵，即 \f$ a b^T \f$
	 *
	 * @code
	 * VectorXd a(2); a << 1, 2;
	 * VectorXd b(3); b << 3, 4, 5;
	 * MatrixXd outer = LinearAlgebraHelper::Outer(a, b); // 2x3 矩阵
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Outer(const VectorXd &a, const VectorXd &b)
	{
		return a * b.transpose();
	}

	/**
	 * @brief 计算两个向量的内积（点乘）
	 *
	 * @param a  左向量
	 * @param b  右向量（与 a 长度相同）
	 * @return   标量内积结果 \f$ a \cdot b \f$
	 *
	 * @code
	 * VectorXd a(3); a << 1, 2, 3;
	 * VectorXd b(3); b << 4, 5, 6;
	 * double dot = LinearAlgebraHelper::Inter(a, b); // 32.0
	 * @endcode
	 */
	double LinearAlgebraHelper::Inter(const VectorXd &a, const VectorXd &b)
	{
		return a.dot(b);
	}

#pragma endregion

#pragma region Vector Concatenation

	/**
	 * @brief 将两个 std::vector<double> 水平拼接为单个 VectorXd
	 *
	 * @param a  第一个序列
	 * @param b  第二个序列
	 * @return   长度为 (a.size() + b.size()) 的 VectorXd
	 *
	 * @code
	 * std::vector<double> a = {1, 2};
	 * std::vector<double> b = {3, 4, 5};
	 * VectorXd v = LinearAlgebraHelper::Hact(a, b); // [1, 2, 3, 4, 5]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(const std::vector<double> &a, const std::vector<double> &b)
	{
		int al = static_cast<int>(a.size());
		int bl = static_cast<int>(b.size());
		VectorXd result(al + bl);

		for (int i = 0; i < al; ++i)
		{
			result[i] = a[i];
		}
		for (int i = 0; i < bl; ++i)
		{
			result[al + i] = b[i];
		}

		return result;
	}

	/**
	 * @brief 将两个整数向量水平拼接为单个 VectorXi
	 *
	 * @param a  第一个整数向量
	 * @param b  第二个整数向量
	 * @return   拼接后的 VectorXi
	 *
	 * @code
	 * VectorXi a(2); a << 1, 2;
	 * VectorXi b(2); b << 3, 4;
	 * VectorXi v = LinearAlgebraHelper::Hact(a, b); // [1, 2, 3, 4]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXi LinearAlgebraHelper::Hact(const VectorXi &a, const VectorXi &b)
	{
		int al = static_cast<int>(a.size());
		int bl = static_cast<int>(b.size());
		VectorXi result(al + bl);

		result.head(al) = a;
		result.tail(bl) = b;

		return result;
	}

	/**
	 * @brief 将标量和向量拼接，标量放在头部
	 *
	 * @param a  头部标量
	 * @param b  连接的向量
	 * @return   长度为 (1 + b.size()) 的 VectorXd
	 *
	 * @code
	 * VectorXd b(3); b << 2, 3, 4;
	 * VectorXd v = LinearAlgebraHelper::Hact(1.0, b); // [1, 2, 3, 4]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(double a, const VectorXd &b)
	{
		int bl = static_cast<int>(b.size());
		VectorXd result(1 + bl);

		result[0] = a;
		result.tail(bl) = b;

		return result;
	}

	/**
	 * @brief 将向量和标量拼接，标量放在尾部
	 *
	 * @param a  头部向量
	 * @param b  尾部标量
	 * @return   长度为 (a.size() + 1) 的 VectorXd
	 *
	 * @code
	 * VectorXd a(3); a << 1, 2, 3;
	 * VectorXd v = LinearAlgebraHelper::Hact(a, 4.0); // [1, 2, 3, 4]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(const VectorXd &a, double b)
	{
		int al = static_cast<int>(a.size());
		VectorXd result(al + 1);

		result.head(al) = a;
		result[al] = b;

		return result;
	}

	/**
	 * @brief 将多个矩阵按列水平拼接为单个矩阵
	 *
	 * @details
	 * 要求所有矩阵具有相同的行数。列数可以不同。
	 *
	 * @param input  矩阵列表
	 * @return       水平拼接后的矩阵，列数为所有矩阵列数之和
	 *
	 * @code
	 * MatrixXd A(2,2); A << 1,2, 3,4;
	 * MatrixXd B(2,3); B << 5,6,7, 8,9,10;
	 * MatrixXd C = LinearAlgebraHelper::Hact({A, B}); // 2×5 矩阵
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Hact(const std::vector<MatrixXd> &input)
	{
		if (input.empty())
		{
			return MatrixXd();
		}

		int totalCols = 0;
		int rows = static_cast<int>(input[0].rows());

		for (const auto &mat : input)
		{
			totalCols += static_cast<int>(mat.cols());
		}

		MatrixXd result(rows, totalCols);
		int colOffset = 0;

		for (const auto &mat : input)
		{
			result.middleCols(colOffset, mat.cols()) = mat;
			colOffset += static_cast<int>(mat.cols());
		}

		return result;
	}

	/**
	 * @brief 将多个 VectorXd 水平拼接为单个长向量
	 *
	 * @param input  向量列表
	 * @return       拼接后的 VectorXd，长度为所有向量长度之和
	 *
	 * @code
	 * VectorXd a(2); a << 1, 2;
	 * VectorXd b(3); b << 3, 4, 5;
	 * VectorXd v = LinearAlgebraHelper::Hact({a, b}); // [1,2,3,4,5]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(const std::vector<VectorXd> &input)
	{
		int totalSize = 0;
		for (const auto &vec : input)
		{
			totalSize += static_cast<int>(vec.size());
		}

		VectorXd result(totalSize);
		int offset = 0;

		for (const auto &vec : input)
		{
			result.segment(offset, vec.size()) = vec;
			offset += static_cast<int>(vec.size());
		}

		return result;
	}

	/**
	 * @brief 将 Vector3d 数组按 [x0,y0,z0, x1,y1,z1, ...] 顺序展平为 VectorXd
	 *
	 * @param input  Vector3d 对象的列表
	 * @return       长度为 (input.size() * 3) 的 VectorXd
	 *
	 * @code
	 * std::vector<Vector3d> pts = { {1,2,3}, {4,5,6} };
	 * VectorXd v = LinearAlgebraHelper::Hact(pts); // [1,2,3,4,5,6]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(const std::vector<Vector3d> &input)
	{
		VectorXd result(static_cast<int>(input.size()) * 3);

		for (size_t i = 0; i < input.size(); ++i)
		{
			result[i * 3 + 0] = input[i].x();
			result[i * 3 + 1] = input[i].y();
			result[i * 3 + 2] = input[i].z();
		}

		return result;
	}

	/**
	 * @brief 将多个 std::vector<double> 水平拼接为单个 VectorXd
	 *
	 * @param input  一维数组的列表，每个元素为一个 subarray
	 * @return       拼接后的 VectorXd
	 *
	 * @code
	 * std::vector<std::vector<double>> data = {{1,2},{3,4,5}};
	 * VectorXd v = LinearAlgebraHelper::Hact(data); // [1,2,3,4,5]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(const std::vector<std::vector<double>> &input)
	{
		int totalSize = 0;
		for (const auto &vec : input)
		{
			totalSize += static_cast<int>(vec.size());
		}

		VectorXd result(totalSize);
		int offset = 0;

		for (const auto &vec : input)
		{
			for (size_t i = 0; i < vec.size(); ++i)
			{
				result[offset + i] = vec[i];
			}
			offset += static_cast<int>(vec.size());
		}

		return result;
	}

	/**
	 * @brief 将多个 VectorXd 射5向排列，每列为一个向量，形成矩阵
	 *
	 * @param a  向量列表，每个向量长度必须相同
	 * @return   rows×cols 矩阵，其中 rows=a[0].size()，cols=a.size()
	 *
	 * @note 每个输入向量作为一列放入输出矩阵。
	 *
	 * @code
	 * VectorXd v1(2); v1 << 1, 2;
	 * VectorXd v2(2); v2 << 3, 4;
	 * MatrixXd M = LinearAlgebraHelper::Vact({v1, v2}); // 2×2 矩阵
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Vact(const std::vector<VectorXd> &a)
	{
		if (a.empty())
		{
			return MatrixXd();
		}

		int rows = static_cast<int>(a[0].size());
		int cols = static_cast<int>(a.size());
		MatrixXd result(rows, cols);

		for (int i = 0; i < cols; ++i)
		{
			result.col(i) = a[i];
		}

		return result;
	}

	/**
	 * @brief 将多个矩阵按行垂直拼接
	 *
	 * @param a  矩阵列表，所有矩阵列数必须相同
	 * @return   行数为所有矩阵行数之和的新矩阵
	 *
	 * @code
	 * MatrixXd A(2,3); A << 1,2,3, 4,5,6;
	 * MatrixXd B(1,3); B << 7,8,9;
	 * MatrixXd C = LinearAlgebraHelper::Vact({A, B}); // 3×3 矩阵
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Vact(const std::vector<MatrixXd> &a)
	{
		if (a.empty())
		{
			return MatrixXd();
		}

		int totalRows = 0;
		int cols = static_cast<int>(a[0].cols());

		for (const auto &mat : a)
		{
			totalRows += static_cast<int>(mat.rows());
		}

		MatrixXd result(totalRows, cols);
		int rowOffset = 0;

		for (const auto &mat : a)
		{
			result.middleRows(rowOffset, mat.rows()) = mat;
			rowOffset += static_cast<int>(mat.rows());
		}

		return result;
	}

	/**
	 * @brief 将多个 std::vector<double> 按列翁并排列为矩阵
	 *
	 * @details
	 * 每个输入子数组作为一列放入输出矩阵。
	 * 所有子数组长度应相同（或使用 rows=a[0].size()）。
	 *
	 * @param a  二维序列列表，a[i] 为第 i 列的数据
	 * @return   rows×cols 矩阵
	 *
	 * @code
	 * std::vector<std::vector<double>> cols = {{1.0,2.0},{3.0,4.0}};
	 * MatrixXd M = LinearAlgebraHelper::Vact(cols); // 2×2 矩阵
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Vact(const std::vector<std::vector<double>> &a)
	{
		if (a.empty())
		{
			return MatrixXd();
		}

		int rows = static_cast<int>(a[0].size());
		int cols = static_cast<int>(a.size());
		MatrixXd result(rows, cols);

		for (int i = 0; i < cols; ++i)
		{
			for (int j = 0; j < rows; ++j)
			{
				result(j, i) = a[i][j];
			}
		}

		return result;
	}

	/**
	 * @brief 将两个 VectorXd 拼接为矩阵
	 *
	 * @details
	 * 当 `row=true` 时，输出为 2×n 矩阵（两个向量各占一行）；
	 * 当 `row=false` 时，输出为 n×2 矩阵（两个向量各占一列）。
	 * 最要求两个向量长度相同，否则抛出异常。
	 *
	 * @param a    第一个向量
	 * @param b    第二个向量（与 a 长度相同）
	 * @param row  true = 行排列（2×n）；false = 列排列（n×2）
	 * @return     拼接后的矩阵
	 *
	 * @throws std::runtime_error 当 a.size() != b.size() 时抛出
	 *
	 * @code
	 * VectorXd a(3); a << 1,2,3;
	 * VectorXd b(3); b << 4,5,6;
	 * MatrixXd M = LinearAlgebraHelper::Vact(a, b, false); // 3×2 矩阵
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Vact(const VectorXd &a, const VectorXd &b, bool row)
	{
		int al = static_cast<int>(a.size());
		int bl = static_cast<int>(b.size());

		if (al != bl)
		{
			throw std::runtime_error("a 和 b 的长度不一样，无法 vcat！");
		}

		if (row)
		{
			MatrixXd result(2, al);
			result.row(0) = a.transpose();
			result.row(1) = b.transpose();
			return result;
		}
		else
		{
			MatrixXd result(al, 2);
			result.col(0) = a;
			result.col(1) = b;
			return result;
		}
	}

#pragma endregion

#pragma region Special Matrices

	/**
	 * @brief 由三维向量生成其反对称（歪对称）矩阵
	 *
	 * @details
	 * 对于向量形式的三维向量 \f$ a = [a_1, a_2, a_3]^T \f$，生成：
	 * \f[ \text{skew}(a) = \begin{bmatrix} 0 & -a_3 & a_2 \\ a_3 & 0 & -a_1 \\ -a_2 & a_1 & 0 \end{bmatrix} \f]
	 *
	 * @param a  三维 VectorXd
	 * @return   3×3 反对称矩阵
	 *
	 * @throws std::runtime_error 当 a.size() != 3 时抛出
	 *
	 * @code
	 * VectorXd omega(3); omega << 0, 0, 1;
	 * MatrixXd S = LinearAlgebraHelper::SkewMatrix(omega);
	 * // S = [[0,-1,0],[1,0,0],[0,0,0]]
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::SkewMatrix(const VectorXd &a)
	{
		if (a.size() != 3)
		{
			throw std::runtime_error("Vector must have exactly 3 elements for skew matrix");
		}

		MatrixXd skew(3, 3);
		skew << 0, -a[2], a[1],
			a[2], 0, -a[0],
			-a[1], a[0], 0;

		return skew;
	}

	/**
	 * @brief 由 Vector3d 生成其反对称矩阵
	 *
	 * @param a  三维 Eigen::Vector3d
	 * @return   3×3 反对称矩阵
	 *
	 * @code
	 * Vector3d w(0, 0, 1);
	 * MatrixXd S = LinearAlgebraHelper::SkewMatrix(w);
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::SkewMatrix(const Vector3d &a)
	{
		MatrixXd skew(3, 3);
		skew << 0, -a.z(), a.y(),
			a.z(), 0, -a.x(),
			-a.y(), a.x(), 0;

		return skew;
	}

	/**
	 * @brief 计算矩阵的迹（对角线元素之和）
	 *
	 * @param a  输入方阵
	 * @return   \f$ \text{tr}(a) = \sum_i a_{ii} \f$
	 *
	 * @code
	 * MatrixXd A(2,2); A << 1,2, 3,4;
	 * double t = LinearAlgebraHelper::Trace(A); // 5.0
	 * @endcode
	 */
	double LinearAlgebraHelper::Trace(const MatrixXd &a)
	{
		return a.trace();
	}

	/**
	 * @brief 计算矩阵的行列式
	 *
	 * @param a  输入方阵
	 * @return   \f$ \det(a) \f$
	 *
	 * @code
	 * MatrixXd A(2,2); A << 1,2, 3,4;
	 * double d = LinearAlgebraHelper::Det(A); // -2.0
	 * @endcode
	 */
	double LinearAlgebraHelper::Det(const MatrixXd &a)
	{
		return a.determinant();
	}

	/**
	 * @brief 对矩阵进行 QR 正交化，返回正交矩阵
	 *
	 * @details
	 * 内部使用 Eigen 的 HouseholderQR 分解，提取 Q 矩阵。
	 *
	 * @param a  输入矩阵（m×n）
	 * @return   正交矩阵 Q（m×m）
	 *
	 * @code
	 * MatrixXd A(3,2); A << 1,2, 3,4, 5,6;
	 * MatrixXd Q = LinearAlgebraHelper::Orthogonal(A);
	 * // Q^T Q = I
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Orthogonal(const MatrixXd &a)
	{
		Eigen::HouseholderQR<MatrixXd> qr(a);
		return qr.householderQ();
	}

#pragma endregion

#pragma region Difference Operations

	/**
	 * @brief 计算向量中相邂元素之间的离散差分
	 *
	 * @details
	 * 返回长度为 (n-1) 的向量，其中 result[i] = input[i+1] - input[i]。
	 *
	 * @param input  输入向量（长度至少为 2）
	 * @return       差分向量，长度为 (input.size() - 1)
	 *
	 * @throws std::runtime_error 当向量长度 < 2 时抛出
	 *
	 * @code
	 * VectorXd v(4); v << 1, 3, 6, 10;
	 * VectorXd d = LinearAlgebraHelper::Diff(v); // [2, 3, 4]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Diff(const VectorXd &input)
	{
		if (input.size() < 2)
		{
			throw std::runtime_error("Input vector must contain at least two elements");
		}

		VectorXd result(input.size() - 1);
		for (int i = 0; i < result.size(); ++i)
		{
			result[i] = input[i + 1] - input[i];
		}

		return result;
	}

#pragma endregion

#pragma region Norm Calculations

	/**
	 * @brief 计算向量的指定范数
	 *
	 * @param vector  输入向量
	 * @param a       范数类型：1 = L1范数，2 = L2范数（欧直里得），3 = 无穷范数
	 * @return        对应范数的标量值
	 *
	 * @throws std::runtime_error 不支持的范数类型时抛出
	 *
	 * @code
	 * VectorXd v(3); v << 3, 4, 0;
	 * double n2 = LinearAlgebraHelper::Norm(v, 2); // 5.0
	 * double n1 = LinearAlgebraHelper::Norm(v, 1); // 7.0
	 * @endcode
	 */
	double LinearAlgebraHelper::Norm(const VectorXd &vector, int a)
	{
		if (a == 1)
		{
			return vector.lpNorm<1>();
		}
		else if (a == 2)
		{
			return vector.norm();
		}
		else if (a == 3)
		{
			return vector.lpNorm<Eigen::Infinity>();
		}
		else
		{
			throw std::runtime_error("Unsupported norm type");
		}
	}

	/**
	 * @brief 计算 Vector3d 的指定范数
	 *
	 * @param vector  输入 Eigen::Vector3d
	 * @param a       范数类型：1 = L1，2 = L2，3 = 无穷
	 * @return        对应范数
	 *
	 * @throws std::runtime_error 不支持的范数类型时抛出
	 *
	 * @code
	 * Vector3d v(3, 4, 0);
	 * double n2 = LinearAlgebraHelper::Norm(v, 2); // 5.0
	 * @endcode
	 */
	double LinearAlgebraHelper::Norm(const Vector3d &vector, int a)
	{
		if (a == 1)
		{
			return vector.lpNorm<1>();
		}
		else if (a == 2)
		{
			return vector.norm();
		}
		else if (a == 3)
		{
			return vector.lpNorm<Eigen::Infinity>();
		}
		else
		{
			throw std::runtime_error("Unsupported norm type");
		}
	}

	/**
	 * @brief 将 VectorXd 的前三元素转换为 Vector3f
	 *
	 * @param vector  输入 VectorXd（长度至少为 3）
	 * @return        单精度 Vector3f，元素以 static_cast<float> 转换
	 *
	 * @throws std::runtime_error 向量长度 < 3 时抛出
	 *
	 * @code
	 * VectorXd v(3); v << 1.0, 2.0, 3.0;
	 * Vector3f v3 = LinearAlgebraHelper::ToVec3(v);
	 * @endcode
	 */
	LinearAlgebraHelper::Vector3f LinearAlgebraHelper::ToVec3(const VectorXd &vector)
	{
		if (vector.size() < 3)
		{
			throw std::runtime_error("Vector must have at least 3 elements");
		}

		return Vector3f(static_cast<float>(vector[0]),
						static_cast<float>(vector[1]),
						static_cast<float>(vector[2]));
	}

	/**
	 * @brief 将 Vector3d 转换为长度为 3 的 VectorXd
	 *
	 * @param vector  输入 Eigen::Vector3d
	 * @return        VectorXd，元素依次为 [x, y, z]
	 *
	 * @code
	 * Vector3d v3(1, 2, 3);
	 * VectorXd v = LinearAlgebraHelper::Vec3ToVe(v3); // [1,2,3]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Vec3ToVe(const Vector3d &vector)
	{
		VectorXd result(3);
		result << vector.x(), vector.y(), vector.z();
		return result;
	}

	/**
	 * @brief 计算向量的指定范数（字符串指定类型）
	 *
	 * @param matrix  输入向量
	 * @param a       "inf" = 无穷范数，"fro" = Frobenius 范数
	 * @return        对应范数值
	 *
	 * @throws std::runtime_error 不支持的范数类型时抛出
	 *
	 * @code
	 * VectorXd v(3); v << 3, -4, 0;
	 * double normInf = LinearAlgebraHelper::Norm(v, "inf"); // 4.0
	 * @endcode
	 */
	double LinearAlgebraHelper::Norm(const VectorXd &matrix, const std::string &a)
	{
		if (a == "inf")
		{
			return matrix.lpNorm<Eigen::Infinity>();
		}
		else if (a == "fro")
		{
			return matrix.norm();
		}
		else
		{
			throw std::runtime_error("Unsupported norm type: " + a);
		}
	}

	/**
	 * @brief 计算矩阵的指定范数（整数类型）
	 *
	 * @param matrix  输入矩阵
	 * @param a       范数类型：1 = 元素绝对值之和，2 = 最大奇异值（谱范数）
	 * @return        矩阵范数值
	 *
	 * @throws std::runtime_error 不支持的范数类型时抛出
	 *
	 * @code
	 * MatrixXd A(2,2); A << 3,0, 0,4;
	 * double n2 = LinearAlgebraHelper::Norm(A, 2); // 4.0
	 * @endcode
	 */
	double LinearAlgebraHelper::Norm(const MatrixXd &matrix, int a)
	{
		if (a == 1)
		{
			return matrix.lpNorm<1>();
		}
		else if (a == 2)
		{
			Eigen::JacobiSVD<MatrixXd> svd(matrix);
			return svd.singularValues()(0);
		}
		else
		{
			throw std::runtime_error("Unsupported norm type");
		}
	}

	/**
	 * @brief 计算矩阵的指定范数（字符串类型）
	 *
	 * @param matrix  输入矩阵
	 * @param a       "inf" = 无穷范数，"fro" = Frobenius 范数
	 * @return        矩阵范数值
	 *
	 * @throws std::runtime_error 不支持的范数类型时抛出
	 *
	 * @code
	 * MatrixXd A(2,2); A << 1,2, 3,4;
	 * double nFro = LinearAlgebraHelper::Norm(A, "fro"); // 约 5.477
	 * @endcode
	 */
	double LinearAlgebraHelper::Norm(const MatrixXd &matrix, const std::string &a)
	{
		if (a == "inf")
		{
			return matrix.lpNorm<Eigen::Infinity>();
		}
		else if (a == "fro")
		{
			return matrix.norm();
		}
		else
		{
			throw std::runtime_error("Unsupported norm type: " + a);
		}
	}

#pragma endregion

#pragma region Matrix Repetition (Repmat)

	/**
	 * @brief 将向量沿指定维度重复 num 次以创建矩阵
	 *
	 * @param a    输入向量
	 * @param num  重复次数（必须 > 0）
	 * @param dim  1 = 按行重复（num×n 矩阵），2 = 按列重复（n×num 矩阵）
	 * @return     重复后的矩阵
	 *
	 * @throws std::runtime_error num <= 0 或 dim 不为 1/2 时抛出
	 *
	 * @code
	 * VectorXd v(2); v << 1, 2;
	 * MatrixXd M = LinearAlgebraHelper::Repmat(v, 3, 1); // 3×2 矩阵每行非 [1,2]
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Repmat(const VectorXd &a, int num, int dim)
	{
		if (num <= 0)
		{
			throw std::runtime_error("Number of repetitions must be greater than 0");
		}

		int size = static_cast<int>(a.size());

		if (dim == 1)
		{
			// Repeat along rows
			MatrixXd result(num, size);
			for (int i = 0; i < num; ++i)
			{
				result.row(i) = a.transpose();
			}
			return result;
		}
		else if (dim == 2)
		{
			// Repeat along columns
			MatrixXd result(size, num);
			for (int i = 0; i < num; ++i)
			{
				result.col(i) = a;
			}
			return result;
		}
		else
		{
			throw std::runtime_error("Dimension must be 1 or 2");
		}
	}

	/**
	 * @brief 将向量整体重复 × num 次，返回展开向量
	 *
	 * @param a    输入向量
	 * @param num  重复次数（必须 > 0）
	 * @return     长度为 (a.size() * num) 的 VectorXd
	 *
	 * @throws std::runtime_error num <= 0 时抛出
	 *
	 * @code
	 * VectorXd v(2); v << 3, 4;
	 * VectorXd r = LinearAlgebraHelper::Repmat(v, 3); // [3,4,3,4,3,4]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Repmat(const VectorXd &a, int num)
	{
		if (num <= 0)
		{
			throw std::runtime_error("Number of repetitions must be greater than 0");
		}

		int size = static_cast<int>(a.size());
		VectorXd result(size * num);

		for (int i = 0; i < num; ++i)
		{
			result.segment(i * size, size) = a;
		}

		return result;
	}

	/**
	 * @brief 将矩阵的行或列重复 × num 次
	 *
	 * @param a    输入矩阵
	 * @param num  重复次数（必须 > 0）
	 * @param row  true = 重复行，返回 (rows*num)×cols 矩阵；false = 重复列，返回 rows×(cols*num) 矩阵
	 * @return     重复后的矩阵
	 *
	 * @throws std::runtime_error num <= 0 时抛出
	 *
	 * @code
	 * MatrixXd A(2,2); A << 1,2,3,4;
	 * MatrixXd B = LinearAlgebraHelper::Repmat(A, 2, true); // 4×2
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Repmat(const MatrixXd &a, int num, bool row)
	{
		if (num <= 0)
		{
			throw std::runtime_error("Number of repetitions must be greater than 0");
		}

		int rows = static_cast<int>(a.rows());
		int cols = static_cast<int>(a.cols());

		if (row)
		{
			// Repeat rows
			MatrixXd result(rows * num, cols);
			for (int i = 0; i < num; ++i)
			{
				result.middleRows(i * rows, rows) = a;
			}
			return result;
		}
		else
		{
			// Repeat columns
			MatrixXd result(rows, cols * num);
			for (int i = 0; i < num; ++i)
			{
				result.middleCols(i * cols, cols) = a;
			}
			return result;
		}
	}

	/**
	 * @brief 将单精度向量沿指定维度重复 × num 次以创建矩阵（float 版本）
	 *
	 * @param a    输入单精度向量
	 * @param num  重复次数（必须 > 0）
	 * @param dim  1 = 按行重复（num×n 矩阵），2 = 按列重复（n×num 矩阵）
	 * @return     重复后的 MatrixXf
	 *
	 * @throws std::runtime_error num <= 0 或 dim 不为 1/2 时抛出
	 *
	 * @code
	 * VectorXf v(2); v << 1.0f, 2.0f;
	 * MatrixXf M = LinearAlgebraHelper::Repmat(v, 2, 2); // 2×2 float 矩阵
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::Repmat(const VectorXf &a, int num, int dim)
	{
		if (num <= 0)
		{
			throw std::runtime_error("Number of repetitions must be greater than 0");
		}

		int size = static_cast<int>(a.size());

		if (dim == 1)
		{
			// Repeat along rows
			MatrixXf result(num, size);
			for (int i = 0; i < num; ++i)
			{
				result.row(i) = a.transpose();
			}
			return result;
		}
		else if (dim == 2)
		{
			// Repeat along columns
			MatrixXf result(size, num);
			for (int i = 0; i < num; ++i)
			{
				result.col(i) = a;
			}
			return result;
		}
		else
		{
			throw std::runtime_error("Dimension must be 1 or 2");
		}
	}

#pragma endregion

#pragma region Matrix and Vector Creation (zeros, ones, eye)

	/**
	 * @brief 创建与给定双精度矩阵大小相同的矩阵
	 *
	 * @param matrix  参照矩阵
	 * @param co      true = 初始化为全零；false = 仅分配内存，不初始化
	 * @return        与 matrix 大小相同的 MatrixXd
	 *
	 * @code
	 * MatrixXd A(3,4);
	 * MatrixXd B = LinearAlgebraHelper::zeros(A, true);  // 3x4 全零矩阵
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::zeros(const MatrixXd &matrix, bool co)
	{
		if (co)
		{
			return MatrixXd::Zero(matrix.rows(), matrix.cols());
		}
		return MatrixXd(matrix.rows(), matrix.cols());
	}

	/**
	 * @brief 创建与给定双精度矩阵大小相同的稀疏零矩阵
	 *
	 * @param matrix  参照矩阵
	 * @return        相同指定维度的 SparseMatrix（全零）
	 *
	 * @code
	 * MatrixXd A(3,4);
	 * auto sp = LinearAlgebraHelper::zerosp(A);
	 * @endcode
	 */
	LinearAlgebraHelper::SparseMatrix LinearAlgebraHelper::zerosp(const MatrixXd &matrix)
	{
		return SparseMatrix(matrix.rows(), matrix.cols());
	}

	/**
	 * @brief 创建与给定单精度矩阵大小相同的单精度矩阵
	 *
	 * @param matrix  参照矩阵（单精度）
	 * @param co      true = 初始化为全零；false = 仅分配内存
	 * @return        与 matrix 大小相同的 MatrixXf
	 *
	 * @code
	 * MatrixXf A(2,3);
	 * MatrixXf B = LinearAlgebraHelper::zeros(A, true); // 2x3 全零
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::zeros(const MatrixXf &matrix, bool co)
	{
		if (co)
		{
			return MatrixXf::Zero(matrix.rows(), matrix.cols());
		}
		return MatrixXf(matrix.rows(), matrix.cols());
	}

	/**
	 * @brief 创建与给定矩阵列表每个矩阵大小相同的新矩阵列表
	 *
	 * @param matrix  参照矩阵列表
	 * @param co      true = 初始化为全零；false = 仅分配内存
	 * @return        与每个矩阵大小相同的 MatrixXd 列表
	 *
	 * @code
	 * std::vector<MatrixXd> mats = { MatrixXd(2,3), MatrixXd(4,5) };
	 * auto result = LinearAlgebraHelper::zeros(mats, true);
	 * @endcode
	 */
	std::vector<LinearAlgebraHelper::MatrixXd> LinearAlgebraHelper::zeros(const std::vector<MatrixXd> &matrix, bool co)
	{
		std::vector<MatrixXd> result(matrix.size());
		for (size_t i = 0; i < matrix.size(); ++i)
		{
			result[i] = zeros(matrix[i], co);
		}
		return result;
	}

	/**
	 * @brief 将 MatrixXd 转换为外层=(rows)内层=(cols) 的二维 double 数组
	 *
	 * @param matrix  输入矩阵
	 * @return        result[i][j] == matrix(i, j)的二维 std::vector
	 *
	 * @code
	 * MatrixXd A(2,3); A << 1,2,3, 4,5,6;
	 * auto arr = LinearAlgebraHelper::ConvertToDouble2(A); // arr[0]={1,2,3}, arr[1]={4,5,6}
	 * @endcode
	 */
	std::vector<std::vector<double>> LinearAlgebraHelper::ConvertToDouble2(const MatrixXd &matrix)
	{
		int rows = static_cast<int>(matrix.rows());
		int cols = static_cast<int>(matrix.cols());

		std::vector<std::vector<double>> result(rows, std::vector<double>(cols));

		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				result[i][j] = matrix(i, j);
			}
		}

		return result;
	}

	/**
	 * @brief 将 MatrixXf 转换为二维 float 数组
	 *
	 * @param matrix  输入单精度矩阵
	 * @return        result[i][j] == matrix(i, j)的二维 std::vector<float>
	 *
	 * @code
	 * MatrixXf A(2,2); A << 1.0f,2.0f, 3.0f,4.0f;
	 * auto arr = LinearAlgebraHelper::ConvertTofloat2(A);
	 * @endcode
	 */
	std::vector<std::vector<float>> LinearAlgebraHelper::ConvertTofloat2(const MatrixXf &matrix)
	{
		int rows = static_cast<int>(matrix.rows());
		int cols = static_cast<int>(matrix.cols());

		std::vector<std::vector<float>> result(rows, std::vector<float>(cols));

		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				result[i][j] = matrix(i, j);
			}
		}

		return result;
	}

	/**
	 * @brief 以 VectorXd 的长度创建全零 std::vector<double>
	 *
	 * @param vector  参照向量（仅用于确定长度）
	 * @return        长度为 vector.size() 的全零 std::vector<double>
	 *
	 * @code
	 * VectorXd v(4);
	 * auto arr = LinearAlgebraHelper::zeros(v); // {0,0,0,0}
	 * @endcode
	 */
	std::vector<double> LinearAlgebraHelper::zeros(const VectorXd &vector)
	{
		return std::vector<double>(vector.size(), 0.0);
	}

	/**
	 * @brief 将指定 VectorXd 的所有元素设为零（原地操作）
	 *
	 * @param vector  要置零的向量（引用传递）
	 *
	 * @code
	 * VectorXd v(3); v << 1, 2, 3;
	 * LinearAlgebraHelper::zeros(v); // v 变为 [0,0,0]
	 * @endcode
	 */
	void LinearAlgebraHelper::zeros(VectorXd &vector)
	{
		vector.setZero();
	}

	/**
	 * @brief 将指定 MatrixXd 的所有元素设为零（原地操作）
	 *
	 * @param mat  要置零的矩阵（引用传递）
	 *
	 * @code
	 * MatrixXd M(2,2); M << 1,2,3,4;
	 * LinearAlgebraHelper::zeros(M); // M 变为全零矩阵
	 * @endcode
	 */
	void LinearAlgebraHelper::zeros(MatrixXd &mat)
	{
		mat.setZero();
	}

	/**
	 * @brief 以 VectorXf 的长度创建全零 std::vector<float>
	 *
	 * @param vector  参照向量（单精度）
	 * @return        长度为 vector.size() 的全零 std::vector<float>
	 *
	 * @code
	 * VectorXf v(3);
	 * auto arr = LinearAlgebraHelper::zeros(v); // {0,0,0}
	 * @endcode
	 */
	std::vector<float> LinearAlgebraHelper::zeros(const VectorXf &vector)
	{
		return std::vector<float>(vector.size(), 0.0f);
	}

	/**
	 * @brief 创建与给定 VectorXd 长度相同的双精度向量
	 *
	 * @param vector  参照向量
	 * @param co      true = 初始化为全零；false = 仅分配内存
	 * @return        长度相同的 VectorXd
	 *
	 * @code
	 * VectorXd v(5);
	 * VectorXd z = LinearAlgebraHelper::zeros(v, true); // 长度为5的全零向量
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::zeros(const VectorXd &vector, bool co)
	{
		if (co)
		{
			return VectorXd::Zero(vector.size());
		}
		return VectorXd(vector.size());
	}

	/**
	 * @brief 创建与给定 VectorXf 长度相同的单精度向量
	 *
	 * @param vector  参照向量（单精度）
	 * @param co      true = 初始化为全零；false = 仅分配内存
	 * @return        长度相同的 VectorXf
	 *
	 * @code
	 * VectorXf v(4);
	 * VectorXf z = LinearAlgebraHelper::fzeros(v, true); // 长度为4的全零单精度向量
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXf LinearAlgebraHelper::fzeros(const VectorXf &vector, bool co)
	{
		if (co)
		{
			return VectorXf::Zero(vector.size());
		}
		return VectorXf(vector.size());
	}

	/**
	 * @brief 以二维 double 数组的大小创建全零 MatrixXd
	 *
	 * @param matrix  参照二维数组
	 * @return        与 matrix 大小相同的全零 MatrixXd
	 *
	 * @code
	 * std::vector<std::vector<double>> m(3, std::vector<double>(4));
	 * MatrixXd Z = LinearAlgebraHelper::zeros(m); // 3x4 全零矩阵
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::zeros(const std::vector<std::vector<double>> &matrix)
	{
		if (matrix.empty())
			return MatrixXd();

		int rows = static_cast<int>(matrix.size());
		int cols = static_cast<int>(matrix[0].size());

		return MatrixXd::Zero(rows, cols);
	}

	/**
	 * @brief 以二维 float 数组的大小创建全零 MatrixXf
	 *
	 * @param matrix  参照二维单精度数组
	 * @return        与 matrix 大小相同的全零 MatrixXf
	 *
	 * @code
	 * std::vector<std::vector<float>> m(2, std::vector<float>(3));
	 * MatrixXf Z = LinearAlgebraHelper::fzeros(m); // 2x3 全零
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::fzeros(const std::vector<std::vector<float>> &matrix)
	{
		if (matrix.empty())
			return MatrixXf();

		int rows = static_cast<int>(matrix.size());
		int cols = static_cast<int>(matrix[0].size());

		return MatrixXf::Zero(rows, cols);
	}

	/**
	 * @brief 以一维 double 数组的长度创建全零 VectorXd
	 *
	 * @param matrix  参照一维数组
	 * @return        长度相同的全零 VectorXd
	 *
	 * @code
	 * std::vector<double> v(5);
	 * VectorXd z = LinearAlgebraHelper::zeros(v); // 长度5的全零向量
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::zeros(const std::vector<double> &matrix)
	{
		return VectorXd::Zero(matrix.size());
	}

	/**
	 * @brief 以一维 float 数组的长度创建全零 VectorXf
	 *
	 * @param matrix  参照一维单精度数组
	 * @return        长度相同的全零 VectorXf
	 *
	 * @code
	 * std::vector<float> v(3);
	 * VectorXf z = LinearAlgebraHelper::fzeros(v);
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXf LinearAlgebraHelper::fzeros(const std::vector<float> &matrix)
	{
		return VectorXf::Zero(matrix.size());
	}

	/**
	 * @brief 以整数数组的大小创建全零 VectorXd
	 *
	 * @param matrix  参照整数数组
	 * @return        长度相同的全零 VectorXd（double 类型）
	 *
	 * @code
	 * std::vector<int> v = {1, 2, 3};
	 * VectorXd z = LinearAlgebraHelper::zeros(v); // [0.0, 0.0, 0.0]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::zeros(const std::vector<int> &matrix)
	{
		return VectorXd::Zero(matrix.size());
	}

	/**
	 * @brief 以整数数组的大小创建全零 VectorXi
	 *
	 * @param matrix  参照整数数组
	 * @param unused  阅读标志（仅用于重载区分，不影响返回値）
	 * @return        长度相同的全零 VectorXi
	 *
	 * @code
	 * std::vector<int> v = {1, 2, 3};
	 * VectorXi z = LinearAlgebraHelper::zeros(v, true); // [0, 0, 0]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXi LinearAlgebraHelper::zeros(const std::vector<int> &matrix, bool unused)
	{
		return VectorXi::Zero(matrix.size());
	}

	/**
	 * @brief 生成十维度为 m×n×x 的三维 double 数组，元素初始化为指定值
	 *
	 * @param m      第一维大小
	 * @param n      第二维大小
	 * @param x      第三维大小
	 * @param value  初始化元素的值（默认通常传入 0.0）
	 * @return       m×n×x 的三维 std::vector
	 *
	 * @code
	 * auto arr3 = LinearAlgebraHelper::zeros(2, 3, 4, 0.0);
	 * @endcode
	 */
	std::vector<std::vector<std::vector<double>>> LinearAlgebraHelper::zeros(int m, int n, int x, double value)
	{
		return std::vector<std::vector<std::vector<double>>>(m,
															 std::vector<std::vector<double>>(n, std::vector<double>(x, value)));
	}

	/**
	 * @brief 生成 m×n 的双精度全零矩阵
	 *
	 * @param m  行数
	 * @param n  列数
	 * @return   m×n 的 MatrixXd::Zero
	 *
	 * @code
	 * MatrixXd Z = LinearAlgebraHelper::zeros(3, 4); // 3×4 全零矩阵
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::zeros(int m, int n)
	{
		return MatrixXd::Zero(m, n);
	}

	/**
	 * @brief 生成 m×n 的稀疏全零矩阵
	 *
	 * @param m  行数
	 * @param n  列数
	 * @return   m×n 的 SparseMatrix（全零）
	 *
	 * @code
	 * auto sp = LinearAlgebraHelper::zerosp(10, 10);
	 * @endcode
	 */
	LinearAlgebraHelper::SparseMatrix LinearAlgebraHelper::zerosp(int m, int n)
	{
		return SparseMatrix(m, n);
	}

	/**
	 * @brief 生成 m×n 的单精度全零矩阵
	 *
	 * @param m  行数
	 * @param n  列数
	 * @return   m×n 的 MatrixXf::Zero
	 *
	 * @code
	 * MatrixXf Z = LinearAlgebraHelper::fzeros(2, 3);
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::fzeros(int m, int n)
	{
		return MatrixXf::Zero(m, n);
	}

	/**
	 * @brief 生成长度为 m 的双精度全零向量
	 *
	 * @param m  向量长度
	 * @return   VectorXd::Zero(m)
	 *
	 * @code
	 * VectorXd z = LinearAlgebraHelper::zeros(5); // [0,0,0,0,0]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::zeros(int m)
	{
		return VectorXd::Zero(m);
	}

	/**
	 * @brief 生成长度为 m 的单精度全零向量
	 *
	 * @param m  向量长度
	 * @return   VectorXf::Zero(m)
	 *
	 * @code
	 * VectorXf z = LinearAlgebraHelper::fzeros(4);
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXf LinearAlgebraHelper::fzeros(int m)
	{
		return VectorXf::Zero(m);
	}

	/**
	 * @brief 生成 num 个大小为 row×column 的全零 MatrixXd 列表
	 *
	 * @param row     矩阵行数
	 * @param column  矩阵列数
	 * @param num     矩阵数量
	 * @return        num 个全零 MatrixXd 的列表
	 *
	 * @code
	 * auto mats = LinearAlgebraHelper::zeros(3, 4, 5); // 5 个 3×4 全零矩阵
	 * @endcode
	 */
	std::vector<LinearAlgebraHelper::MatrixXd> LinearAlgebraHelper::zeros(int row, int column, int num)
	{
		return std::vector<MatrixXd>(num, MatrixXd::Zero(row, column));
	}

	/**
	 * @brief 生成 num 个大小为 row×column 的全零 MatrixXf 列表
	 *
	 * @param row     矩阵行数
	 * @param column  矩阵列数
	 * @param num     矩阵数量
	 * @return        num 个全零 MatrixXf 的列表
	 *
	 * @code
	 * auto mats = LinearAlgebraHelper::fzeros(2, 3, 4); // 4 个 2×3 全零单精度矩阵
	 * @endcode
	 */
	std::vector<LinearAlgebraHelper::MatrixXf> LinearAlgebraHelper::fzeros(int row, int column, int num)
	{
		return std::vector<MatrixXf>(num, MatrixXf::Zero(row, column));
	}

	/**
	 * @brief 生成 num1×num 个大小为 row×column 的全零 MatrixXd 嵌套列表
	 *
	 * @param row     矩阵行数
	 * @param column  矩阵列数
	 * @param num     内层列表大小
	 * @param num1    外层列表大小
	 * @return        num1×num 嵌套全零矩阵列表
	 *
	 * @code
	 * auto mats = LinearAlgebraHelper::zeros(3, 4, 5, 2); // 2×5 嵌套，每个为 3×4 全零矩阵
	 * @endcode
	 */
	std::vector<std::vector<LinearAlgebraHelper::MatrixXd>> LinearAlgebraHelper::zeros(int row, int column, int num, int num1)
	{
		return std::vector<std::vector<MatrixXd>>(num1, std::vector<MatrixXd>(num, MatrixXd::Zero(row, column)));
	}

	/**
	 * @brief 创建 VectorXd 的副本（深拷贝）
	 *
	 * @param x  输入向量
	 * @return   新建的 VectorXd 副本
	 *
	 * @code
	 * VectorXd v(3); v << 1, 2, 3;
	 * VectorXd copy = LinearAlgebraHelper::Copy<VectorXd>(v);
	 * @endcode
	 */
	template <>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Copy<LinearAlgebraHelper::VectorXd>(const VectorXd &x)
	{
		return VectorXd(x);
	}

	/**
	 * @brief 创建嵌套 MatrixXd 列表的深拷贝
	 *
	 * @param matrix  输入嵌套矩阵列表
	 * @return        与原内容相同的新嵌套矩阵列表
	 *
	 * @code
	 * std::vector<std::vector<MatrixXd>> src = { { MatrixXd::Zero(2,2) } };
	 * auto dst = LinearAlgebraHelper::Copy(src);
	 * @endcode
	 */
	template <>
	std::vector<std::vector<LinearAlgebraHelper::MatrixXd>> LinearAlgebraHelper::Copy(const std::vector<std::vector<MatrixXd>> &matrix)
	{
		return std::vector<std::vector<MatrixXd>>(matrix);
	}

	/**
	 * @brief 创建 MatrixXd 列表的深拷贝
	 *
	 * @param matrix  输入 MatrixXd 列表
	 * @return        拷贝后的新列表
	 *
	 * @code
	 * std::vector<MatrixXd> src = { MatrixXd::Zero(2,3) };
	 * auto dst = LinearAlgebraHelper::Copy<MatrixXd>(src);
	 * @endcode
	 */
	template <>
	std::vector<LinearAlgebraHelper::MatrixXd> LinearAlgebraHelper::Copy<LinearAlgebraHelper::MatrixXd>(const std::vector<MatrixXd> &matrix)
	{
		return std::vector<MatrixXd>(matrix);
	}

	/**
	 * @brief 创建五维 double 数组的深拷贝
	 *
	 * @param ori  输入五维数组
	 * @return     拷贝后的新五维数组
	 *
	 * @code
	 * // auto dst = LinearAlgebraHelper::Copy<double>(ori5); // 5D 数组
	 * @endcode
	 */
	template <>
	std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>> LinearAlgebraHelper::Copy<double>(
		const std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>> &ori)
	{
		return std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>>(ori);
	}

	/**
	 * @brief 创建四维 double 数组的深拷贝
	 *
	 * @param ori  输入四维数组
	 * @return     拷贝后的新四维数组
	 *
	 * @code
	 * // auto dst = LinearAlgebraHelper::Copy<double>(ori4); // 4D 数组
	 * @endcode
	 */
	template <>
	std::vector<std::vector<std::vector<std::vector<double>>>> LinearAlgebraHelper::Copy<double>(
		const std::vector<std::vector<std::vector<std::vector<double>>>> &ori)
	{
		return std::vector<std::vector<std::vector<std::vector<double>>>>(ori);
	}

	/**
	 * @brief 创建三维 double 数组的深拷贝
	 *
	 * @param ori  输入三维数组
	 * @return     拷贝后的新三维数组
	 *
	 * @code
	 * // auto dst = LinearAlgebraHelper::Copy<double>(ori3);
	 * @endcode
	 */
	template <>
	std::vector<std::vector<std::vector<double>>> LinearAlgebraHelper::Copy<double>(const std::vector<std::vector<std::vector<double>>> &ori)
	{
		return std::vector<std::vector<std::vector<double>>>(ori);
	}

	/**
	 * @brief 创建二维 double 数组的深拷贝
	 *
	 * @param ori  输入二维数组
	 * @return     拷贝后的新二维数组
	 *
	 * @code
	 * std::vector<std::vector<double>> src = {{1,2},{3,4}};
	 * auto dst = LinearAlgebraHelper::Copy(src);
	 * @endcode
	 */
	template <>
	std::vector<std::vector<double>> LinearAlgebraHelper::Copy(const std::vector<std::vector<double>> &ori)
	{
		return std::vector<std::vector<double>>(ori);
	}
	/**
	 * @brief 创建 std::vector<std::string> 的深拷贝
	 *
	 * @param ori  输入字符串数组
	 * @return     拷贝后的新字符串数组
	 *
	 * @code
	 * std::vector<std::string> src = {"a", "b"};
	 * auto dst = LinearAlgebraHelper::Copy<std::string>(src);
	 * @endcode
	 */
	template <>
	std::vector<std::string> LinearAlgebraHelper::Copy<std::string>(const std::vector<std::string> &ori)
	{
		return std::vector<std::string>(ori);
	}

	template <>
	std::vector<int> LinearAlgebraHelper::Copy<int>(const std::vector<int> &ori)
	{
		return std::vector<int>(ori);
	}
	template <>
	std::vector<float> LinearAlgebraHelper::Copy<float>(const std::vector<float> &ori)
	{
		return std::vector<float>(ori);
	}

	template <>
	std::vector<double> LinearAlgebraHelper::Copy<double>(const std::vector<double> &ori)
	{
		return std::vector<double>(ori);
	}

	template <>
	std::vector<bool> LinearAlgebraHelper::Copy<bool>(const std::vector<bool> &ori)
	{
		return std::vector<bool>(ori);
	}

	/**
	 * @brief 创建 n×n 双精度单位矩阵
	 *
	 * @details 内部调用 MatrixXd::Identity(n, n)，性能最优。
	 * @param[in] n  矩阵阶数（行列均为 n）
	 * @return       n×n 单位矩阵（对角 1，其余 0）
	 *
	 * @code
	 * MatrixXd I3 = LinearAlgebraHelper::Eye(3);
	 * // I3 = {{1,0,0},{0,1,0},{0,0,1}}
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Eye(int n)
	{
		return MatrixXd::Identity(n, n);
	}

	/**
	 * @brief 创建 n×n 双精度对角矩阵，对角线元素均为指定值
	 *
	 * @param[in] n      矩阵阶数
	 * @param[in] value  对角线元素值
	 * @return           n×n 矩阵（value·I_n）
	 *
	 * @code
	 * MatrixXd M = LinearAlgebraHelper::Eye(3, 2.5);
	 * // M = diag(2.5, 2.5, 2.5)
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Eye(int n, double value)
	{
		return value * MatrixXd::Identity(n, n);
	}

	/**
	 * @brief 创建 n×m 双精度矩形单位矩阵
	 *
	 * @details 非方阵情形：左上角 min(n,m)×min(n,m) 子块为单位矩阵，超出部分为零。
	 * @param[in] n  行数
	 * @param[in] m  列数
	 * @return       n×m 矩阵，对角线（i==j）元素为 1，其余为 0
	 *
	 * @code
	 * MatrixXd E = LinearAlgebraHelper::Eye(3, 5);
	 * // E.rows()==3, E.cols()==5, E(0,0)==E(1,1)==E(2,2)==1
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Eye(int n, int m)
	{
		MatrixXd result = MatrixXd::Zero(n, m);
		int minDim = std::min(n, m);
		for (int i = 0; i < minDim; ++i)
		{
			result(i, i) = 1.0;
		}
		return result;
	}

	/**
	 * @brief 由 VectorXd 创建双精度对角矩阵
	 *
	 * @param diag  对角线元素向量
	 * @return      n×n 对角矩阵
	 *
	 * @code
	 * VectorXd d(3); d << 2, 3, 4;
	 * MatrixXd D = LinearAlgebraHelper::Eye(d); // diag(2,3,4)
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Eye(const VectorXd &diag)
	{
		return diag.asDiagonal();
	}

	/**
	 * @brief 由 std::vector<double> 创建双精度对角矩阵
	 *
	 * @param diag  对角线元素数组
	 * @return      n×n 对角矩阵
	 *
	 * @code
	 * std::vector<double> d = {2, 3, 4};
	 * MatrixXd D = LinearAlgebraHelper::Eye(d);
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Eye(const std::vector<double> &diag)
	{
		VectorXd eigenDiag = Eigen::Map<const VectorXd>(diag.data(), diag.size());
		return eigenDiag.asDiagonal();
	}

	/**
	 * @brief 创建 n×n 单精度单位矩阵
	 *
	 * @param[in] n  矩阵阶数
	 * @return       n×n 单精度单位矩阵（对角 1.0f，其余 0.0f）
	 *
	 * @code
	 * MatrixXf I = LinearAlgebraHelper::fEye(4);
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::fEye(int n)
	{
		return MatrixXf::Identity(n, n);
	}

	/**
	 * @brief 创建 n×n 单精度对角矩阵，对角线元素均为指定值
	 *
	 * @param[in] n      矩阵阶数
	 * @param[in] value  对角线元素值（float）
	 * @return           n×n 单精度矩阵（value·I_n）
	 *
	 * @code
	 * MatrixXf M = LinearAlgebraHelper::Eye(3, 5.0f);
	 * // M = diag(5.0f, 5.0f, 5.0f)
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::Eye(int n, float value)
	{
		return value * MatrixXf::Identity(n, n);
	}

	/**
	 * @brief 创建 n×m 单精度矩形单位矩阵
	 *
	 * @details 非方阵：左上角 min(n,m)×min(n,m) 为单位矩阵，其余为 0.0f。
	 * @param[in] n  行数
	 * @param[in] m  列数
	 * @return       n×m 单精度矩阵
	 *
	 * @code
	 * MatrixXf E = LinearAlgebraHelper::fEye(4, 6);
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::fEye(int n, int m)
	{
		MatrixXf result = MatrixXf::Zero(n, m);
		int minDim = std::min(n, m);
		for (int i = 0; i < minDim; ++i)
		{
			result(i, i) = 1.0f;
		}
		return result;
	}

	/**
	 * @brief 由 VectorXf 创建单精度对角矩阵
	 *
	 * @param diag  单精度对角线元素向量
	 * @return      n×n 单精度对角矩阵
	 *
	 * @code
	 * VectorXf d(2); d << 3.0f, 4.0f;
	 * MatrixXf D = LinearAlgebraHelper::Eye(d);
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::Eye(const VectorXf &diag)
	{
		return diag.asDiagonal();
	}

	/**
	 * @brief 由 std::vector<float> 创建单精度对角矩阵
	 *
	 * @param diag  对角线元素单精度数组
	 * @return      n×n 单精度对角矩阵
	 *
	 * @code
	 * std::vector<float> d = {2.0f, 3.0f};
	 * MatrixXf D = LinearAlgebraHelper::Eye(d);
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::Eye(const std::vector<float> &diag)
	{
		VectorXf eigenDiag = Eigen::Map<const VectorXf>(diag.data(), diag.size());
		return eigenDiag.asDiagonal();
	}

	/**
	 * @brief 生成双精度对角矩阵，对角线元素由 VectorXd 指定
	 *
	 * @param a  对角线元素向量
	 * @return   n×n 对角矩阵
	 *
	 * @code
	 * VectorXd d(3); d << 1, 2, 3;
	 * MatrixXd D = LinearAlgebraHelper::Diag(d);
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Diag(const VectorXd &a)
	{
		return a.asDiagonal();
	}

	/**
	 * @brief 生成单精度对角矩阵，对角线元素由 VectorXf 指定
	 *
	 * @param a  对角线元素单精度向量
	 * @return   n×n 单精度对角矩阵
	 *
	 * @code
	 * VectorXf d(2); d << 5.0f, 6.0f;
	 * MatrixXf D = LinearAlgebraHelper::Diag(d);
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::Diag(const VectorXf &a)
	{
		return a.asDiagonal();
	}

	/**
	 * @brief 由 std::vector<double> 生成双精度对角矩阵
	 *
	 * @param a  对角线元素数组
	 * @return   n×n 对角矩阵
	 *
	 * @code
	 * std::vector<double> d = {1.0, 2.0, 3.0};
	 * MatrixXd D = LinearAlgebraHelper::Diag(d);
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Diag(const std::vector<double> &a)
	{
		VectorXd eigenVec = Eigen::Map<const VectorXd>(a.data(), a.size());
		return eigenVec.asDiagonal();
	}

	/**
	 * @brief 生成 n×m 的序列第一个全为1的双精度矩阵
	 *
	 * @param n  行数
	 * @param m  列数
	 * @return   MatrixXd::Ones(n, m)
	 *
	 * @code
	 * MatrixXd O = LinearAlgebraHelper::ones(3, 4); // 3×4 全1矩阵
	 * @endcode
	 */
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::ones(int n, int m)
	{
		return MatrixXd::Ones(n, m);
	}

	/**
	 * @brief 生成元素全为 1 的双精度列向量
	 *
	 * @param[in] n  向量长度
	 * @return       长度为 n 的列向量，所有元素均为 1.0
	 *
	 * @code
	 * VectorXd v = LinearAlgebraHelper::ones(5);
	 * // v = [1, 1, 1, 1, 1]
	 * @endcode
	 */
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::ones(int n)
	{
		return VectorXd::Ones(n);
	}

#pragma endregion

	// Explicit template instantiations for common types
	template std::tuple<int, double> LinearAlgebraHelper::FindClosestIndexAndValue<double>(const std::vector<double> &, double);
	template std::tuple<int, float> LinearAlgebraHelper::FindClosestIndexAndValue<float>(const std::vector<float> &, float);
	template std::vector<double> LinearAlgebraHelper::Sort<double>(const std::vector<double> &);
	template std::vector<float> LinearAlgebraHelper::Sort<float>(const std::vector<float> &);
	template std::vector<int> LinearAlgebraHelper::Sort<int>(const std::vector<int> &);
}
