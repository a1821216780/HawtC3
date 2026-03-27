//**********************************************************************************************************************************
//LICENSING
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

#include "IntegrationHelper.h"
#include "InterpolateHelper.h"
#include "MathHelper.h"
#include "LinearAlgebraHelper.h"
#include <algorithm>
#include <sstream>

#include "../Log/LogHelper.h"

using namespace Qahse::IO::Log;
namespace Qahse {
	namespace IO {
		namespace Math {

			// ============================================================================
			// 私有辅助函数实现
			// ============================================================================

			/**
			 * @brief 验证两个 std::vector 的长度匹配且非空
			 *
			 * @details 如果 x 与 y 长度不一致或任一为空，将调用 LogHelper::ErrorLog 中止程序。
			 *
			 * @tparam T 元素类型（通常为 double 或 float）
			 * @param[in] x          自变量向量
			 * @param[in] y          因变量向量
			 * @param[in] methodName 调用方法名称，用于错误信息中显示
			 *
			 * @code
			 * std::vector<double> x = {0,1,2}, y = {0,1,4};
			 * IntegrationHelper::ValidateInputSizes(x, y, "Trapz"); // 通过验证
			 * @endcode
			 */
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

			/**
			 * @brief 验证两个 Eigen 矩阵/向量的元素数量匹配且非空
			 *
			 * @details 如果 x.size() != y.size() 或任一为 0 元素，将调用 LogHelper::ErrorLog 中止程序。
			 *
			 * @tparam Derived1 Eigen 更派类型 1
			 * @tparam Derived2 Eigen 更派类型 2
			 * @param[in] x          Eigen 向量/矩阵（自变量）
			 * @param[in] y          Eigen 向量/矩阵（因变量）
			 * @param[in] methodName 调用方法名称，用于错误信息中显示
			 *
			 * @code
			 * Eigen::VectorXd x = Eigen::VectorXd::LinSpaced(5, 0, 4);
			 * Eigen::VectorXd y = Eigen::VectorXd::Ones(5);
			 * IntegrationHelper::ValidateInputSizes(x, y, "Trapz");
			 * @endcode
			 */
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

			/**
			 * @brief 验证辛普森积分的最小点数要求（>= 3）
			 *
			 * @details 辛普森公式要求至少 3 个控制点。如果点数不足，抛出 std::invalid_argument。
			 *
			 * @tparam T 容器类型（需支持 .size() 方法）
			 * @param[in] x          自变量容器
			 * @param[in] methodName 调用方法名称，用于异常信息中显示
			 *
			 * @throws std::invalid_argument 当点数 < 3 时抛出
			 *
			 * @code
			 * std::vector<double> x = {0, 1, 2, 3, 4};
			 * IntegrationHelper::ValidateSimpsMinimumPoints(x, "Simps"); // 通过，共 5 点
			 * @endcode
			 */
			template<typename T>
			void IntegrationHelper::ValidateSimpsMinimumPoints(const T& x, const char* methodName) {
				if (x.size() < 3) {
					std::ostringstream oss; ///< 错误信息拼接流
					oss << methodName << ": Input vector length must be >= 3, got " << x.size();
					throw std::invalid_argument(oss.str());
				}
				if (x.size() == 0) {
					std::ostringstream oss; ///< 错误信息拼接流
					oss << methodName << ": Input vector length must be >= 3, got " << x.size();
					throw std::invalid_argument(oss.str());
				}
			}

			// ============================================================================
			// 梯形积分实现 - 双精度数组版本
			// ============================================================================

			/**
			 * @brief 对双精度 std::vector 执行梯形积分
			 *
			 * @details 使用梯形公式对积分进行积分：
			 * \f$ \int y\,dx \approx \sum_{i=0}^{n-2} \frac{(x_{i+1}-x_i)(y_i+y_{i+1})}{2} \f$
			 *
			 * @param[in] x 自变量点列表（必须与 y 等长）
			 * @param[in] y 因变量点列表
			 * @return     积分结果（double）
			 *
			 * @note 调用前将自动验证 x 和 y 的长度匹配性
			 *
			 * @code
			 * std::vector<double> x = {0.0, 1.0, 2.0, 3.0};
			 * std::vector<double> y = {0.0, 1.0, 4.0, 9.0}; // y = x^2
			 * double area = IntegrationHelper::Trapz(x, y); // ≈ 9.0
			 * @endcode
			 */
			double IntegrationHelper::Trapz(const std::vector<double>& x, const std::vector<double>& y) {
				ValidateInputSizes(x, y, "Trapz");

				double integral = 0.0; ///< 积分累加器，初始化为 0
				for (size_t i = 0; i < x.size() - 1; ++i) {
					integral += (x[i + 1] - x[i]) * 0.5 * (y[i] + y[i + 1]);
				}
				return integral;
			}

			/**
			 * @brief 对双精度 std::vector 执行梯形积分（输出参数版）
			 *
			 * @details 委托给返回値版本，结果通过引用参数返回。
			 *
			 * @param[in]  x        自变量点列表
			 * @param[in]  y        因变量点列表
			 * @param[out] integral 结果写入此双精度引用参数
			 *
			 * @code
			 * double area;
			 * IntegrationHelper::Trapz(x, y, area);
			 * @endcode
			 */
			void IntegrationHelper::Trapz(const std::vector<double>& x, const std::vector<double>& y, double& integral) {
				integral = Trapz(x, y);
			}

			// ============================================================================
			// 梯形积分实现 - 单精度数组版本
			// ============================================================================

			/**
			 * @brief 对单精度 std::vector 执行梯形积分
			 *
			 * @details 与双精度版本相同，但采用单精度思年数据进行梯形积分。
			 *
			 * @param[in] x 自变量点列表（必须与 y 等长）
			 * @param[in] y 因变量点列表
			 * @return     积分结果（float）
			 *
			 * @code
			 * std::vector<float> x = {0.f, 1.f, 2.f};
			 * std::vector<float> y = {0.f, 1.f, 4.f};
			 * float area = IntegrationHelper::Trapz(x, y); // ≈ 2.5f
			 * @endcode
			 */
			float IntegrationHelper::Trapz(const std::vector<float>& x, const std::vector<float>& y) {

				ValidateInputSizes(x, y, "Trapz");

				float integral = 0.0f; ///< 积分累加器，单精度初始化为 0
				for (size_t i = 0; i < x.size() - 1; ++i) {
					integral += (x[i + 1] - x[i]) * 0.5f * (y[i] + y[i + 1]);
				}
				return integral;
			}

			/**
			 * @brief 对单精度 std::vector 执行梯形积分（输出参数版）
			 *
			 * @param[in]  x        自变量点列表
			 * @param[in]  y        因变量点列表
			 * @param[out] integral 结果写入此单精度引用参数
			 *
			 * @code
			 * float area;
			 * IntegrationHelper::Trapz(xf, yf, area);
			 * @endcode
			 */
			void IntegrationHelper::Trapz(const std::vector<float>& x, const std::vector<float>& y, float& integral) {
				integral = Trapz(x, y);
			}

			// ============================================================================
			// 梯形积分实现 - 双精度向量版本
			// ============================================================================

			/**
			 * @brief 对双精度 Eigen 列向量执行梯形积分
			 *
			 * @details 使用 Eigen 索引符号对向量进行梯形积分计算。
			 *
			 * @param[in] x 自变量列向量（必须与 y 等长）
			 * @param[in] y 因变量列向量
			 * @return     积分结果（double）
			 *
			 * @code
			 * Eigen::VectorXd x = Eigen::VectorXd::LinSpaced(5, 0, 1);
			 * Eigen::VectorXd y = x.array().square(); // y = x^2
			 * double area = IntegrationHelper::Trapz(x, y);
			 * @endcode
			 */
			double IntegrationHelper::Trapz(const Eigen::VectorXd& x, const Eigen::VectorXd& y) {
				ValidateInputSizes(x, y, "Trapz");

				double integral = 0.0; ///< 积分累加器，初始化为 0
				for (int i = 0; i < x.size() - 1; ++i) {
					integral += (x(i + 1) - x(i)) * 0.5 * (y(i) + y(i + 1));
				}
				return integral;
			}

			// ============================================================================
			// 梯形积分实现 - 单精度向量版本
			// ============================================================================

			/**
			 * @brief 对单精度 Eigen 列向量执行梯形积分
			 *
			 * @param[in] x 自变量列向量（必须与 y 等长）
			 * @param[in] y 因变量列向量
			 * @return     积分结果（float）
			 *
			 * @code
			 * Eigen::VectorXf xf = Eigen::VectorXf::LinSpaced(5, 0.f, 1.f);
			 * Eigen::VectorXf yf = xf.array().square();
			 * float area = IntegrationHelper::Trapz(xf, yf);
			 * @endcode
			 */
			float IntegrationHelper::Trapz(const Eigen::VectorXf& x, const Eigen::VectorXf& y) {
				ValidateInputSizes(x, y, "Trapz");

				float integral = 0.0f; ///< 积分累加器，单精度初始化为 0
				for (int i = 0; i < x.size() - 1; ++i) {
					integral += (x(i + 1) - x(i)) * 0.5f * (y(i) + y(i + 1));
				}
				return integral;
			}

			// ============================================================================
			// 梯形积分实现 - 双精度矩阵版本
			// ============================================================================

			/**
			 * @brief 对双精度 Eigen 矩阵按指定维度执行梯形积分
			 *
			 * @details 
			 * - dim=1：对每一列沿行方向积分，返回长度为 yCols 的向量
			 * - dim=2：对每一行沿列方向积分，返回长度为 yRows 的向量
			 *
			 * @param[in] x   自变量列向量
			 * @param[in] y   因变量矩阵
			 * @param[in] dim 积分方向：1=沿行对列积分，2=沿列对行积分
			 * @return        各列/行积分结果向量
			 *
			 * @throws std::invalid_argument 当 dim 与矩阵维度不匹配时抛出
			 *
			 * @code
			 * Eigen::VectorXd t = Eigen::VectorXd::LinSpaced(10, 0, 1);
			 * Eigen::MatrixXd Y = Eigen::MatrixXd::Random(10, 3); // 3通道
			 * Eigen::VectorXd areas = IntegrationHelper::Trapz(t, Y, 1); // 对每列积分
			 * @endcode
			 */
			Eigen::VectorXd IntegrationHelper::Trapz(const Eigen::VectorXd& x, const Eigen::MatrixXd& y, int dim) {
				const int xCount = static_cast<int>(x.size()); ///< 自变量点数
				const int yRows = static_cast<int>(y.rows());   ///< 矩阵行数
				const int yCols = static_cast<int>(y.cols());   ///< 矩阵列数

				if (dim == 1 && xCount == yRows) {
					// 沿行积分（对每一列）
					Eigen::VectorXd result = Eigen::VectorXd::Zero(yCols); ///< 各列积分结果向量
					for (int i = 0; i < yCols; ++i) {
						result(i) = Trapz(x, y.col(i));
					}
					return result;
				}
				else if (dim == 2 && xCount == yCols) {
					// 沿列积分（对每一行）
					Eigen::VectorXd result = Eigen::VectorXd::Zero(yRows); ///< 各行积分结果向量
					for (int i = 0; i < yRows; ++i) {
						result(i) = Trapz(x, y.row(i));
					}
					return result;
				}
				else {
					std::ostringstream oss; ///< 错误信息拼接流
					oss << "Trapz: Dimension mismatch. dim=" << dim
						<< ", x.size()=" << xCount
						<< ", y.rows()=" << yRows
						<< ", y.cols()=" << yCols;
					throw std::invalid_argument(oss.str());
				}
			}

			/**
			 * @brief 对双精度 Eigen 矩阵按指定维度执行梯形积分（输出参数版）
			 *
			 * @param[in]  x      自变量列向量
			 * @param[in]  y      因变量矩阵
			 * @param[in]  dim    积分方向：1=对列，2=对行
			 * @param[out] result 结果写入此向量引用
			 *
			 * @code
			 * Eigen::VectorXd areas;
			 * IntegrationHelper::Trapz(t, Y, 1, areas);
			 * @endcode
			 */
			void IntegrationHelper::Trapz(const Eigen::VectorXd& x, const Eigen::MatrixXd& y, int dim, Eigen::VectorXd& result) {
				result = Trapz(x, y, dim);
			}

			// ============================================================================
			// 梯形积分实现 - 单精度矩阵版本
			// ============================================================================

			/**
			 * @brief 对单精度 Eigen 矩阵按指定维度执行梯形积分
			 *
			 * @details 与双精度版本相同，但内部使用单精度浮点运算。
			 *
			 * @param[in] x   自变量列向量
			 * @param[in] y   因变量矩阵
			 * @param[in] dim 积分方向：1=沿行对列积分，2=沿列对行积分
			 * @return        各列/行积分结果向量（float）
			 *
			 * @throws std::invalid_argument 当 dim 与矩阵维度不匹配时抛出
			 *
			 * @code
			 * Eigen::VectorXf tf = Eigen::VectorXf::LinSpaced(10, 0.f, 1.f);
			 * Eigen::MatrixXf Yf = Eigen::MatrixXf::Random(10, 3);
			 * Eigen::VectorXf areas = IntegrationHelper::Trapz(tf, Yf, 1);
			 * @endcode
			 */
			Eigen::VectorXf IntegrationHelper::Trapz(const Eigen::VectorXf& x, const Eigen::MatrixXf& y, int dim) {
				const int xCount = static_cast<int>(x.size()); ///< 自变量点数
				const int yRows = static_cast<int>(y.rows());   ///< 矩阵行数
				const int yCols = static_cast<int>(y.cols());   ///< 矩阵列数

				if (dim == 1 && xCount == yRows) {
					// 沿行积分（对每一列）
					Eigen::VectorXf result = Eigen::VectorXf::Zero(yCols); ///< 单精度各列积分结果向量
					for (int i = 0; i < yCols; ++i) {
						result(i) = Trapz(x, y.col(i));
					}
					return result;
				}
				else if (dim == 2 && xCount == yCols) {
					// 沿列积分（对每一行）
					Eigen::VectorXf result = Eigen::VectorXf::Zero(yRows); ///< 单精度各行积分结果向量
					for (int i = 0; i < yRows; ++i) {
						result(i) = Trapz(x, y.row(i));
					}
					return result;
				}
				else {
					std::ostringstream oss; ///< 错误信息拼接流
					oss << "Trapz: Dimension mismatch. dim=" << dim
						<< ", x.size()=" << xCount
						<< ", y.rows()=" << yRows
						<< ", y.cols()=" << yCols;
					throw std::invalid_argument(oss.str());
				}
			}

			/**
			 * @brief 对单精度 Eigen 矩阵按指定维度执行梯形积分（输出参数版）
			 *
			 * @param[in]  x      自变量列向量
			 * @param[in]  y      因变量矩阵
			 * @param[in]  dim    积分方向：1=对列，2=对行
			 * @param[out] result 结果写入此单精度向量引用
			 *
			 * @code
			 * Eigen::VectorXf areas;
			 * IntegrationHelper::Trapz(tf, Yf, 1, areas);
			 * @endcode
			 */
			void IntegrationHelper::Trapz(const Eigen::VectorXf& x, const Eigen::MatrixXf& y, int dim, Eigen::VectorXf& result) {
				result = Trapz(x, y, dim);
			}

			// ============================================================================
			// 累积梯形积分实现 - 双精度版本
			// ============================================================================

			/**
			 * @brief 对双精度 Eigen 列向量执行累积梯形积分（TrapzA）
			 *
			 * @details 从最后一个区间开始反向累加梯形积分，结果向量的第 i 个元素
			 * 表示从第 i 个点到最后一个点的积分。
			 *
			 * @param[in] x 自变量列向量
			 * @param[in] y 因变量列向量
			 * @return     累积积分结果向量，第 i 项为 \f$\int_{x_i}^{x_{n-1}} y\,dx\f$
			 *
			 * @code
			 * Eigen::VectorXd t = Eigen::VectorXd::LinSpaced(5, 0, 1);
			 * Eigen::VectorXd y = t.array().square();
			 * Eigen::VectorXd cumArea = IntegrationHelper::TrapzA(t, y);
			 * // cumArea(0) ≈ 整个区间的积分
			 * @endcode
			 */
			Eigen::VectorXd IntegrationHelper::TrapzA(const Eigen::VectorXd& x, const Eigen::VectorXd& y) {
				ValidateInputSizes(x, y, "TrapzA");

				const int n = static_cast<int>(x.size()); ///< 信号点数
				Eigen::VectorXd result = Eigen::VectorXd::Zero(n); ///< 累积积分结果向量，初始化为 0

				double integral = 0.0; ///< 积分累加器，从末尾向头部累加
				for (int i = n - 2; i >= 0; --i) {
					integral += (x(i + 1) - x(i)) * 0.5 * (y(i) + y(i + 1));
					result(i) = integral;
				}

				return result;
			}

			// ============================================================================
			// 累积梯形积分实现 - 单精度版本
			// ============================================================================

			/**
			 * @brief 对单精度 Eigen 列向量执行累积梯形积分（TrapzA）
			 *
			 * @details 与双精度版本逻辑相同，使用单精度浮点运算。
			 *
			 * @param[in] x 自变量列向量
			 * @param[in] y 因变量列向量
			 * @return     单精度累积积分结果向量
			 *
			 * @code
			 * Eigen::VectorXf tf = Eigen::VectorXf::LinSpaced(5, 0.f, 1.f);
			 * Eigen::VectorXf yf = tf.array().square();
			 * Eigen::VectorXf cumArea = IntegrationHelper::TrapzA(tf, yf);
			 * @endcode
			 */
			Eigen::VectorXf IntegrationHelper::TrapzA(const Eigen::VectorXf& x, const Eigen::VectorXf& y) {
				ValidateInputSizes(x, y, "TrapzA");

				const int n = static_cast<int>(x.size()); ///< 信号点数
				Eigen::VectorXf result = Eigen::VectorXf::Zero(n); ///< 单精度累积积分结果向量

				float integral = 0.0f; ///< 单精度积分累加器
				for (int i = n - 2; i >= 0; --i) {
					integral += (x(i + 1) - x(i)) * 0.5f * (y(i) + y(i + 1));
					result(i) = integral;
				}

				return result;
			}

			// ============================================================================
			// 辛普森积分实现 - 双精度数组版本
			// ============================================================================

			/**
			 * @brief 对双精度 std::vector 执行辛普森积分
			 *
			 * @details 使用复合辛普森 1/3 公式对成对区间积分：
			 * \f$ \int y\,dx \approx \sum_{i=0,2,4,...} \frac{H}{6}(y_i + 4y_{i+1} + y_{i+2}) \f$
			 * 其中 \f$H = h_1 + h_2\f$。若点数为偶数，最后一个区间使用颍一次公式完善。
			 *
			 * @param[in] x 自变量点列表（长度 >= 3）
			 * @param[in] y 因变量点列表
			 * @return     积分结果（double）
			 *
			 * @throws std::invalid_argument 当点数 < 3 时抛出
			 *
			 * @code
			 * std::vector<double> x = {0.0, 0.5, 1.0, 1.5, 2.0};
			 * std::vector<double> y = {0.0, 0.25, 1.0, 2.25, 4.0}; // y = x^2
			 * double area = IntegrationHelper::Simps(x, y); // ≈ 2.667
			 * @endcode
			 */
			double IntegrationHelper::Simps(const std::vector<double>& x, const std::vector<double>& y) {
				ValidateInputSizes(x, y, "Simps");
				ValidateSimpsMinimumPoints(x, "Simps");

				const size_t n = x.size(); ///< 控制点数量
				double result = 0.0; ///< 积分累加器

				// 对成对的区间应用辛普森规则
				for (size_t i = 0; i < n - 2; i += 2) {
					double h1 = x[i + 1] - x[i];     ///< 左子区间步长
					double h2 = x[i + 2] - x[i + 1]; ///< 右子区间步长
					double H = h1 + h2;               ///< 合并区间总长度

					result += (H / 6.0) * (y[i] + 4.0 * y[i + 1] + y[i + 2]);
				}

				// 如果点数为偶数，处理最后一个区间
				if (n % 2 == 0) {
					size_t end = n - 1; ///< 最后一个点的索引
					double h1 = x[end - 1] - x[end - 2]; ///< 倒数第二个子区间步长
					double h2 = x[end] - x[end - 1];     ///< 最后一个子区间步长
					double H = h1 + h2;                   ///< 尾部配对区间总长

					result += (H / 6.0) * (y[end - 2] + 4.0 * y[end - 1] + y[end]);
				}

				return result;
			}

			// ============================================================================
			// 辛普森积分实现 - 双精度向量版本
			// ============================================================================

			/**
			 * @brief 对双精度 Eigen 列向量执行辛普森积分
			 *
			 * @details 与 std::vector 版本逻辑相同，使用 Eigen 索引符号访问元素。
			 *
			 * @param[in] x 双精度自变量列向量（长度 >= 3）
			 * @param[in] y 双精度因变量列向量
			 * @return     积分结果（double）
			 *
			 * @throws std::invalid_argument 当点数 < 3 时抛出
			 *
			 * @code
			 * Eigen::VectorXd x = Eigen::VectorXd::LinSpaced(5, 0, 2);
			 * Eigen::VectorXd y = x.array().square();
			 * double area = IntegrationHelper::Simps(x, y);
			 * @endcode
			 */
			double IntegrationHelper::Simps(const Eigen::VectorXd& x, const Eigen::VectorXd& y) {
				ValidateInputSizes(x, y, "Simps");
				ValidateSimpsMinimumPoints(x, "Simps");

				const size_t n = x.size(); ///< 控制点数量
				double result = 0.0; ///< 积分累加器

				// 对成对的区间应用辛普森规则
				for (size_t i = 0; i < n - 2; i += 2) {
					double h1 = x[i + 1] - x[i];     ///< 左子区间步长
					double h2 = x[i + 2] - x[i + 1]; ///< 右子区间步长
					double H = h1 + h2;               ///< 合并区间总长度

					result += (H / 6.0) * (y[i] + 4.0 * y[i + 1] + y[i + 2]);
				}

				// 如果点数为偶数，处理最后一个区间
				if (n % 2 == 0) {
					size_t end = n - 1; ///< 最后一个点的索引
					double h1 = x[end - 1] - x[end - 2]; ///< 倒数第二个子区间步长
					double h2 = x[end] - x[end - 1];     ///< 最后一个子区间步长
					double H = h1 + h2;                   ///< 尾部配对区间总长

					result += (H / 6.0) * (y[end - 2] + 4.0 * y[end - 1] + y[end]);
				}

				return result;
			}

			// ============================================================================
			// 辛普森积分实现 - 双精度矩阵版本
			// ============================================================================

			/**
			 * @brief 对双精度 Eigen 矩阵按指定维度执行辛普森积分
			 *
			 * @details 
			 * - dim=1：对每一列沿行方向积分，返回长度为 yCols 的向量
			 * - dim=2：对每一行沿列方向积分，返回长度为 yRows 的向量
			 *
			 * @param[in] x   自变量列向量（长度 >= 3）
			 * @param[in] y   因变量矩阵
			 * @param[in] dim 积分方向：1=沿行对列积分，2=沿列对行积分
			 * @return        各列/行积分结果向量
			 *
			 * @throws std::invalid_argument 当 dim 与矩阵维度不匹配，或点数 < 3 时抛出
			 *
			 * @code
			 * Eigen::VectorXd t = Eigen::VectorXd::LinSpaced(5, 0, 1);
			 * Eigen::MatrixXd Y = Eigen::MatrixXd::Random(5, 3);
			 * Eigen::VectorXd areas = IntegrationHelper::Simps(t, Y, 1);
			 * @endcode
			 */
			Eigen::VectorXd IntegrationHelper::Simps(const Eigen::VectorXd& x, const Eigen::MatrixXd& y, int dim) {
				const int xCount = static_cast<int>(x.size()); ///< 自变量点数
				const int yRows = static_cast<int>(y.rows());   ///< 矩阵行数
				const int yCols = static_cast<int>(y.cols());   ///< 矩阵列数

				if (dim == 1 && xCount == yRows) {
					// 沿行积分（对每一列）
					Eigen::VectorXd result(yCols); ///< 各列辛普森积分结果向量
					for (int i = 0; i < yCols; ++i) {
						result(i) = Simps(x, y.col(i));
					}
					return result;
				}
				else if (dim == 2 && xCount == yCols) {
					// 沿列积分（对每一行）
					Eigen::VectorXd result(yRows); ///< 各行辛普森积分结果向量
					for (int i = 0; i < yRows; ++i) {
						result(i) = Simps(x, y.row(i));
					}
					return result;
				}
				else {
					std::ostringstream oss; ///< 错误信息拼接流
					oss << "Simps: Dimension mismatch. dim=" << dim
						<< ", x.size()=" << xCount
						<< ", y.rows()=" << yRows
						<< ", y.cols()=" << yCols;
					throw std::invalid_argument(oss.str());
				}
			}

		} // namespace Math
	} // namespace IO
} // namespace Qahse
