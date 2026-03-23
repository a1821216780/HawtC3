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


#include <limits>
#include <sstream>
#include <stdexcept>
#include <complex>
#include <cmath>
#include <execution>

#include "LinearAlgebraHelper.h"


namespace HawtC3::IO::Math
{
	// Helper functions for special mathematical functions
	namespace {
		// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Simplified implementations for special functions
// These are basic approximations - for production use, consider proper implementations

/// <summary>
/// Basic approximation of Modified Bessel function K_nu(z)
/// This is a simplified implementation - for accurate results use proper mathematical libraries
/// </summary>
		double besselk_approx(double nu, double z) {
			if (z <= 0) return std::numeric_limits<double>::infinity();

			// Very basic approximation for demonstration
			// In production, use proper Bessel function implementation
			if (z < 1.0) {
				return std::pow(2.0, nu - 1.0) * std::tgamma(nu) * std::pow(z, -nu);
			}
			else {
				return std::sqrt(M_PI / (2.0 * z)) * std::exp(-z);
			}
		}

		/// <summary>
		/// Gamma function implementation
		/// </summary>
		double gamma_approx(double x) {
			return std::tgamma(x);
		}
	}

#pragma region Array Expansion and Processing

	/// <summary>
	/// 将数组扩展到指定的目标长度，同时保持原始数组元素在计算位置的精确值，并对其他位置进行插值
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::ExpandArrayPreserveExact(const VectorXd& originalArray, int targetLength,
		double low, double up) {

		if (originalArray.size() == 0) {
			throw std::runtime_error("原始数组不能为空");
		}

		if (targetLength <= originalArray.size()) {
			throw std::runtime_error("目标长度必须大于原数组长度");
		}

		// Handle NaN values for bounds
		if (std::isnan(low)) {
			low = originalArray.minCoeff();
		}
		if (std::isnan(up)) {
			up = originalArray.maxCoeff();
		}

		// Find boundary indices
		int lowIdx = -1, upIdx = -1;
		for (int i = 0; i < originalArray.size(); ++i) {
			if (std::abs(originalArray[i] - low) < 1e-10) {
				lowIdx = i;
			}
			if (std::abs(originalArray[i] - up) < 1e-10) {
				upIdx = i;
			}
		}

		VectorXd workingArray = originalArray;

		// Add boundary points if not found
		if (lowIdx == -1) {
			VectorXd temp(originalArray.size() + 1);
			temp[0] = low;
			temp.segment(1, originalArray.size()) = originalArray;
			workingArray = temp;
			lowIdx = 0;
			if (upIdx >= 0) upIdx++;
		}

		if (upIdx == -1) {
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
		for (int i = 0; i < centerPart.size(); ++i) {
			exactIndices[i] = static_cast<int>(std::round(static_cast<double>(i) * (adjustedTargetLength - 1) / (centerPart.size() - 1)));
		}

		// Set exact points
		for (int i = 0; i < centerPart.size(); ++i) {
			result[exactIndices[i]] = centerPart[i];
		}

		// Interpolate other positions
		for (int i = 0; i < adjustedTargetLength; ++i) {
			bool isExactPoint = false;
			for (int exactIdx : exactIndices) {
				if (i == exactIdx) {
					isExactPoint = true;
					break;
				}
			}

			if (!isExactPoint) {
				// Find left and right exact points
				int leftIndex = -1, rightIndex = -1;

				for (int j = 0; j < exactIndices.size() - 1; ++j) {
					if (i > exactIndices[j] && i < exactIndices[j + 1]) {
						leftIndex = j;
						rightIndex = j + 1;
						break;
					}
				}

				if (leftIndex >= 0 && rightIndex >= 0) {
					// Linear interpolation
					double leftPos = exactIndices[leftIndex];
					double rightPos = exactIndices[rightIndex];
					double t = (i - leftPos) / (rightPos - leftPos);
					result[i] = centerPart[leftIndex] + t * (centerPart[rightIndex] - centerPart[leftIndex]);
				}
				else if (i < exactIndices[0]) {
					// Left boundary extrapolation
					result[i] = centerPart[0];
				}
				else if (i > exactIndices[exactIndices.size() - 1]) {
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

	/// <summary>
	/// 将值插入到已排序向量的适当位置，保持排序顺序
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::AddSortedValue(const VectorXd& v1, double value, bool sort) {
		VectorXd v = v1;

		if (sort) {
			std::vector<double> temp(v.data(), v.data() + v.size());
			std::sort(temp.begin(), temp.end());
			v = Eigen::Map<VectorXd>(temp.data(), temp.size());
		}

		if (v.size() == 0) {
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

	/// <summary>
	/// 计算指定向量元素的累积和
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Cumsum(const VectorXd& v) {
		VectorXd result(v.size());
		result[0] = v[0];
		for (int i = 1; i < v.size(); ++i) {
			result[i] = result[i - 1] + v[i];
		}
		return result;
	}

	/// <summary>
	/// 计算指定数组元素的累积和
	/// </summary>
	std::vector<double> LinearAlgebraHelper::Cumsum(const std::vector<double>& v) {
		std::vector<double> result(v.size());
		result[0] = v[0];
		for (size_t i = 1; i < v.size(); ++i) {
			result[i] = result[i - 1] + v[i];
		}
		return result;
	}

	/// <summary>
	/// 计算指定向量元素的反向累积和（差分）
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::ReCumsum(const VectorXd& v) {
		VectorXd result(v.size());
		result[0] = v[0];
		for (int i = 1; i < v.size(); ++i) {
			result[i] = v[i] - v[i - 1];
		}
		return result;
	}

	/// <summary>
	/// 计算指定数组元素的反向累积和（差分）
	/// </summary>
	std::vector<double> LinearAlgebraHelper::ReCumsum(const std::vector<double>& v) {
		std::vector<double> result(v.size());
		result[0] = v[0];
		for (size_t i = 1; i < v.size(); ++i) {
			result[i] = v[i] - v[i - 1];
		}
		return result;
	}

#pragma endregion

#pragma region Search and Find Operations

	/// <summary>
	/// 查找向量中最接近指定目标值的元素的索引和值
	/// </summary>
	template<>
	std::tuple<int, double> LinearAlgebraHelper::FindClosestIndexAndValue<double>(const std::vector<double>& v, double target) {
		if (v.empty()) {
			throw std::runtime_error("The input vector must not be null or empty.");
		}

		int closestIndex = 0;
		double closestValue = static_cast<double>(v[0]);
		double minDiff = std::abs(closestValue - target);

		for (size_t i = 1; i < v.size(); ++i) {
			double value = static_cast<double>(v[i]);
			double diff = std::abs(value - target);
			if (diff < minDiff) {
				minDiff = diff;
				closestIndex = static_cast<int>(i);
				closestValue = value;
			}
		}

		return std::make_tuple(closestIndex, closestValue);
	}

	/// <summary>
	/// 查找集合中最接近指定目标值的元素的索引和值（单精度版本）
	/// </summary>
	template<>
	std::tuple<int, float> LinearAlgebraHelper::FindClosestIndexAndValue<float>(const std::vector<float>& v, float target) {
		if (v.empty()) {
			throw std::runtime_error("The input vector must not be null or empty.");
		}

		int closestIndex = 0;
		float closestValue = static_cast<float>(v[0]);
		float minDiff = std::abs(closestValue - target);

		for (size_t i = 1; i < v.size(); ++i) {
			float value = static_cast<float>(v[i]);
			float diff = std::abs(value - target);
			if (diff < minDiff) {
				minDiff = diff;
				closestIndex = static_cast<int>(i);
				closestValue = value;
			}
		}

		return std::make_tuple(closestIndex, closestValue);
	}

	template<>
	std::tuple<int, int> LinearAlgebraHelper::FindClosestIndexAndValue<int>(const std::vector<int>& v, int target) {
		if (v.empty()) {
			throw std::runtime_error("The input vector must not be null or empty.");
		}

		int closestIndex = 0;
		int closestValue = static_cast<int>(v[0]);
		int minDiff = std::abs(closestValue - target);

		for (size_t i = 1; i < v.size(); ++i) {
			int value = static_cast<int>(v[i]);
			int diff = std::abs(value - target);
			if (diff < minDiff) {
				minDiff = diff;
				closestIndex = static_cast<int>(i);
				closestValue = value;
			}
		}

		return std::make_tuple(closestIndex, closestValue);
	}


	/// <summary>
	/// 对指定序列的元素按升序排序
	/// </summary>
	template<typename T>
	std::vector<T> LinearAlgebraHelper::Sort(const std::vector<T>& values) {
		std::vector<T> result = values;
		std::sort(result.begin(), result.end());
		return result;
	}

	/// <summary>
	/// 在已排序向量中查找第一个大于或等于指定目标值的元素的索引
	/// </summary>
	int LinearAlgebraHelper::FindSortedFirst(const VectorXd& v, double target) {
		int left = 0;
		int right = static_cast<int>(v.size()) - 1;

		while (left <= right) {
			int mid = (left + right) / 2;
			if (v[mid] < target) {
				left = mid + 1;
			}
			else {
				right = mid - 1;
			}
		}

		return left; // 返回第一个大于等于 target 的索引
	}

#pragma endregion

#pragma region Comparison and Equality

	/// <summary>
	/// 判断两个实数是否在指定容差内近似相等
	/// </summary>
	bool LinearAlgebraHelper::EqualRealNos(double a, double b, double epsilon) {
		return std::abs(a - b) < epsilon;
	}

#pragma endregion

#pragma region Matrix Information

	/// <summary>
	/// 获取指定矩阵的行数和列数
	/// </summary>
	std::tuple<int, int> LinearAlgebraHelper::Size(const MatrixXd& matrix) {
		return std::make_tuple(static_cast<int>(matrix.rows()), static_cast<int>(matrix.cols()));
	}

	/// <summary>
	/// 获取二维数组的行数和列数
	/// </summary>
	std::tuple<int, int> LinearAlgebraHelper::Size(const std::vector<std::vector<double>>& matrix) {
		if (matrix.empty()) return std::make_tuple(0, 0);
		return std::make_tuple(static_cast<int>(matrix.size()), static_cast<int>(matrix[0].size()));
	}

	/// <summary>
	/// 获取三维数组的行数、列数和深度
	/// </summary>
	std::tuple<int, int, int> LinearAlgebraHelper::Size(const std::vector<std::vector<std::vector<double>>>& matrix) {
		if (matrix.empty()) return std::make_tuple(0, 0, 0);
		if (matrix[0].empty()) return std::make_tuple(static_cast<int>(matrix.size()), 0, 0);
		return std::make_tuple(static_cast<int>(matrix.size()),
			static_cast<int>(matrix[0].size()),
			static_cast<int>(matrix[0][0].size()));
	}

	/// <summary>
	/// 获取矩阵指定维度的大小（与MATLAB完全一致）
	/// </summary>
	int LinearAlgebraHelper::Size(const MatrixXd& matrix, int a) {
		if (a == 1) {
			return static_cast<int>(matrix.rows());
		}
		else if (a == 2) {
			return static_cast<int>(matrix.cols());
		}
		else {
			throw std::runtime_error("错误使用size a=1 or a=2");
		}
	}

#pragma endregion

#pragma region Matrix Computations

	/// <summary>
	/// 基于提供的向量生成表示坐标网格的两个2D矩阵
	/// </summary>
	std::tuple<LinearAlgebraHelper::MatrixXd, LinearAlgebraHelper::MatrixXd>
		LinearAlgebraHelper::meshgrid(const VectorXd& x, const VectorXd& y, bool f) {

		MatrixXd xx, yy;

		if (f) {
			xx = MatrixXd(x.size(), y.size());
			yy = MatrixXd(x.size(), y.size());
			for (int i = 0; i < x.size(); ++i) {
				for (int j = 0; j < y.size(); ++j) {
					xx(i, j) = x[x.size() - 1 - i];
					yy(i, j) = y[j];
				}
			}
		}
		else {
			xx = MatrixXd(x.size(), y.size());
			yy = MatrixXd(x.size(), y.size());
			for (int i = 0; i < x.size(); ++i) {
				for (int j = 0; j < y.size(); ++j) {
					xx(i, j) = x[i];
					yy(i, j) = y[j];
				}
			}
		}

		return std::make_tuple(xx, yy);
	}

	/// <summary>
	/// 执行矩阵乘法A*B，将结果乘以alpha，并加上乘以beta的矩阵C
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Mul(MatrixXd& C, const MatrixXd& A, const MatrixXd& B, double alpha, double beta) {
		C = alpha * (A * B) + beta * C;
		return C;
	}

	/// <summary>
	/// 将两个矩阵相乘并将结果存储在指定的输出矩阵中
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Mul(MatrixXd& C, const MatrixXd& A, const MatrixXd& B) {
		C = A * B;
		return C;
	}

	/// <summary>
	/// 执行对称正定矩阵的LDL^T分解
	/// </summary>
	std::tuple<LinearAlgebraHelper::MatrixXd, LinearAlgebraHelper::MatrixXd>
		LinearAlgebraHelper::LDL(const MatrixXd& A) {

		int n = static_cast<int>(A.rows());
		MatrixXd L = Eye(n);
		MatrixXd D = MatrixXd::Zero(n, n);  // Use Eigen directly instead of zeros function

		for (int j = 0; j < n; ++j) {
			double sumD = 0.0;
			for (int m = 0; m < j; ++m) {
				sumD += L(j, m) * L(j, m) * D(m, m);
			}
			D(j, j) = A(j, j) - sumD;

			for (int i = j + 1; i < n; ++i) {
				double sumL = 0.0;
				for (int m = 0; m < j; ++m) {
					sumL += L(i, m) * L(j, m) * D(m, m);
				}
				L(i, j) = (A(i, j) - sumL) / D(j, j);
			}
		}

		return std::make_tuple(L, D);
	}

	/// <summary>
	/// 执行对称矩阵的LDL分解（单精度版本）
	/// </summary>
	std::tuple<LinearAlgebraHelper::MatrixXf, LinearAlgebraHelper::MatrixXf>
		LinearAlgebraHelper::LDL(const MatrixXf& A) {

		int n = static_cast<int>(A.rows());
		MatrixXf L = fEye(n);
		MatrixXf D = MatrixXf::Zero(n, n);  // Use Eigen directly instead of fzeros function

		for (int j = 0; j < n; ++j) {
			float sumD = 0.0f;
			for (int m = 0; m < j; ++m) {
				sumD += L(j, m) * L(j, m) * D(m, m);
			}
			D(j, j) = A(j, j) - sumD;

			for (int i = j + 1; i < n; ++i) {
				float sumL = 0.0f;
				for (int m = 0; m < j; ++m) {
					sumL += L(i, m) * L(j, m) * D(m, m);
				}
				L(i, j) = (A(i, j) - sumL) / D(j, j);
			}
		}

		return std::make_tuple(L, D);
	}

	/// <summary>
	/// 执行给定方形矩阵的PLDL分解（并行版本）
	/// </summary>
	std::tuple<LinearAlgebraHelper::MatrixXf, LinearAlgebraHelper::MatrixXf>
		LinearAlgebraHelper::PLDL(const MatrixXf& A) {

		int n = static_cast<int>(A.rows());
		MatrixXf L = MatrixXf::Zero(n, n);  // Use Eigen directly
		MatrixXf D = MatrixXf::Zero(n, n);  // Use Eigen directly

		// 并行计算 LDLT 分解
		for (int k = 0; k < n; ++k) {
			// 计算 D 的对角元素
			D(k, k) = A(k, k);
			for (int j = 0; j < k; ++j) {
				D(k, k) -= L(k, j) * L(k, j) * D(j, j);
			}

			// 填充 L 的下三角部分
			L(k, k) = 1.0f; // L 的对角元素设置为 1

			// Simple loop instead of OpenMP for compatibility
			for (int i = k + 1; i < n; ++i) {
				L(i, k) = A(i, k);
				for (int j = 0; j < k; ++j) {
					L(i, k) -= L(i, j) * L(k, j) * D(j, j);
				}
				L(i, k) /= D(k, k); // 更新 L 的元素
			}
		}

		return std::make_tuple(L, D);
	}

#pragma endregion

#pragma region File I/O Operations

	/// <summary>
	/// 从文件中读取矩阵数据
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::ReadMatrixFromFile(const std::string& filePath) {
		std::ifstream file(filePath);
		if (!file.is_open()) {
			throw std::runtime_error("Cannot open file: " + filePath);
		}

		std::vector<std::vector<double>> data;
		std::string line;

		while (std::getline(file, line)) {
			std::istringstream iss(line);
			std::vector<double> row;
			std::string token;

			while (std::getline(iss, token, ' ') || std::getline(iss, token, ',') || std::getline(iss, token, '\t')) {
				if (!token.empty()) {
					try {
						row.push_back(std::stod(token));
					}
					catch (const std::exception&) {
						// Skip invalid tokens
					}
				}
			}

			if (!row.empty()) {
				data.push_back(row);
			}
		}

		if (data.empty()) {
			throw std::runtime_error("No valid data found in file");
		}

		int rows = static_cast<int>(data.size());
		int cols = static_cast<int>(data[0].size());
		MatrixXd matrix(rows, cols);

		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols && j < static_cast<int>(data[i].size()); ++j) {
				matrix(i, j) = data[i][j];
			}
		}

		return matrix;
	}

	/// <summary>
	/// 从文件读取MATLAB风格的复数矩阵并返回矩阵对象
	/// </summary>
	Eigen::MatrixXcd LinearAlgebraHelper::ReadMatlabMatrixFromFile(const std::string& filePath) {
		std::ifstream file(filePath);
		if (!file.is_open()) {
			throw std::runtime_error("Cannot open file: " + filePath);
		}

		std::vector<std::vector<std::complex<double>>> data;
		std::string line;

		while (std::getline(file, line)) {
			std::istringstream iss(line);
			std::vector<std::complex<double>> row;
			std::string token;

			while (std::getline(iss, token, '\t')) {
				if (!token.empty()) {
					// Parse complex number in format "a+bi" or "a-bi"
					try {
						// This is a simplified parser - would need more robust implementation
						std::complex<double> value(0, 0);
						// TODO: Implement proper complex number parsing
						row.push_back(value);
					}
					catch (const std::exception&) {
						// Skip invalid tokens
					}
				}
			}

			if (!row.empty()) {
				data.push_back(row);
			}
		}

		if (data.empty()) {
			throw std::runtime_error("No valid data found in file");
		}

		int rows = static_cast<int>(data.size());
		int cols = static_cast<int>(data[0].size());
		Eigen::MatrixXcd matrix(rows, cols);

		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols && j < static_cast<int>(data[i].size()); ++j) {
				matrix(i, j) = data[i][j];
			}
		}

		return matrix;
	}

#pragma endregion

#pragma region Eigenvalue and Eigenvector Operations

	/// <summary>
	/// 计算矩阵的特征值和特征向量，可选择按特征值大小升序排序
	/// </summary>
	std::tuple<LinearAlgebraHelper::VectorXd, LinearAlgebraHelper::MatrixXd>
		LinearAlgebraHelper::Eig(const MatrixXd& M, bool sort) {

		Eigen::EigenSolver<MatrixXd> solver(M);
		VectorXd eigenValues = solver.eigenvalues().real();
		MatrixXd eigenVectors = solver.eigenvectors().real();

		if (!sort) {
			return std::make_tuple(eigenValues, eigenVectors);
		}

		// Sort eigenvalues and eigenvectors
		std::vector<std::pair<double, int>> eigenPairs;
		for (int i = 0; i < eigenValues.size(); ++i) {
			eigenPairs.push_back(std::make_pair(eigenValues[i], i));
		}

		std::sort(eigenPairs.begin(), eigenPairs.end());

		VectorXd sortedEigenValues(eigenValues.size());
		MatrixXd sortedEigenVectors(eigenVectors.rows(), eigenVectors.cols());

		for (size_t i = 0; i < eigenPairs.size(); ++i) {
			sortedEigenValues[i] = eigenPairs[i].first;
			sortedEigenVectors.col(i) = eigenVectors.col(eigenPairs[i].second);
		}

		return std::make_tuple(sortedEigenValues, sortedEigenVectors);
	}

	/// <summary>
	/// 计算矩阵的特征值和特征向量（单精度版本）
	/// </summary>
	std::tuple<LinearAlgebraHelper::VectorXf, LinearAlgebraHelper::MatrixXf>
		LinearAlgebraHelper::Eig(const MatrixXf& M, bool sort) {

		Eigen::EigenSolver<MatrixXf> solver(M);
		VectorXf eigenValues = solver.eigenvalues().real();
		MatrixXf eigenVectors = solver.eigenvectors().real();

		if (!sort) {
			return std::make_tuple(eigenValues, eigenVectors);
		}

		// Sort eigenvalues and eigenvectors
		std::vector<std::pair<float, int>> eigenPairs;
		for (int i = 0; i < eigenValues.size(); ++i) {
			eigenPairs.push_back(std::make_pair(eigenValues[i], i));
		}

		std::sort(eigenPairs.begin(), eigenPairs.end());

		VectorXf sortedEigenValues(eigenValues.size());
		MatrixXf sortedEigenVectors(eigenVectors.rows(), eigenVectors.cols());

		for (int i = 0; i < eigenPairs.size(); ++i) {
			sortedEigenValues[i] = eigenPairs[i].first;
			sortedEigenVectors.col(i) = eigenVectors.col(eigenPairs[i].second);
		}

		return std::make_tuple(sortedEigenValues, sortedEigenVectors);
	}

	/// <summary>
	/// 计算广义特征值问题的特征值和特征向量
	/// </summary>
	std::tuple<LinearAlgebraHelper::VectorXd, LinearAlgebraHelper::MatrixXd>
		LinearAlgebraHelper::Eig(const MatrixXd& K, const MatrixXd& M, bool sort) {

		Eigen::GeneralizedEigenSolver<MatrixXd> solver(K, M);
		VectorXd eigenValues = solver.eigenvalues().real();
		MatrixXd eigenVectors = solver.eigenvectors().real();

		if (!sort) {
			return std::make_tuple(eigenValues, eigenVectors);
		}

		// Sort eigenvalues and eigenvectors
		std::vector<std::pair<double, int>> eigenPairs;
		for (int i = 0; i < eigenValues.size(); ++i) {
			eigenPairs.push_back(std::make_pair(eigenValues[i], i));
		}

		std::sort(eigenPairs.begin(), eigenPairs.end());

		VectorXd sortedEigenValues(eigenValues.size());
		MatrixXd sortedEigenVectors(eigenVectors.rows(), eigenVectors.cols());

		for (size_t i = 0; i < eigenPairs.size(); ++i) {
			sortedEigenValues[i] = eigenPairs[i].first;
			sortedEigenVectors.col(i) = eigenVectors.col(eigenPairs[i].second);
		}

		return std::make_tuple(sortedEigenValues, sortedEigenVectors);
	}

#pragma endregion

#pragma region Vector Products

	/// <summary>
	/// 计算向量的外积（外积矩阵）
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Outer(const VectorXd& a, const VectorXd& b) {
		return a * b.transpose();
	}

	/// <summary>
	/// 计算向量的内积（点积）
	/// </summary>
	double LinearAlgebraHelper::Inter(const VectorXd& a, const VectorXd& b) {
		return a.dot(b);
	}

#pragma endregion

#pragma region Vector Concatenation

	/// <summary>
	/// 将两个可枚举序列水平连接为一个向量
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(const std::vector<double>& a, const std::vector<double>& b) {
		int al = static_cast<int>(a.size());
		int bl = static_cast<int>(b.size());
		VectorXd result(al + bl);

		for (int i = 0; i < al; ++i) {
			result[i] = a[i];
		}
		for (int i = 0; i < bl; ++i) {
			result[al + i] = b[i];
		}

		return result;
	}

	/// <summary>
	/// 将两个整数向量水平连接
	/// </summary>
	LinearAlgebraHelper::VectorXi LinearAlgebraHelper::Hact(const VectorXi& a, const VectorXi& b) {
		int al = static_cast<int>(a.size());
		int bl = static_cast<int>(b.size());
		VectorXi result(al + bl);

		result.head(al) = a;
		result.tail(bl) = b;

		return result;
	}

	/// <summary>
	/// 将一个标量和一个向量连接为一个向量
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(double a, const VectorXd& b) {
		int bl = static_cast<int>(b.size());
		VectorXd result(1 + bl);

		result[0] = a;
		result.tail(bl) = b;

		return result;
	}

	/// <summary>
	/// 将一个向量和一个标量连接为一个向量
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(const VectorXd& a, double b) {
		int al = static_cast<int>(a.size());
		VectorXd result(al + 1);

		result.head(al) = a;
		result[al] = b;

		return result;
	}

	/// <summary>
	/// 将多个矩阵水平连接成单个矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Hact(const std::vector<MatrixXd>& input) {
		if (input.empty()) {
			return MatrixXd();
		}

		int totalCols = 0;
		int rows = static_cast<int>(input[0].rows());

		for (const auto& mat : input) {
			totalCols += static_cast<int>(mat.cols());
		}

		MatrixXd result(rows, totalCols);
		int colOffset = 0;

		for (const auto& mat : input) {
			result.middleCols(colOffset, mat.cols()) = mat;
			colOffset += static_cast<int>(mat.cols());
		}

		return result;
	}

	/// <summary>
	/// 将多个向量连接为一个向量
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(const std::vector<VectorXd>& input) {
		int totalSize = 0;
		for (const auto& vec : input) {
			totalSize += static_cast<int>(vec.size());
		}

		VectorXd result(totalSize);
		int offset = 0;

		for (const auto& vec : input) {
			result.segment(offset, vec.size()) = vec;
			offset += static_cast<int>(vec.size());
		}

		return result;
	}

	/// <summary>
	/// 将Vector3数组转换为Vector，通过展平其分量
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(const std::vector<Vector3d>& input) {
		VectorXd result(static_cast<int>(input.size()) * 3);

		for (size_t i = 0; i < input.size(); ++i) {
			result[i * 3 + 0] = input[i].x();
			result[i * 3 + 1] = input[i].y();
			result[i * 3 + 2] = input[i].z();
		}

		return result;
	}

	/// <summary>
	/// 将多个一维数组连接为一个向量
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Hact(const std::vector<std::vector<double>>& input) {
		int totalSize = 0;
		for (const auto& vec : input) {
			totalSize += static_cast<int>(vec.size());
		}

		VectorXd result(totalSize);
		int offset = 0;

		for (const auto& vec : input) {
			for (size_t i = 0; i < vec.size(); ++i) {
				result[offset + i] = vec[i];
			}
			offset += static_cast<int>(vec.size());
		}

		return result;
	}

	/// <summary>
	/// 将多个向量垂直连接为一个矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Vact(const std::vector<VectorXd>& a) {
		if (a.empty()) {
			return MatrixXd();
		}

		int rows = static_cast<int>(a[0].size());
		int cols = static_cast<int>(a.size());
		MatrixXd result(rows, cols);

		for (int i = 0; i < cols; ++i) {
			result.col(i) = a[i];
		}

		return result;
	}

	/// <summary>
	/// 将多个矩阵垂直连接为新的矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Vact(const std::vector<MatrixXd>& a) {
		if (a.empty()) {
			return MatrixXd();
		}

		int totalRows = 0;
		int cols = static_cast<int>(a[0].cols());

		for (const auto& mat : a) {
			totalRows += static_cast<int>(mat.rows());
		}

		MatrixXd result(totalRows, cols);
		int rowOffset = 0;

		for (const auto& mat : a) {
			result.middleRows(rowOffset, mat.rows()) = mat;
			rowOffset += static_cast<int>(mat.rows());
		}

		return result;
	}

	/// <summary>
	/// 将多个一维数组垂直连接为一个矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Vact(const std::vector<std::vector<double>>& a) {
		if (a.empty()) {
			return MatrixXd();
		}

		int rows = static_cast<int>(a[0].size());
		int cols = static_cast<int>(a.size());
		MatrixXd result(rows, cols);

		for (int i = 0; i < cols; ++i) {
			for (int j = 0; j < rows; ++j) {
				result(j, i) = a[i][j];
			}
		}

		return result;
	}

	/// <summary>
	/// 将两个向量连接为一个矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Vact(const VectorXd& a, const VectorXd& b, bool row) {
		int al = static_cast<int>(a.size());
		int bl = static_cast<int>(b.size());

		if (al != bl) {
			throw std::runtime_error("a 和 b 的长度不一样，无法 vcat！");
		}

		if (row) {
			MatrixXd result(2, al);
			result.row(0) = a.transpose();
			result.row(1) = b.transpose();
			return result;
		}
		else {
			MatrixXd result(al, 2);
			result.col(0) = a;
			result.col(1) = b;
			return result;
		}
	}

#pragma endregion

#pragma region Special Matrices

	/// <summary>
	/// 计算给定三维向量的反对称矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::SkewMatrix(const VectorXd& a) {
		if (a.size() != 3) {
			throw std::runtime_error("Vector must have exactly 3 elements for skew matrix");
		}

		MatrixXd skew(3, 3);
		skew << 0, -a[2], a[1],
			a[2], 0, -a[0],
			-a[1], a[0], 0;

		return skew;
	}

	/// <summary>
	/// 计算给定Vector3的反对称矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::SkewMatrix(const Vector3d& a) {
		MatrixXd skew(3, 3);
		skew << 0, -a.z(), a.y(),
			a.z(), 0, -a.x(),
			-a.y(), a.x(), 0;

		return skew;
	}

	/// <summary>
	/// 计算矩阵的迹（对角线元素之和）
	/// </summary>
	double LinearAlgebraHelper::Trace(const MatrixXd& a) {
		return a.trace();
	}

	/// <summary>
	/// 计算矩阵的行列式
	/// </summary>
	double LinearAlgebraHelper::Det(const MatrixXd& a) {
		return a.determinant();
	}

	/// <summary>
	/// 计算矩阵的正交化矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Orthogonal(const MatrixXd& a) {
		Eigen::HouseholderQR<MatrixXd> qr(a);
		return qr.householderQ();
	}

#pragma endregion

#pragma region Difference Operations

	/// <summary>
	/// 计算输入向量中连续元素之间的离散差分
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Diff(const VectorXd& input) {
		if (input.size() < 2) {
			throw std::runtime_error("Input vector must contain at least two elements");
		}

		VectorXd result(input.size() - 1);
		for (int i = 0; i < result.size(); ++i) {
			result[i] = input[i + 1] - input[i];
		}

		return result;
	}

#pragma endregion

#pragma region Norm Calculations

	/// <summary>
	/// 计算向量的范数，默认为L2范数
	/// </summary>
	double LinearAlgebraHelper::Norm(const VectorXd& vector, int a) {
		if (a == 1) {
			return vector.lpNorm<1>();
		}
		else if (a == 2) {
			return vector.norm();
		}
		else if (a == 3) {
			return vector.lpNorm<Eigen::Infinity>();
		}
		else {
			throw std::runtime_error("Unsupported norm type");
		}
	}

	/// <summary>
	/// 计算3D向量的范数
	/// </summary>
	double LinearAlgebraHelper::Norm(const Vector3d& vector, int a) {
		if (a == 1) {
			return vector.lpNorm<1>();
		}
		else if (a == 2) {
			return vector.norm();
		}
		else if (a == 3) {
			return vector.lpNorm<Eigen::Infinity>();
		}
		else {
			throw std::runtime_error("Unsupported norm type");
		}
	}

	/// <summary>
	/// 将MathNet.Numerics向量转换为Vector3
	/// </summary>
	LinearAlgebraHelper::Vector3f LinearAlgebraHelper::ToVec3(const VectorXd& vector) {
		if (vector.size() < 3) {
			throw std::runtime_error("Vector must have at least 3 elements");
		}

		return Vector3f(static_cast<float>(vector[0]),
			static_cast<float>(vector[1]),
			static_cast<float>(vector[2]));
	}

	/// <summary>
	/// 将Vector3实例转换为Vector
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Vec3ToVe(const Vector3d& vector) {
		VectorXd result(3);
		result << vector.x(), vector.y(), vector.z();
		return result;
	}

	/// <summary>
	/// 计算向量的指定范数（字符串版本）
	/// </summary>
	double LinearAlgebraHelper::Norm(const VectorXd& matrix, const std::string& a) {
		if (a == "inf") {
			return matrix.lpNorm<Eigen::Infinity>();
		}
		else if (a == "fro") {
			return matrix.norm();
		}
		else {
			throw std::runtime_error("Unsupported norm type: " + a);
		}
	}

	/// <summary>
	/// 计算矩阵的范数
	/// </summary>
	double LinearAlgebraHelper::Norm(const MatrixXd& matrix, int a) {
		if (a == 1) {
			return matrix.lpNorm<1>();
		}
		else if (a == 2) {
			Eigen::JacobiSVD<MatrixXd> svd(matrix);
			return svd.singularValues()(0);
		}
		else {
			throw std::runtime_error("Unsupported norm type");
		}
	}

	/// <summary>
	/// 计算矩阵的指定范数（字符串版本）
	/// </summary>
	double LinearAlgebraHelper::Norm(const MatrixXd& matrix, const std::string& a) {
		if (a == "inf") {
			return matrix.lpNorm<Eigen::Infinity>();
		}
		else if (a == "fro") {
			return matrix.norm();
		}
		else {
			throw std::runtime_error("Unsupported norm type: " + a);
		}
	}

#pragma endregion

#pragma region Matrix Repetition (Repmat)

	/// <summary>
	/// 通过沿指定维度多次重复向量来创建矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Repmat(const VectorXd& a, int num, int dim) {
		if (num <= 0) {
			throw std::runtime_error("Number of repetitions must be greater than 0");
		}

		int size = static_cast<int>(a.size());

		if (dim == 1) {
			// Repeat along rows
			MatrixXd result(num, size);
			for (int i = 0; i < num; ++i) {
				result.row(i) = a.transpose();
			}
			return result;
		}
		else if (dim == 2) {
			// Repeat along columns
			MatrixXd result(size, num);
			for (int i = 0; i < num; ++i) {
				result.col(i) = a;
			}
			return result;
		}
		else {
			throw std::runtime_error("Dimension must be 1 or 2");
		}
	}

	/// <summary>
	/// 重复指定向量的元素指定次数
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Repmat(const VectorXd& a, int num) {
		if (num <= 0) {
			throw std::runtime_error("Number of repetitions must be greater than 0");
		}

		int size = static_cast<int>(a.size());
		VectorXd result(size * num);

		for (int i = 0; i < num; ++i) {
			result.segment(i * size, size) = a;
		}

		return result;
	}

	/// <summary>
	/// 重复指定矩阵的行或列指定次数
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Repmat(const MatrixXd& a, int num, bool row) {
		if (num <= 0) {
			throw std::runtime_error("Number of repetitions must be greater than 0");
		}

		int rows = static_cast<int>(a.rows());
		int cols = static_cast<int>(a.cols());

		if (row) {
			// Repeat rows
			MatrixXd result(rows * num, cols);
			for (int i = 0; i < num; ++i) {
				result.middleRows(i * rows, rows) = a;
			}
			return result;
		}
		else {
			// Repeat columns
			MatrixXd result(rows, cols * num);
			for (int i = 0; i < num; ++i) {
				result.middleCols(i * cols, cols) = a;
			}
			return result;
		}
	}

	/// <summary>
	/// 沿指定维度复制单精度向量以创建矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::Repmat(const VectorXf& a, int num, int dim) {
		if (num <= 0) {
			throw std::runtime_error("Number of repetitions must be greater than 0");
		}

		int size = static_cast<int>(a.size());

		if (dim == 1) {
			// Repeat along rows
			MatrixXf result(num, size);
			for (int i = 0; i < num; ++i) {
				result.row(i) = a.transpose();
			}
			return result;
		}
		else if (dim == 2) {
			// Repeat along columns
			MatrixXf result(size, num);
			for (int i = 0; i < num; ++i) {
				result.col(i) = a;
			}
			return result;
		}
		else {
			throw std::runtime_error("Dimension must be 1 or 2");
		}
	}

#pragma endregion

#pragma region Matrix and Vector Creation (zeros, ones, eye)

	/// <summary>
	/// 返回一个与给定矩阵大小相同的矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::zeros(const MatrixXd& matrix, bool co) {
		if (co) {
			return MatrixXd::Zero(matrix.rows(), matrix.cols());
		}
		return MatrixXd(matrix.rows(), matrix.cols());
	}

	/// <summary>
	/// 返回一个与给定矩阵大小相同的稀疏零矩阵
	/// </summary>
	LinearAlgebraHelper::SparseMatrix LinearAlgebraHelper::zerosp(const MatrixXd& matrix) {
		return SparseMatrix(matrix.rows(), matrix.cols());
	}

	/// <summary>
	/// 返回一个与给定单精度矩阵大小相同的矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::zeros(const MatrixXf& matrix, bool co) {
		if (co) {
			return MatrixXf::Zero(matrix.rows(), matrix.cols());
		}
		return MatrixXf(matrix.rows(), matrix.cols());
	}

	/// <summary>
	/// 返回一个与给定矩阵列表大小相同的矩阵列表
	/// </summary>
	std::vector<LinearAlgebraHelper::MatrixXd> LinearAlgebraHelper::zeros(const std::vector<MatrixXd>& matrix, bool co) {
		std::vector<MatrixXd> result(matrix.size());
		for (size_t i = 0; i < matrix.size(); ++i) {
			result[i] = zeros(matrix[i], co);
		}
		return result;
	}

	/// <summary>
	/// 将矩阵转换为二维数组
	/// </summary>
	std::vector<std::vector<double>> LinearAlgebraHelper::ConvertToDouble2(const MatrixXd& matrix) {
		int rows = static_cast<int>(matrix.rows());
		int cols = static_cast<int>(matrix.cols());

		std::vector<std::vector<double>> result(rows, std::vector<double>(cols));

		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				result[i][j] = matrix(i, j);
			}
		}

		return result;
	}

	/// <summary>
	/// 将单精度矩阵转换为二维数组
	/// </summary>
	std::vector<std::vector<float>> LinearAlgebraHelper::ConvertTofloat2(const MatrixXf& matrix) {
		int rows = static_cast<int>(matrix.rows());
		int cols = static_cast<int>(matrix.cols());

		std::vector<std::vector<float>> result(rows, std::vector<float>(cols));

		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				result[i][j] = matrix(i, j);
			}
		}

		return result;
	}

	/// <summary>
	/// 将向量转换为一维数组
	/// </summary>
	std::vector<double> LinearAlgebraHelper::zeros(const VectorXd& vector) {
		return std::vector<double>(vector.size(), 0.0);
	}

	/// <summary>
	/// 将指定向量的所有元素设为零
	/// </summary>
	void LinearAlgebraHelper::zeros(VectorXd& vector) {
		vector.setZero();
	}

	/// <summary>
	/// 将指定矩阵的所有元素设为零
	/// </summary>
	void LinearAlgebraHelper::zeros(MatrixXd& mat) {
		mat.setZero();
	}

	/// <summary>
	/// 将单精度向量转换为一维数组
	/// </summary>
	std::vector<float> LinearAlgebraHelper::zeros(const VectorXf& vector) {
		return std::vector<float>(vector.size(), 0.0f);
	}

	/// <summary>
	/// 返回一个与给定向量大小相同的向量
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::zeros(const VectorXd& vector, bool co) {
		if (co) {
			return VectorXd::Zero(vector.size());
		}
		return VectorXd(vector.size());
	}

	/// <summary>
	/// 返回一个与给定单精度向量大小相同的向量
	/// </summary>
	LinearAlgebraHelper::VectorXf LinearAlgebraHelper::fzeros(const VectorXf& vector, bool co) {
		if (co) {
			return VectorXf::Zero(vector.size());
		}
		return VectorXf(vector.size());
	}

	/// <summary>
	/// 将二维数组转换为矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::zeros(const std::vector<std::vector<double>>& matrix) {
		if (matrix.empty()) return MatrixXd();

		int rows = static_cast<int>(matrix.size());
		int cols = static_cast<int>(matrix[0].size());

		return MatrixXd::Zero(rows, cols);
	}

	/// <summary>
	/// 将二维单精度数组转换为矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::fzeros(const std::vector<std::vector<float>>& matrix) {
		if (matrix.empty()) return MatrixXf();

		int rows = static_cast<int>(matrix.size());
		int cols = static_cast<int>(matrix[0].size());

		return MatrixXf::Zero(rows, cols);
	}

	/// <summary>
	/// 将一维数组转换为向量
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::zeros(const std::vector<double>& matrix) {
		return VectorXd::Zero(matrix.size());
	}

	/// <summary>
	/// 将一维单精度数组转换为向量
	/// </summary>
	LinearAlgebraHelper::VectorXf LinearAlgebraHelper::fzeros(const std::vector<float>& matrix) {
		return VectorXf::Zero(matrix.size());
	}

	/// <summary>
	/// 将整数数组转换为双精度向量
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::zeros(const std::vector<int>& matrix) {
		return VectorXd::Zero(matrix.size());
	}

	/// <summary>
	/// 将整数数组转换为整数向量
	/// </summary>
	LinearAlgebraHelper::VectorXi LinearAlgebraHelper::zeros(const std::vector<int>& matrix, bool unused) {
		return VectorXi::Zero(matrix.size());
	}

	/// <summary>
	/// 生成指定维度的三维数组，并可选择初始值
	/// </summary>
	std::vector<std::vector<std::vector<double>>> LinearAlgebraHelper::zeros(int m, int n, int x, double value) {
		return std::vector<std::vector<std::vector<double>>>(m,
			std::vector<std::vector<double>>(n, std::vector<double>(x, value)));
	}


	/// <summary>
	/// 生成指定维度的双精度零矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::zeros(int m, int n) {
		return MatrixXd::Zero(m, n);
	}

	/// <summary>
	/// 生成指定维度的稀疏零矩阵
	/// </summary>
	LinearAlgebraHelper::SparseMatrix LinearAlgebraHelper::zerosp(int m, int n) {
		return SparseMatrix(m, n);
	}

	/// <summary>
	/// 生成指定维度的单精度零矩阵
	/// </summary>
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::fzeros(int m, int n) {
		return MatrixXf::Zero(m, n);
	}

	/// <summary>
	/// 生成指定长度的双精度零向量
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::zeros(int m) {
		return VectorXd::Zero(m);
	}

	/// <summary>
	/// 生成指定长度的单精度零向量
	/// </summary>
	LinearAlgebraHelper::VectorXf LinearAlgebraHelper::fzeros(int m) {
		return VectorXf::Zero(m);
	}

	/// <summary>
	/// 生成指定数量的矩阵列表
	/// </summary>
	std::vector<LinearAlgebraHelper::MatrixXd> LinearAlgebraHelper::zeros(int row, int column, int num) {
		return std::vector<MatrixXd>(num, MatrixXd::Zero(row, column));
	}

	/// <summary>
	/// 生成指定数量的单精度矩阵列表
	/// </summary>
	std::vector<LinearAlgebraHelper::MatrixXf> LinearAlgebraHelper::fzeros(int row, int column, int num) {
		return std::vector<MatrixXf>(num, MatrixXf::Zero(row, column));
	}

	/// <summary>
	/// 生成嵌套的矩阵列表
	/// </summary>
	std::vector<std::vector<LinearAlgebraHelper::MatrixXd>> LinearAlgebraHelper::zeros(int row, int column, int num, int num1) {
		return std::vector<std::vector<MatrixXd>>(num1, std::vector<MatrixXd>(num, MatrixXd::Zero(row, column)));
	}


	/// <summary>
	/// 创建向量的副本
	/// </summary>
	template<>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::Copy<LinearAlgebraHelper::VectorXd>(const VectorXd& x)
	{
		return VectorXd(x);
	}

	/// <summary>
	/// 创建嵌套矩阵列表的深拷贝
	/// </summary>
	template<>
	std::vector<std::vector<LinearAlgebraHelper::MatrixXd>> LinearAlgebraHelper::Copy(const std::vector<std::vector<MatrixXd>>& matrix) {
		return std::vector<std::vector<MatrixXd>>(matrix);
	}


	/// <summary>
	/// 创建矩阵列表的深拷贝
	/// </summary>
	template<>
	std::vector<LinearAlgebraHelper::MatrixXd> LinearAlgebraHelper::Copy<LinearAlgebraHelper::MatrixXd>(const std::vector<MatrixXd>& matrix) {
		return std::vector<MatrixXd>(matrix);
	}


	

	/// <summary>
	/// 创建五维数组的深拷贝
	/// </summary>
	template<>
	std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>> LinearAlgebraHelper::Copy<double>(
		const std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>>& ori) {
		return std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>>(ori);
	}

	/// <summary>
	/// 创建四维数组的深拷贝
	/// </summary>
	template<>
	std::vector<std::vector<std::vector<std::vector<double>>>> LinearAlgebraHelper::Copy<double>(
		const std::vector<std::vector<std::vector<std::vector<double>>>>& ori) {
		return std::vector<std::vector<std::vector<std::vector<double>>>>(ori);
	}

	/// <summary>
	/// 创建三维数组的深拷贝
	/// </summary>
	template<>
	std::vector<std::vector<std::vector<double>>> LinearAlgebraHelper::Copy<double>(const std::vector<std::vector<std::vector<double>>>& ori) {
		return std::vector<std::vector<std::vector<double>>>(ori);
	}

	/// <summary>
	/// 创建二维数组的深拷贝（高性能版本）
	/// </summary>
	template<>
	std::vector<std::vector<double>> LinearAlgebraHelper::Copy(const std::vector<std::vector<double>>& ori) {
		return std::vector<std::vector<double			>>(ori);
	}
	/// <summary>
/// 创建1维数组的深拷贝（高性能版本）
/// </summary>
	template<>
	std::vector<std::string> LinearAlgebraHelper::Copy<std::string>(const std::vector<std::string>& ori) {
		return std::vector<std::string>(ori);
	}

	template<>
	std::vector<int> LinearAlgebraHelper::Copy<int>(const std::vector<int>& ori) {
		return std::vector<int>(ori);
	}
	template<>
	std::vector<float> LinearAlgebraHelper::Copy<float>(const std::vector<float>& ori) {
		return std::vector <float>(ori);
	}

	template<>
	std::vector<double> LinearAlgebraHelper::Copy<double>(const std::vector<double>& ori) {
		return std::vector <double>(ori);
	}

	template<>
	std::vector<bool> LinearAlgebraHelper::Copy<bool>(const std::vector<bool>& ori) {
		return std::vector <bool>(ori);
	}

	/// <summary>
	/// 创建n×n单位矩阵（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Eye(int n) {
		return MatrixXd::Identity(n, n);
	}

	/// <summary>
	/// 创建n×n矩阵，对角线元素为指定值（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Eye(int n, double value) {
		return value * MatrixXd::Identity(n, n);
	}

	/// <summary>
	/// 创建n×m单位矩阵（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Eye(int n, int m) {
		MatrixXd result = MatrixXd::Zero(n, m);
		int minDim = std::min(n, m);
		for (int i = 0; i < minDim; ++i) {
			result(i, i) = 1.0;
		}
		return result;
	}

	/// <summary>
	/// 创建对角矩阵，对角线元素由向量指定（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Eye(const VectorXd& diag) {
		return diag.asDiagonal();
	}

	/// <summary>
	/// 创建对角矩阵，对角线元素由数组指定（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Eye(const std::vector<double>& diag) {
		VectorXd eigenDiag = Eigen::Map<const VectorXd>(diag.data(), diag.size());
		return eigenDiag.asDiagonal();
	}

	/// <summary>
	/// 创建n×n单精度单位矩阵（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::fEye(int n) {
		return MatrixXf::Identity(n, n);
	}

	/// <summary>
	/// 创建n×n单精度矩阵，对角线元素为指定值（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::Eye(int n, float value) {
		return value * MatrixXf::Identity(n, n);
	}

	/// <summary>
	/// 创建n×m的单精度单位矩阵（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::fEye(int n, int m) {
		MatrixXf result = MatrixXf::Zero(n, m);
		int minDim = std::min(n, m);
		for (int i = 0; i < minDim; ++i) {
			result(i, i) = 1.0f;
		}
		return result;
	}

	/// <summary>
	/// 创建对角矩阵，对角线元素由单精度向量指定（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::Eye(const VectorXf& diag) {
		return diag.asDiagonal();
	}

	/// <summary>
	/// 创建对角矩阵，对角线元素由单精度数组指定（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::Eye(const std::vector<float>& diag) {
		VectorXf eigenDiag = Eigen::Map<const VectorXf>(diag.data(), diag.size());
		return eigenDiag.asDiagonal();
	}

	/// <summary>
	/// 生成双精度对角矩阵（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Diag(const VectorXd& a) {
		return a.asDiagonal();
	}

	/// <summary>
	/// 生成单精度对角矩阵（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXf LinearAlgebraHelper::Diag(const VectorXf& a) {
		return a.asDiagonal();
	}

	/// <summary>
	/// 生成双精度对角矩阵，支持可变参数（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::Diag(const std::vector<double>& a) {
		VectorXd eigenVec = Eigen::Map<const VectorXd>(a.data(), a.size());
		return eigenVec.asDiagonal();
	}

	/// <summary>
	/// 生成元素全为1的双精度矩阵（高性能版本）
	/// </summary>
	LinearAlgebraHelper::MatrixXd LinearAlgebraHelper::ones(int n, int m) {
		return MatrixXd::Ones(n, m);
	}

	/// <summary>
	/// 生成元素全为1的双精度向量
	/// </summary>
	LinearAlgebraHelper::VectorXd LinearAlgebraHelper::ones(int n) {
		return VectorXd::Ones(n);
	}

#pragma endregion

	// Explicit template instantiations for common types
	template std::tuple<int, double> LinearAlgebraHelper::FindClosestIndexAndValue<double>(const std::vector<double>&, double);
	template std::tuple<int, float> LinearAlgebraHelper::FindClosestIndexAndValue<float>(const std::vector<float>&, float);
	template std::vector<double> LinearAlgebraHelper::Sort<double>(const std::vector<double>&);
	template std::vector<float> LinearAlgebraHelper::Sort<float>(const std::vector<float>&);
	template std::vector<int> LinearAlgebraHelper::Sort<int>(const std::vector<int>&);
}

