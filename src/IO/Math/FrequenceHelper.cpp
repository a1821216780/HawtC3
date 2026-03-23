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

#ifndef FREQUENCEHELPER_H
#define FREQUENCEHELPER_H


#include "FrequenceHelper.h"
#include <algorithm>
#include <stdexcept>
#include "../Log/LogHelper.h"

using namespace HawtC3::IO::Log;

namespace HawtC3 {
    namespace IO {
        namespace Math {

            // ============================================================================
            // 内部辅助函数实现
            // ============================================================================

            void FrequenceHelper::ExecuteFFT(std::vector<std::complex<double>>& data, bool inverse) {
                const int n = static_cast<int>(data.size());
                if (n == 0) {
                    throw std::invalid_argument("输入数据不能为空");
                }

                // 创建FFTW计划
                fftw_complex* in = reinterpret_cast<fftw_complex*>(data.data());
                //fftw_complex* out = fftw_malloc(sizeof(fftw_complex) * n);
                fftw_complex* out = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * n));
                if (!out) {
                    throw std::runtime_error("FFTW内存分配失败");
                }

                fftw_plan plan;
                if (inverse) {
                    plan = fftw_plan_dft_1d(n, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
                }
                else {
                    plan = fftw_plan_dft_1d(n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
                }

                if (!plan) {
                    fftw_free(out);
                    throw std::runtime_error("FFTW计划创建失败");
                }

                // 执行FFT
                fftw_execute(plan);

                // 复制结果并应用MATLAB兼容的归一化
                if (inverse) {
                    // IFFT需要除以n以匹配MATLAB的约定
                    for (int i = 0; i < n; ++i) {
                        data[i] = std::complex<double>(out[i][0] / n, out[i][1] / n);
                    }
                }
                else {
                    // FFT不需要归一化
                    for (int i = 0; i < n; ++i) {
                        data[i] = std::complex<double>(out[i][0], out[i][1]);
                    }
                }

                // 清理
                fftw_destroy_plan(plan);
                fftw_free(out);
            }

            void FrequenceHelper::ExecuteFFT(std::vector<std::complex<float>>& data, bool inverse) {
                const int n = static_cast<int>(data.size());
                if (n == 0) {
                    throw std::invalid_argument("输入数据不能为空");
                }

                // 创建FFTW单精度计划
                fftwf_complex* in = reinterpret_cast<fftwf_complex*>(data.data());
                //fftwf_complex* out = fftwf_malloc(sizeof(fftwf_complex) * n);
                //fftw_complex* out = fftw_malloc(sizeof(fftw_complex) * n);
                fftwf_complex* out = reinterpret_cast<fftwf_complex*>(fftw_malloc(sizeof(fftw_complex) * n));

                if (!out) {
                    throw std::runtime_error("FFTW内存分配失败");
                }

                fftwf_plan plan;
                if (inverse) {
                    plan = fftwf_plan_dft_1d(n, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
                }
                else {
                    plan = fftwf_plan_dft_1d(n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
                }

                if (!plan) {
                    fftwf_free(out);
                    throw std::runtime_error("FFTW计划创建失败");
                }

                // 执行FFT
                fftwf_execute(plan);

                // 复制结果并应用MATLAB兼容的归一化
                if (inverse) {
                    // IFFT需要除以n以匹配MATLAB的约定
                    const float scale = 1.0f / n;
                    for (int i = 0; i < n; ++i) {
                        data[i] = std::complex<float>(out[i][0] * scale, out[i][1] * scale);
                    }
                }
                else {
                    // FFT不需要归一化
                    for (int i = 0; i < n; ++i) {
                        data[i] = std::complex<float>(out[i][0], out[i][1]);
                    }
                }

                // 清理
                fftwf_destroy_plan(plan);
                fftwf_free(out);
            }

            // ============================================================================
            // IFFT 实现
            // ============================================================================

            std::vector<std::complex<double>> FrequenceHelper::Ifft(const std::vector<std::complex<double>>& f) {
                // 复制输入数据
                std::vector<std::complex<double>> result = f;

                // 执行IFFT
                ExecuteFFT(result, true);

                return result;
            }

            std::vector<std::complex<float>> FrequenceHelper::Ifft(const std::vector<std::complex<float>>& f) {
                // 复制输入数据
                std::vector<std::complex<float>> result = f;

                // 执行IFFT
                ExecuteFFT(result, true);

                return result;
            }

            Eigen::VectorXcd FrequenceHelper::Ifft(const Eigen::VectorXcd& f) {
                const int n = static_cast<int>(f.size());

                // 转换为std::vector
                std::vector<std::complex<double>> data(n);
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i);
                }

                // 执行IFFT
                ExecuteFFT(data, true);

                // 转换回Eigen::VectorXcd
                Eigen::VectorXcd result(n);
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i];
                }

                return result;
            }

            Eigen::VectorXcf FrequenceHelper::Ifft(const Eigen::VectorXcf& f) {
                const int n = static_cast<int>(f.size());

                // 转换为std::vector
                std::vector<std::complex<float>> data(n);
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i);
                }

                // 执行IFFT
                ExecuteFFT(data, true);

                // 转换回Eigen::VectorXcf
                Eigen::VectorXcf result(n);
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i];
                }

                return result;
            }

            Eigen::MatrixXcd FrequenceHelper::Ifft(const Eigen::MatrixXcd& f) {
                const int rows = static_cast<int>(f.rows());
                const int cols = static_cast<int>(f.cols());

                Eigen::MatrixXcd result(rows, cols);

                // 对每一列执行IFFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcd column = f.col(col);
                    result.col(col) = Ifft(column);
                }

                return result;
            }

            Eigen::MatrixXcf FrequenceHelper::Ifft(const Eigen::MatrixXcf& f) {
                const int rows = static_cast<int>(f.rows());
                const int cols = static_cast<int>(f.cols());

                Eigen::MatrixXcf result(rows, cols);

                // 对每一列执行IFFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcf column = f.col(col);
                    result.col(col) = Ifft(column);
                }

                return result;
            }

            // ============================================================================
            // FFT 实现
            // ============================================================================

            std::vector<std::complex<double>> FrequenceHelper::Fft(const std::vector<std::complex<double>>& f) {
                // 复制输入数据
                std::vector<std::complex<double>> result = f;

                // 执行FFT
                ExecuteFFT(result, false);

                return result;
            }



            std::pair<std::vector<double>, std::vector<double>> FrequenceHelper::Fft(
                const std::vector<double>& real, const std::vector<double>& imag) {

                if (real.size() != imag.size())
                {
					LogHelper::ErrorLog("实部和虚部数组长度不匹配","","",20,"FrequenceHelper::Fft");
                }

                const size_t n = real.size();

                // 构造复数数组
                std::vector<std::complex<double>> data(n);
                for (size_t i = 0; i < n; ++i) {
                    data[i] = std::complex<double>(real[i], imag[i]);
                }

                // 执行FFT
                ExecuteFFT(data, false);

                // 分离实部和虚部
                std::vector<double> realResult(n);
                std::vector<double> imagResult(n);
                for (size_t i = 0; i < n; ++i) {
                    realResult[i] = data[i].real();
                    imagResult[i] = data[i].imag();
                }

                return { realResult, imagResult };
            }

            std::vector<std::complex<float>> FrequenceHelper::Fft(const std::vector<std::complex<float>>& f) {
                // 复制输入数据
                std::vector<std::complex<float>> result = f;

                // 执行FFT
                ExecuteFFT(result, false);

                return result;
            }

            Eigen::VectorXcd FrequenceHelper::Fft(const Eigen::VectorXcd& f) {
                const int n = static_cast<int>(f.size());

                // 转换为std::vector
                std::vector<std::complex<double>> data(n);
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i);
                }

                // 执行FFT
                ExecuteFFT(data, false);

                // 转换回Eigen::VectorXcd
                Eigen::VectorXcd result(n);
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i];
                }

                return result;
            }

            Eigen::VectorXcd FrequenceHelper::Fft(const Eigen::VectorXd& fd) {
                
				auto f = fd.cast<std::complex<double>>();
                
                const int n = static_cast<int>(f.size());

                // 转换为std::vector
                std::vector<std::complex<double>> data(n);
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i);
                }

                // 执行FFT
                ExecuteFFT(data, false);

                // 转换回Eigen::VectorXcd
                Eigen::VectorXcd result(n);
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i];
                }

                return result;
            }

            Eigen::VectorXcf FrequenceHelper::Fft(const Eigen::VectorXcf& f) {
                const int n = static_cast<int>(f.size());

                // 转换为std::vector
                std::vector<std::complex<float>> data(n);
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i);
                }

                // 执行FFT
                ExecuteFFT(data, false);

                // 转换回Eigen::VectorXcf
                Eigen::VectorXcf result(n);
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i];
                }

                return result;
            }

            Eigen::VectorXcf FrequenceHelper::Fft(const Eigen::VectorXf& fd) {

				auto f = fd.cast<std::complex<float>>();

                const int n = static_cast<int>(f.size());

                // 转换为std::vector
                std::vector<std::complex<float>> data(n);
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i);
                }

                // 执行FFT
                ExecuteFFT(data, false);

                // 转换回Eigen::VectorXcf
                Eigen::VectorXcf result(n);
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i];
                }

                return result;
            }

            Eigen::MatrixXcd FrequenceHelper::Fft(const Eigen::MatrixXcd& f) {
                const int rows = static_cast<int>(f.rows());
                const int cols = static_cast<int>(f.cols());

                Eigen::MatrixXcd result(rows, cols);

                // 对每一列执行FFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcd column = f.col(col);
                    result.col(col) = Fft(column);
                }

                return result;
            }

            Eigen::MatrixXcd FrequenceHelper::Fft(const Eigen::MatrixXd& fd) {
				auto f = fd.cast<std::complex<double>>();
                const int rows = static_cast<int>(f.rows());
                const int cols = static_cast<int>(f.cols());

                Eigen::MatrixXcd result(rows, cols);

                // 对每一列执行FFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcd column = f.col(col);
                    result.col(col) = Fft(column);
                }

                return result;
            }


            Eigen::MatrixXcf FrequenceHelper::Fft(const Eigen::MatrixXcf& f) {
                const int rows = static_cast<int>(f.rows());
                const int cols = static_cast<int>(f.cols());

                Eigen::MatrixXcf result(rows, cols);

                // 对每一列执行FFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcf column = f.col(col);
                    result.col(col) = Fft(column);
                }

                return result;
            }


            Eigen::MatrixXcf FrequenceHelper::Fft(const Eigen::MatrixXf& fd) {
				auto f = fd.cast<std::complex<float>>();
                const int rows = static_cast<int>(f.rows());
                const int cols = static_cast<int>(f.cols());

                Eigen::MatrixXcf result(rows, cols);

                // 对每一列执行FFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcf column = f.col(col);
                    result.col(col) = Fft(column);
                }

                return result;
            }

        } // namespace Math
    } // namespace IO
} // namespace HawtC3

#endif // FREQUENCEHELPER_H
