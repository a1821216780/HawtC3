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

#include "IntegrationHelper.h"
#include "InterpolateHelper.h"
#include "MathHelper.h"
#include "LinearAlgebraHelper.h"
#include <algorithm>
#include <sstream>

#include "../Log/LogHelper.h"

using namespace HawtC3::IO::Log;
namespace HawtC3 {
	namespace IO {
		namespace Math {

			// ============================================================================
			// 私有辅助函数实现
			// ============================================================================

			template<typename T>
			void IntegrationHelper::ValidateInputSizes(const std::vector<T>& x, const std::vector<T>& y, const char* methodName) {
				if (x.size() != y.size())
				{
					LogHelper::ErrorLog("Input size mismatch. x.size()= " + std::to_string(x.size()) + ", y.size()= " + std::to_string(y.size()));
				}
				if (x.size() == 0 || y.size() == 0)
				{
					LogHelper::ErrorLog("x.size()= " + std::to_string(0) + ", y.size()= " + std::to_string(0));
				}
			}

			template<typename Derived1, typename Derived2>
			void IntegrationHelper::ValidateInputSizes(const Eigen::MatrixBase<Derived1>& x,
				const Eigen::MatrixBase<Derived2>& y,
				const char* methodName) {
				if (x.size() != y.size())
				{
					LogHelper::ErrorLog("Input size mismatch. x.size()= " + std::to_string(x.size()) + ", y.size()= " + std::to_string(y.size()));
				}
				if (x.size() == 0 || y.size() == 0)
				{
					LogHelper::ErrorLog("x.size()= " + std::to_string(0) + ", y.size()= " + std::to_string(0));
				}
			}

			template<typename T>
			void IntegrationHelper::ValidateSimpsMinimumPoints(const T& x, const char* methodName) {
				if (x.size() < 3) {
					std::ostringstream oss;
					oss << methodName << ": Input vector length must be >= 3, got " << x.size();
					throw std::invalid_argument(oss.str());
				}
				if (x.size() == 0) {
					std::ostringstream oss;
					oss << methodName << ": Input vector length must be >= 3, got " << x.size();
					throw std::invalid_argument(oss.str());
				}
			}

			// ============================================================================
			// 梯形积分实现 - 双精度数组版本
			// ============================================================================

			double IntegrationHelper::Trapz(const std::vector<double>& x, const std::vector<double>& y) {
				ValidateInputSizes(x, y, "Trapz");

				double integral = 0.0;
				for (size_t i = 0; i < x.size() - 1; ++i) {
					integral += (x[i + 1] - x[i]) * 0.5 * (y[i] + y[i + 1]);
				}
				return integral;
			}

			void IntegrationHelper::Trapz(const std::vector<double>& x, const std::vector<double>& y, double& integral) {
				integral = Trapz(x, y);
			}

			// ============================================================================
			// 梯形积分实现 - 单精度数组版本
			// ============================================================================

			float IntegrationHelper::Trapz(const std::vector<float>& x, const std::vector<float>& y) {

				ValidateInputSizes(x, y, "Trapz");

				float integral = 0.0f;
				for (size_t i = 0; i < x.size() - 1; ++i) {
					integral += (x[i + 1] - x[i]) * 0.5f * (y[i] + y[i + 1]);
				}
				return integral;
			}

			void IntegrationHelper::Trapz(const std::vector<float>& x, const std::vector<float>& y, float& integral) {
				integral = Trapz(x, y);
			}

			// ============================================================================
			// 梯形积分实现 - 双精度向量版本
			// ============================================================================

			double IntegrationHelper::Trapz(const Eigen::VectorXd& x, const Eigen::VectorXd& y) {
				ValidateInputSizes(x, y, "Trapz");

				double integral = 0.0;
				for (int i = 0; i < x.size() - 1; ++i) {
					integral += (x(i + 1) - x(i)) * 0.5 * (y(i) + y(i + 1));
				}
				return integral;
			}

			// ============================================================================
			// 梯形积分实现 - 单精度向量版本
			// ============================================================================

			float IntegrationHelper::Trapz(const Eigen::VectorXf& x, const Eigen::VectorXf& y) {
				ValidateInputSizes(x, y, "Trapz");

				float integral = 0.0f;
				for (int i = 0; i < x.size() - 1; ++i) {
					integral += (x(i + 1) - x(i)) * 0.5f * (y(i) + y(i + 1));
				}
				return integral;
			}

			// ============================================================================
			// 梯形积分实现 - 双精度矩阵版本
			// ============================================================================

			Eigen::VectorXd IntegrationHelper::Trapz(const Eigen::VectorXd& x, const Eigen::MatrixXd& y, int dim) {
				const int xCount = static_cast<int>(x.size());
				const int yRows = static_cast<int>(y.rows());
				const int yCols = static_cast<int>(y.cols());

				if (dim == 1 && xCount == yRows) {
					// 沿行积分（对每一列）
					Eigen::VectorXd result = Eigen::VectorXd::Zero(yCols);
					for (int i = 0; i < yCols; ++i) {
						result(i) = Trapz(x, y.col(i));
					}
					return result;
				}
				else if (dim == 2 && xCount == yCols) {
					// 沿列积分（对每一行）
					Eigen::VectorXd result = Eigen::VectorXd::Zero(yRows);
					for (int i = 0; i < yRows; ++i) {
						result(i) = Trapz(x, y.row(i));
					}
					return result;
				}
				else {
					std::ostringstream oss;
					oss << "Trapz: Dimension mismatch. dim=" << dim
						<< ", x.size()=" << xCount
						<< ", y.rows()=" << yRows
						<< ", y.cols()=" << yCols;
					throw std::invalid_argument(oss.str());
				}
			}

			void IntegrationHelper::Trapz(const Eigen::VectorXd& x, const Eigen::MatrixXd& y, int dim, Eigen::VectorXd& result) {
				result = Trapz(x, y, dim);
			}

			// ============================================================================
			// 梯形积分实现 - 单精度矩阵版本
			// ============================================================================

			Eigen::VectorXf IntegrationHelper::Trapz(const Eigen::VectorXf& x, const Eigen::MatrixXf& y, int dim) {
				const int xCount = static_cast<int>(x.size());
				const int yRows = static_cast<int>(y.rows());
				const int yCols = static_cast<int>(y.cols());

				if (dim == 1 && xCount == yRows) {
					// 沿行积分（对每一列）
					Eigen::VectorXf result = Eigen::VectorXf::Zero(yCols);
					for (int i = 0; i < yCols; ++i) {
						result(i) = Trapz(x, y.col(i));
					}
					return result;
				}
				else if (dim == 2 && xCount == yCols) {
					// 沿列积分（对每一行）
					Eigen::VectorXf result = Eigen::VectorXf::Zero(yRows);
					for (int i = 0; i < yRows; ++i) {
						result(i) = Trapz(x, y.row(i));
					}
					return result;
				}
				else {
					std::ostringstream oss;
					oss << "Trapz: Dimension mismatch. dim=" << dim
						<< ", x.size()=" << xCount
						<< ", y.rows()=" << yRows
						<< ", y.cols()=" << yCols;
					throw std::invalid_argument(oss.str());
				}
			}

			void IntegrationHelper::Trapz(const Eigen::VectorXf& x, const Eigen::MatrixXf& y, int dim, Eigen::VectorXf& result) {
				result = Trapz(x, y, dim);
			}

			// ============================================================================
			// 累积梯形积分实现 - 双精度版本
			// ============================================================================

			Eigen::VectorXd IntegrationHelper::TrapzA(const Eigen::VectorXd& x, const Eigen::VectorXd& y) {
				ValidateInputSizes(x, y, "TrapzA");

				const int n = static_cast<int>(x.size());
				Eigen::VectorXd result = Eigen::VectorXd::Zero(n);

				double integral = 0.0;
				for (int i = n - 2; i >= 0; --i) {
					integral += (x(i + 1) - x(i)) * 0.5 * (y(i) + y(i + 1));
					result(i) = integral;
				}

				return result;
			}

			// ============================================================================
			// 累积梯形积分实现 - 单精度版本
			// ============================================================================

			Eigen::VectorXf IntegrationHelper::TrapzA(const Eigen::VectorXf& x, const Eigen::VectorXf& y) {
				ValidateInputSizes(x, y, "TrapzA");

				const int n = static_cast<int>(x.size());
				Eigen::VectorXf result = Eigen::VectorXf::Zero(n);

				float integral = 0.0f;
				for (int i = n - 2; i >= 0; --i) {
					integral += (x(i + 1) - x(i)) * 0.5f * (y(i) + y(i + 1));
					result(i) = integral;
				}

				return result;
			}

			// ============================================================================
			// 辛普森积分实现 - 双精度数组版本
			// ============================================================================

			double IntegrationHelper::Simps(const std::vector<double>& x, const std::vector<double>& y) {
				ValidateInputSizes(x, y, "Simps");
				ValidateSimpsMinimumPoints(x, "Simps");

				const size_t n = x.size();
				double result = 0.0;

				// 对成对的区间应用辛普森规则
				for (size_t i = 0; i < n - 2; i += 2) {
					double h1 = x[i + 1] - x[i];
					double h2 = x[i + 2] - x[i + 1];
					double H = h1 + h2;

					result += (H / 6.0) * (y[i] + 4.0 * y[i + 1] + y[i + 2]);
				}

				// 如果点数为偶数，处理最后一个区间
				if (n % 2 == 0) {
					size_t end = n - 1;
					double h1 = x[end - 1] - x[end - 2];
					double h2 = x[end] - x[end - 1];
					double H = h1 + h2;

					result += (H / 6.0) * (y[end - 2] + 4.0 * y[end - 1] + y[end]);
				}

				return result;
			}

			// ============================================================================
			// 辛普森积分实现 - 双精度向量版本
			// ============================================================================

			double IntegrationHelper::Simps(const Eigen::VectorXd& x, const Eigen::VectorXd& y) {
				ValidateInputSizes(x, y, "Simps");
				ValidateSimpsMinimumPoints(x, "Simps");

				const size_t n = x.size();
				double result = 0.0;

				// 对成对的区间应用辛普森规则
				for (size_t i = 0; i < n - 2; i += 2) {
					double h1 = x[i + 1] - x[i];
					double h2 = x[i + 2] - x[i + 1];
					double H = h1 + h2;

					result += (H / 6.0) * (y[i] + 4.0 * y[i + 1] + y[i + 2]);
				}

				// 如果点数为偶数，处理最后一个区间
				if (n % 2 == 0) {
					size_t end = n - 1;
					double h1 = x[end - 1] - x[end - 2];
					double h2 = x[end] - x[end - 1];
					double H = h1 + h2;

					result += (H / 6.0) * (y[end - 2] + 4.0 * y[end - 1] + y[end]);
				}

				return result;
			}

			// ============================================================================
			// 辛普森积分实现 - 双精度矩阵版本
			// ============================================================================

			Eigen::VectorXd IntegrationHelper::Simps(const Eigen::VectorXd& x, const Eigen::MatrixXd& y, int dim) {
				const int xCount = static_cast<int>(x.size());
				const int yRows = static_cast<int>(y.rows());
				const int yCols = static_cast<int>(y.cols());

				if (dim == 1 && xCount == yRows) {
					// 沿行积分（对每一列）
					Eigen::VectorXd result(yCols);
					for (int i = 0; i < yCols; ++i) {
						result(i) = Simps(x, y.col(i));
					}
					return result;
				}
				else if (dim == 2 && xCount == yCols) {
					// 沿列积分（对每一行）
					Eigen::VectorXd result(yRows);
					for (int i = 0; i < yRows; ++i) {
						result(i) = Simps(x, y.row(i));
					}
					return result;
				}
				else {
					std::ostringstream oss;
					oss << "Simps: Dimension mismatch. dim=" << dim
						<< ", x.size()=" << xCount
						<< ", y.rows()=" << yRows
						<< ", y.cols()=" << yCols;
					throw std::invalid_argument(oss.str());
				}
			}

		} // namespace Math
	} // namespace IO
} // namespace HawtC3
