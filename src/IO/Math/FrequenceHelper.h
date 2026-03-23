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

#include <complex>
#include <vector>
#include <Eigen/Dense>
#include <fftw/fftw3.h>

namespace HawtC3 {
	namespace IO {
		namespace Math {

			/// <summary>
			/// 提供执行快速傅里叶变换（FFT）和逆快速傅里叶变换（IFFT）运算的方法。
			/// 使用与MATLAB兼容的算法实现。此类支持各种数据类型，包括数组、向量和矩阵形式的
			/// 复数数据处理。
			/// </summary>
			/// <remarks>
			/// 此类中的方法利用FFTW3库执行FFT和IFFT操作。
			/// 这些操作使用与MATLAB兼容的傅里叶变换选项实现。支持的数据类型包括：
			/// - std::complex<double>数组（std::vector）
			/// - std::complex<float>数组（std::vector）
			/// - Eigen::VectorXcd（对于std::complex<double>类型）
			/// - Eigen::VectorXcf（对于std::complex<float>类型）
			/// - Eigen::MatrixXcd（对于std::complex<double>类型）
			/// - Eigen::MatrixXcf（对于std::complex<float>类型）
			/// 
			/// FFT方法计算前向傅里叶变换，将数据从时域转换到频域。
			/// IFFT方法计算逆傅里叶变换，将数据从频域转换回时域。
			/// 
			/// <para><strong>典型使用场景示例:</strong></para>
			/// <code>
			/// // 信号处理：分析时域信号的频率成分
			/// std::vector<std::complex<double>> timeSignal = { /* 时域数据 */ };
			/// auto spectrum = FrequenceHelper::Fft(timeSignal);
			/// 
			/// // 滤波操作：在频域中进行滤波处理
			/// auto filteredSpectrum = ApplyFilter(spectrum);
			/// auto filteredSignal = FrequenceHelper::Ifft(filteredSpectrum);
			/// 
			/// // 批量处理：同时处理多个信号
			/// Eigen::MatrixXcd multipleSignals(samples, channels);
			/// auto spectra = FrequenceHelper::Fft(multipleSignals);
			/// </code>
			/// </remarks>
			class FrequenceHelper {
			public:
				// 禁用构造函数，这是一个静态工具类
				FrequenceHelper() = delete;
				~FrequenceHelper() = delete;
				FrequenceHelper(const FrequenceHelper&) = delete;
				FrequenceHelper& operator=(const FrequenceHelper&) = delete;

#pragma region IFFT

				/// <summary>
				/// 计算指定复数数组的逆快速傅里叶变换(IFFT)，将频域数据转换回时域。
				/// </summary>
				/// <remarks>
				/// 输入数组f不会被修改，该方法返回一个包含IFFT结果的新数组。
				/// IFFT使用与MATLAB兼容的傅里叶变换选项计算，保持与MATLAB计算结果的一致性。
				/// 
				/// <para><strong>使用示例:</strong></para>
				/// <code>
				/// // 创建一个频域数据数组（例如FFT的输出）
				/// std::vector<std::complex<double>> frequencyData = { 
				///     {10, 0},    // 直流分量
				///     {5, -3},    // 正频率分量
				///     {0, 0},     // 零分量
				///     {5, 3}      // 负频率分量（共轭对称）
				/// };
				/// 
				/// // 计算IFFT获取时域数据
				/// auto timeData = FrequenceHelper::Ifft(frequencyData);
				/// 
				/// // 输出结果，时域信号已恢复
				/// std::cout << "重构的时域信号包含 " << timeData.size() << " 个样本\n";
				/// for (size_t i = 0; i < timeData.size(); i++)
				/// {
				///     std::cout << "t[" << i << "] = " << timeData[i].real() 
				///               << " + " << timeData[i].imag() << "i\n";
				/// }
				/// </code>
				/// 
				/// <para><strong>性能提示:</strong></para>
				/// 当输入数组长度为2的幂次方时，FFT算法性能最佳。建议使用长度为 2^n 的数组。
				/// </remarks>
				/// <param name="f">一系列std::complex<double>代表频域数据的复数。输入数组必须有效，且长度为2的幂次方时计算效率最高。</param>
				/// <returns>一系列std::complex<double>代表时域数据的复数，长度与输入数组相同。</returns>
				static std::vector<std::complex<double>> Ifft(const std::vector<std::complex<double>>& f);

				/// <summary>
				/// 计算指定float复数数组的逆快速傅里叶变换(IFFT)，将频域数据转换回时域。
				/// </summary>
				/// <param name="f">一个表示频域数据的std::complex<float>数组。输入数组长度最好是2的幂次方以获得最佳性能。</param>
				/// <returns>包含通过应用IFFT获得的时域数据的std::complex<float>数组。返回数组的长度与输入数组相同。</returns>
				static std::vector<std::complex<float>> Ifft(const std::vector<std::complex<float>>& f);

				/// <summary>
				/// 计算指定复数向量的逆快速傅里叶变换(IFFT)，将频域数据转换为时域数据。
				/// </summary>
				/// <param name="f">表示频域数据的复数向量。输入向量的长度应该为2的幂次方以获得最佳性能。</param>
				/// <returns>表示时域数据的复数向量，应用IFFT后的结果。返回向量的长度与输入向量相同。</returns>
				static Eigen::VectorXcd Ifft(const Eigen::VectorXcd& f);



				/// <summary>
				/// 计算float类型的复数向量的逆快速傅里叶变换(IFFT)，将频域数据转换为时域数据。
				/// </summary>
				/// <param name="f">一个表示频域数据的float复数向量。向量长度最好是2的幂次方以获得最佳计算效率。</param>
				/// <returns>一个表示通过应用IFFT获得的时域数据的float复数向量。返回向量的长度与输入向量相同。</returns>
				static Eigen::VectorXcf Ifft(const Eigen::VectorXcf& f);

				/// <summary>
				/// 计算指定复数矩阵中每一列的逆快速傅里叶变换(IFFT)，将频域数据转换为时域数据。
				/// </summary>
				/// <param name="f">一个复数矩阵，其中每一列代表频域中的一个信号。矩阵的行数最好是2的幂次方以获得最佳性能。</param>
				/// <returns>一个复数矩阵，其中每一列代表相应的时域中的信号。返回矩阵的维度与输入矩阵相同。</returns>
				static Eigen::MatrixXcd Ifft(const Eigen::MatrixXcd& f);

				/// <summary>
				/// 计算指定float复数矩阵中每一列的快速傅里叶逆变换(IFFT)，将频域数据转换为时域数据。
				/// </summary>
				/// <param name="f">一个矩阵，其中每一列代表一个复频域数据的序列。矩阵的行数最好是2的幂次方以获得最佳计算效率。</param>
				/// <returns>
				/// 返回一个矩阵，其中每一列包含输入矩阵中相应列的时域表示。
				/// 返回矩阵的维度与输入矩阵相同。
				/// </returns>
				static Eigen::MatrixXcf Ifft(const Eigen::MatrixXcf& f);

#pragma endregion

#pragma region FFT

				/// <summary>
				/// 计算指定复数输入数组的快速傅里叶变换(FFT)，将时域数据转换为频域数据。
				/// </summary>
				/// <param name="f">表示时域输入信号的复数数组。数组长度最好是2的幂次方以获得最佳计算效率。</param>
				/// <returns>返回一个表示频域中变换后信号的复数数组。返回的数组与输入数组长度相同。</returns>
				static std::vector<std::complex<double>> Fft(const std::vector<std::complex<double>>& f);

				/// <summary>
				/// 计算给定实部和虚部数组的快速傅里叶变换(FFT)，将时域数据转换为频域数据。
				/// </summary>
				/// <param name="real">输入信号的实部数组。数组长度最好是2的幂次方。</param>
				/// <param name="imag">输入信号的虚部数组。必须与实部数组长度相同。</param>
				/// <returns>包含两个数组的pair：变换后的实部数组和变换后的虚部数组。</returns>
				static std::pair<std::vector<double>, std::vector<double>> Fft(
					const std::vector<double>& real, const std::vector<double>& imag);

				/// <summary>
				/// 计算指定float复数输入数组的快速傅里叶变换(FFT)，将时域数据转换为频域数据。
				/// </summary>
				/// <param name="f">一个表示时域中输入信号的std::complex<float>值数组。
				/// 数组长度最好是2的幂次方以获得最佳计算效率。</param>
				/// <returns>一个表示变换后频域信号的std::complex<float>值数组。
				/// 返回的数组与输入数组的长度相同。</returns>
				static std::vector<std::complex<float>> Fft(const std::vector<std::complex<float>>& f);

				/// <summary>
				/// 计算给定复数向量的快速傅里叶变换(FFT)，将时域数据转换为频域数据。
				/// </summary>
				/// <param name="f">要转换的复数输入向量。不能为null。向量长度最好是2的幂次方以获得最佳性能。</param>
				/// <returns>返回一个表示傅里叶变换后频域数据的复数向量。返回向量的长度与输入向量的长度相同。</returns>
				static Eigen::VectorXcd Fft(const Eigen::VectorXcd& f);

				/// <summary>
					/// 计算给定实数向量的快速傅里叶变换(FFT)，将时域数据转换为频域数据。
					/// </summary>
					/// <param name="f">要转换的复数输入向量。不能为null。向量长度最好是2的幂次方以获得最佳性能。</param>
					/// <returns>返回一个表示傅里叶变换后频域数据的复数向量。返回向量的长度与输入向量的长度相同。</returns>
				static Eigen::VectorXcd Fft(const Eigen::VectorXd& f);

				/// <summary>
				/// 计算输入float复数向量的快速傅里叶变换(FFT)，将时域数据转换为频域数据。
				/// </summary>
				/// <param name="f">输入向量，表示时域中的float复数信号。向量长度最好是2的幂次方以获得最佳性能。</param>
				/// <returns>一个表示频域中信号的float复数向量。返回向量的维度与输入向量相同。</returns>
				static Eigen::VectorXcf Fft(const Eigen::VectorXcf& f);

				/// <summary>
/// 计算输入float实数向量的快速傅里叶变换(FFT)，将时域数据转换为频域数据。
/// </summary>
/// <param name="f">输入向量，表示时域中的float复数信号。向量长度最好是2的幂次方以获得最佳性能。</param>
/// <returns>一个表示频域中信号的float复数向量。返回向量的维度与输入向量相同。</returns>
				static Eigen::VectorXcf Fft(const Eigen::VectorXf& f);

				/// <summary>
				/// 计算指定复数矩阵中每一列的快速傅里叶变换(FFT)，将时域数据转换为频域数据。
				/// </summary>
				/// <param name="f">复数矩阵，其中每列代表一个要转换的时域信号。矩阵的行数最好是2的幂次方以获得最佳性能。</param>
				/// <returns>复数矩阵，其中每列包含输入中对应列的FFT结果，表示频域中的信号。
				/// 返回矩阵的维度与输入矩阵相同。</returns>
				static Eigen::MatrixXcd Fft(const Eigen::MatrixXcd& f);


				/// <summary>
/// 计算指定实数矩阵中每一列的快速傅里叶变换(FFT)，将时域数据转换为频域数据。
/// </summary>
/// <param name="f">复数矩阵，其中每列代表一个要转换的时域信号。矩阵的行数最好是2的幂次方以获得最佳性能。</param>
/// <returns>复数矩阵，其中每列包含输入中对应列的FFT结果，表示频域中的信号。
/// 返回矩阵的维度与输入矩阵相同。</returns>
				static Eigen::MatrixXcd Fft(const Eigen::MatrixXd& f);

				/// <summary>
				/// 计算指定float复数矩阵中每一列的快速傅里叶变换(FFT)，将时域数据转换为频域数据。
				/// </summary>
				/// <param name="f">复数矩阵，其中每列代表一个要转换的时域信号。矩阵的行数最好是2的幂次方以获得最佳性能。</param>
				/// <returns>复数矩阵，其中每列包含输入中对应列的FFT结果，表示频域中的信号。
				/// 返回矩阵的维度与输入矩阵相同。</returns>
				static Eigen::MatrixXcf Fft(const Eigen::MatrixXcf& f);


				/// <summary>
/// 计算指定float实数矩阵中每一列的快速傅里叶变换(FFT)，将时域数据转换为频域数据。
/// </summary>
/// <param name="f">复数矩阵，其中每列代表一个要转换的时域信号。矩阵的行数最好是2的幂次方以获得最佳性能。</param>
/// <returns>复数矩阵，其中每列包含输入中对应列的FFT结果，表示频域中的信号。
/// 返回矩阵的维度与输入矩阵相同。</returns>
				static Eigen::MatrixXcf Fft(const Eigen::MatrixXf& f);

#pragma endregion

			private:
				// 内部辅助函数：执行FFT
					/// <summary>
	/// 执行双精度复数的FFT变换（内部实现）
	/// </summary>
				static void ExecuteFFT(std::vector<std::complex<double>>& data, bool inverse);

				/// <summary>
				/// 执行单精度复数的FFT变换（内部实现）
				/// </summary>
				static void ExecuteFFT(std::vector<std::complex<float>>& data, bool inverse);
			};

		} // namespace Math
	} // namespace IO
} // namespace HawtC3

