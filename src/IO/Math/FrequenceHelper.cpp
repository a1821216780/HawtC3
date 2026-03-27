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
#pragma once

#ifndef FREQUENCEHELPER_H
#define FREQUENCEHELPER_H


#include "FrequenceHelper.h"
#include <algorithm>
#include <stdexcept>
#include "../Log/LogHelper.h"

using namespace Qahse::IO::Log;

namespace Qahse {
    namespace IO {
        namespace Math {

            // ============================================================================
            // 内部辅助函数实现
            // ============================================================================

            /**
             * @brief 底层双精度复数 FFT/IFFT 执行函数（FFTW 封装）
             *
             * @details 使用 FFTW 库对复数向量执行原位一维离散傅里叶变换或逆变换。
             * 结果遵循 MATLAB 归一化约定：
             *   - 正变换（FFT）：不归一化
             *   - 逆变换（IFFT）：除以信号长度 n
             *
             * @param[in,out] data    输入/输出复数向量，原位修改
             * @param[in]     inverse true 表示执行 IFFT，false 表示执行 FFT
             *
             * @throws std::invalid_argument 当输入向量为空时抛出
             *
             * @note 若 FFTW 内存分配或计划创建失败，将调用 LogHelper::ErrorLog 中止程序
             *
             * @code
             * std::vector<std::complex<double>> x = {{1,0},{2,0},{3,0},{4,0}};
             * FrequenceHelper::ExecuteFFT(x, false); // 正变换
             * FrequenceHelper::ExecuteFFT(x, true);  // 再逆变换，还原原始信号
             * @endcode
             */
            void FrequenceHelper::ExecuteFFT(std::vector<std::complex<double>>& data, bool inverse) {
                const int n = static_cast<int>(data.size()); ///< 信号特征点数量
                if (n == 0) {
                    throw std::invalid_argument("输入数据不能为空");
                }

                // 创建FFTW计划
                fftw_complex* in = reinterpret_cast<fftw_complex*>(data.data()); ///< FFTW输入指针，直接复用 data 内存（零拷贝）
                //fftw_complex* out = fftw_malloc(sizeof(fftw_complex) * n);
                fftw_complex* out = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * n)); ///< FFTW对齐输出缓冲区（fftw_malloc分配）
                if (!out) {
                    LogHelper::ErrorLog("FFTW内存分配失败");
                }

                fftw_plan plan; ///< FFTW执行计划句柄
                if (inverse) {
                    plan = fftw_plan_dft_1d(n, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
                }
                else {
                    plan = fftw_plan_dft_1d(n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
                }

                if (!plan) {
                    fftw_free(out);
                    LogHelper::ErrorLog("FFTW计划创建失败");
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

            /**
             * @brief 底层单精度复数 FFT/IFFT 执行函数（FFTW 单精度封装）
             *
             * @details 与双精度版本功能相同，但使用 FFTW 单精度 API（fftwf_*）处理
             * `std::complex<float>` 数据，内存分配使用 `fftw_malloc`（对齐要求）。
             *
             * @param[in,out] data    输入/输出单精度复数向量，原位修改
             * @param[in]     inverse true 表示执行 IFFT，false 表示执行 FFT
             *
             * @throws std::invalid_argument 当输入向量为空时抛出
             *
             * @note 若 FFTW 内存分配或计划创建失败，将调用 LogHelper::ErrorLog 中止程序
             *
             * @code
             * std::vector<std::complex<float>> x = {{1.f,0.f},{2.f,0.f},{3.f,0.f},{4.f,0.f}};
             * FrequenceHelper::ExecuteFFT(x, false); // 单精度正变换
             * @endcode
             */
            void FrequenceHelper::ExecuteFFT(std::vector<std::complex<float>>& data, bool inverse) {
                const int n = static_cast<int>(data.size()); ///< 信号特征点数量
                if (n == 0) {
                    throw std::invalid_argument("输入数据不能为空");
                }

                // 创建FFTW单精度计划
                fftwf_complex* in = reinterpret_cast<fftwf_complex*>(data.data()); ///< FFTW单精度输入指针，直接复用 data 内存
                //fftwf_complex* out = fftwf_malloc(sizeof(fftwf_complex) * n);
                //fftw_complex* out = fftw_malloc(sizeof(fftw_complex) * n);
                fftwf_complex* out = reinterpret_cast<fftwf_complex*>(fftw_malloc(sizeof(fftw_complex) * n)); ///< FFTW单精度对齐输出缓冲区

                if (!out) {
                    LogHelper::ErrorLog("FFTW内存分配失败");
                }

                fftwf_plan plan; ///< FFTW单精度执行计划句柄
                if (inverse) {
                    plan = fftwf_plan_dft_1d(n, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
                }
                else {
                    plan = fftwf_plan_dft_1d(n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
                }

                if (!plan) {
                    fftwf_free(out);
                    LogHelper::ErrorLog("FFTW计划创建失败");
                }

                // 执行FFT
                fftwf_execute(plan);

                // 复制结果并应用MATLAB兼容的归一化
                if (inverse) {
                    // IFFT需要除以n以匹配MATLAB的约定
                    const float scale = 1.0f / n; ///< IFFT归一化系数（1/n），与 MATLAB 约定一致
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

            /**
             * @brief 对双精度复数向量执行逆快速傅里叶变换（IFFT）
             *
             * @details 复制输入数据后调用 ExecuteFFT(true) 执行 IFFT，
             * 结果遵循 MATLAB 归一化约定（除以信号长度 n）。
             *
             * @param[in] f 频域双精度复数向量
             * @return     时域双精度复数向量
             *
             * @throws std::invalid_argument 当输入向量为空时抛出（由 ExecuteFFT 抛出）
             *
             * @code
             * std::vector<std::complex<double>> x = {{1,0},{2,0},{3,0},{4,0}};
             * auto F = FrequenceHelper::Fft(x);       // 正变换
             * auto xr = FrequenceHelper::Ifft(F);     // 逆变换，xr ≈ x
             * @endcode
             */
            std::vector<std::complex<double>> FrequenceHelper::Ifft(const std::vector<std::complex<double>>& f) {
                // 复制输入数据
                std::vector<std::complex<double>> result = f; ///< 输入数据的副本，IFFT 将原位修改此缓冲区

                // 执行IFFT
                ExecuteFFT(result, true);

                return result;
            }

            /**
             * @brief 对单精度复数向量执行逆快速傅里叶变换（IFFT）
             *
             * @details 复制输入数据后调用单精度 ExecuteFFT(true) 执行 IFFT。
             *
             * @param[in] f 频域单精度复数向量
             * @return     时域单精度复数向量
             *
             * @throws std::invalid_argument 当输入向量为空时抛出（由 ExecuteFFT 抛出）
             *
             * @code
             * std::vector<std::complex<float>> F = FrequenceHelper::Fft(xf);
             * auto xr = FrequenceHelper::Ifft(F); // 单精度逆变换
             * @endcode
             */
            std::vector<std::complex<float>> FrequenceHelper::Ifft(const std::vector<std::complex<float>>& f) {
                // 复制输入数据
                std::vector<std::complex<float>> result = f; ///< 输入数据的副本，IFFT 将原位修改此缓冲区

                // 执行IFFT
                ExecuteFFT(result, true);

                return result;
            }

            /**
             * @brief 对双精度 Eigen 复数列向量执行 IFFT
             *
             * @details 将 Eigen::VectorXcd 转换为 std::vector，调用 ExecuteFFT(true)，
             * 再将结果转换回 Eigen::VectorXcd。
             *
             * @param[in] f 频域 Eigen 双精度复数列向量
             * @return     时域 Eigen 双精度复数列向量
             *
             * @throws std::invalid_argument 当输入向量为空时抛出
             *
             * @code
             * Eigen::VectorXcd x(4);
             * x << std::complex<double>(1,0), std::complex<double>(0,0),
             *      std::complex<double>(-1,0), std::complex<double>(0,0);
             * auto F = FrequenceHelper::Fft(x);
             * auto xr = FrequenceHelper::Ifft(F); // xr ≈ x
             * @endcode
             */
            Eigen::VectorXcd FrequenceHelper::Ifft(const Eigen::VectorXcd& f) {
                const int n = static_cast<int>(f.size()); ///< 信号点数

                // 转换为std::vector
                std::vector<std::complex<double>> data(n); ///< 转换中间缓冲区（Eigen → std::vector）
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i);
                }

                // 执行IFFT
                ExecuteFFT(data, true);

                // 转换回Eigen::VectorXcd
                Eigen::VectorXcd result(n); ///< IFFT输出复数向量
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i];
                }

                return result;
            }

            /**
             * @brief 对单精度 Eigen 复数列向量执行 IFFT
             *
             * @details 将 Eigen::VectorXcf 转换为 std::vector<complex<float>>，
             * 调用单精度 ExecuteFFT(true)，再转换回 Eigen::VectorXcf。
             *
             * @param[in] f 频域 Eigen 单精度复数列向量
             * @return     时域 Eigen 单精度复数列向量
             *
             * @throws std::invalid_argument 当输入向量为空时抛出
             *
             * @code
             * Eigen::VectorXcf xf(4);
             * auto Ff = FrequenceHelper::Fft(xf);
             * auto xr = FrequenceHelper::Ifft(Ff); // 单精度逆变换，xr ≈ xf
             * @endcode
             */
            Eigen::VectorXcf FrequenceHelper::Ifft(const Eigen::VectorXcf& f) {
                const int n = static_cast<int>(f.size()); ///< 信号点数

                // 转换为std::vector
                std::vector<std::complex<float>> data(n); ///< 单精度转换中间缓冲区（Eigen → std::vector）
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i);
                }

                // 执行IFFT
                ExecuteFFT(data, true);

                // 转换回Eigen::VectorXcf
                Eigen::VectorXcf result(n); ///< IFFT单精度输出向量
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i];
                }

                return result;
            }

            /**
             * @brief 对双精度 Eigen 复数矩阵逐列执行 IFFT
             *
             * @details 对矩阵的每一列独立调用 Ifft(VectorXcd)，
             * 结果按列写入输出矩阵，适用于多通道频域信号的批量逆变换。
             *
             * @param[in] f 频域双精度复数矩阵，每列为一路信号
             * @return     时域双精度复数矩阵
             *
             * @code
             * // 3通道256点频谱逆变换示例
             * Eigen::MatrixXcd F = FrequenceHelper::Fft(X); // X 为时域矩阵
             * Eigen::MatrixXcd xr = FrequenceHelper::Ifft(F); // xr ≈ X
             * @endcode
             */
            Eigen::MatrixXcd FrequenceHelper::Ifft(const Eigen::MatrixXcd& f) {
                const int rows = static_cast<int>(f.rows()); ///< 矩阵行数（每列信号点数）
                const int cols = static_cast<int>(f.cols()); ///< 矩阵列数（信号通道数）

                Eigen::MatrixXcd result(rows, cols); ///< IFFT输出复数矩阵

                // 对每一列执行IFFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcd column = f.col(col); ///< 当前列的频域向量
                    result.col(col) = Ifft(column);
                }

                return result;
            }

            /**
             * @brief 对单精度 Eigen 复数矩阵逐列执行 IFFT
             *
             * @details 对矩阵每列独立调用 Ifft(VectorXcf)，
             * 适用于多通道单精度频域信号的批量逆变换。
             *
             * @param[in] f 频域单精度复数矩阵，每列为一路信号
             * @return     时域单精度复数矩阵
             *
             * @code
             * Eigen::MatrixXcf Ff = FrequenceHelper::Fft(Xf); // Xf 为单精度时域矩阵
             * Eigen::MatrixXcf xr = FrequenceHelper::Ifft(Ff); // 批量逆变换
             * @endcode
             */
            Eigen::MatrixXcf FrequenceHelper::Ifft(const Eigen::MatrixXcf& f) {
                const int rows = static_cast<int>(f.rows()); ///< 矩阵行数（每列信号点数）
                const int cols = static_cast<int>(f.cols()); ///< 矩阵列数（信号通道数）

                Eigen::MatrixXcf result(rows, cols); ///< IFFT单精度输出矩阵

                // 对每一列执行IFFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcf column = f.col(col); ///< 当前列的单精度频域向量
                    result.col(col) = Ifft(column);
                }

                return result;
            }

            // ============================================================================
            // FFT 实现
            // ============================================================================

            /**
             * @brief 对双精度复数向量执行快速傅里叶变换（FFT）
             *
             * @details 复制输入后调用 ExecuteFFT(false) 执行正向 FFT，
             * 不归一化，与 MATLAB 的 fft() 行为一致。
             *
             * @param[in] f 时域双精度复数向量
             * @return     频域双精度复数向量
             *
             * @throws std::invalid_argument 当输入向量为空时抛出
             *
             * @code
             * std::vector<std::complex<double>> x = {{1,0},{0,0},{-1,0},{0,0}};
             * auto F = FrequenceHelper::Fft(x); // 计算频谱
             * @endcode
             */
            std::vector<std::complex<double>> FrequenceHelper::Fft(const std::vector<std::complex<double>>& f) {
                // 复制输入数据
                std::vector<std::complex<double>> result = f; ///< 输入数据的副本，FFT 将原位修改此缓冲区

                // 执行FFT
                ExecuteFFT(result, false);

                return result;
            }



            /**
             * @brief 由实部和虚部分离的双精度数组执行 FFT
             *
             * @details 将实部与虚部合并为 std::complex<double> 数组后执行 FFT，
             * 再分离结果的实部和虚部作为 pair 返回。
             *
             * @param[in] real 时域实部向量
             * @param[in] imag 时域虚部向量（必须与 real 等长）
             * @return         std::pair<实部向量, 虚部向量>，为对应频域分离结果
             *
             * @note 若 real.size() != imag.size()，将调用 LogHelper::ErrorLog 中止程序
             * @throws std::invalid_argument 当输入为空时抛出
             *
             * @code
             * std::vector<double> re = {1,2,3,4}, im = {0,0,0,0};
             * auto [Fre, Fim] = FrequenceHelper::Fft(re, im);
             * @endcode
             */
            std::pair<std::vector<double>, std::vector<double>> FrequenceHelper::Fft(
                const std::vector<double>& real, const std::vector<double>& imag) {

                if (real.size() != imag.size())
                {
					LogHelper::ErrorLog("实部和虚部数组长度不匹配","","",20,"FrequenceHelper::Fft");
                }

                const size_t n = real.size(); ///< 信号样本点数

                // 构造复数数组
                std::vector<std::complex<double>> data(n); ///< 由实部和虚部合并的复数中间数组
                for (size_t i = 0; i < n; ++i) {
                    data[i] = std::complex<double>(real[i], imag[i]);
                }

                // 执行FFT
                ExecuteFFT(data, false);

                // 分离实部和虚部
                std::vector<double> realResult(n); ///< FFT结果实部
                std::vector<double> imagResult(n); ///< FFT结果虚部
                for (size_t i = 0; i < n; ++i) {
                    realResult[i] = data[i].real();
                    imagResult[i] = data[i].imag();
                }

                return { realResult, imagResult };
            }

            /**
             * @brief 对单精度复数向量执行 FFT
             *
             * @details 复制输入后调用单精度 ExecuteFFT(false) 执行正向 FFT。
             *
             * @param[in] f 时域单精度复数向量
             * @return     频域单精度复数向量
             *
             * @throws std::invalid_argument 当输入向量为空时抛出
             *
             * @code
             * std::vector<std::complex<float>> xf = {{1.f,0.f},{2.f,0.f},{3.f,0.f},{4.f,0.f}};
             * auto Ff = FrequenceHelper::Fft(xf); // 单精度复数 FFT
             * @endcode
             */
            std::vector<std::complex<float>> FrequenceHelper::Fft(const std::vector<std::complex<float>>& f) {
                // 复制输入数据
                std::vector<std::complex<float>> result = f; ///< 输入数据的单精度副本，FFT 将原位修改此缓冲区

                // 执行FFT
                ExecuteFFT(result, false);

                return result;
            }

            /**
             * @brief 对双精度 Eigen 复数列向量执行 FFT
             *
             * @details 将 Eigen::VectorXcd 转换为 std::vector，调用 ExecuteFFT(false)，
             * 再将结果转换回 Eigen::VectorXcd。
             *
             * @param[in] f 时域双精度 Eigen 复数列向量
             * @return     频域双精度 Eigen 复数列向量
             *
             * @throws std::invalid_argument 当输入向量为空时抛出
             *
             * @code
             * Eigen::VectorXcd x(4);
             * x << std::complex<double>(1,0), std::complex<double>(2,0),
             *      std::complex<double>(3,0), std::complex<double>(4,0);
             * Eigen::VectorXcd F = FrequenceHelper::Fft(x); // 计算 Eigen 向量频谱
             * @endcode
             */
            Eigen::VectorXcd FrequenceHelper::Fft(const Eigen::VectorXcd& f) {
                const int n = static_cast<int>(f.size()); ///< 信号点数

                // 转换为std::vector
                std::vector<std::complex<double>> data(n); ///< 转换中间缓冲区（Eigen → std::vector）
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i);
                }

                // 执行FFT
                ExecuteFFT(data, false);

                // 转换回Eigen::VectorXcd
                Eigen::VectorXcd result(n); ///< FFT输出复数向量
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i];
                }

                return result;
            }

            /**
             * @brief 对实数值双精度 Eigen 列向量执行 FFT
             *
             * @details 将 VectorXd 转换为复数向量（虚部补零），然后执行 FFT。
             * 返回值为共轭对称的频域复数向量。
             *
             * @param[in] fd 时域双精度实数列向量
             * @return      频域双精度复数列向量
             *
             * @throws std::invalid_argument 当输入向量为空时抛出
             *
             * @code
             * Eigen::VectorXd x = Eigen::VectorXd::LinSpaced(8, 0, 7);
             * Eigen::VectorXcd F = FrequenceHelper::Fft(x); // 实数信号 FFT
             * @endcode
             */
            Eigen::VectorXcd FrequenceHelper::Fft(const Eigen::VectorXd& fd) {
                
				auto f = fd.cast<std::complex<double>>(); ///< 将实数向量转换为复数向量（虚部自动补零）
                
                const int n = static_cast<int>(f.size()); ///< 信号点数

                // 转换为std::vector
                std::vector<std::complex<double>> data(n); ///< 转换中间缓冲区（Eigen → std::vector）
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i); ///< 复制复数向量元素
                }

                // 执行 FFT
                ExecuteFFT(data, false);

                // 转换回 Eigen::VectorXcd
                Eigen::VectorXcd result(n); ///< FFT输出复数向量
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i]; ///< 将 FFT 结果写入 Eigen 复数向量
                }

                return result;
            }

            /**
             * @brief 对单精度复数 Eigen 列向量执行 FFT
             *
             * @param f 单精度时域复数列向量
             * @return 单精度频域复数列向量
             *
             * @code
             * Eigen::VectorXcf sf(4);
             * sf << std::complex<float>(1,0), std::complex<float>(2,0),
             *       std::complex<float>(3,0), std::complex<float>(4,0);
             * auto Sf = FrequenceHelper::Fft(sf);
             * @endcode
             */
            Eigen::VectorXcf FrequenceHelper::Fft(const Eigen::VectorXcf& f) {
                const int n = static_cast<int>(f.size()); ///< 信号点数

                // 转换为std::vector
                std::vector<std::complex<float>> data(n); ///< 单精度转换中间缓冲区（Eigen → std::vector）
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i);
                }

                // 执行FFT
                ExecuteFFT(data, false);

                // 转换回Eigen::VectorXcf
                Eigen::VectorXcf result(n); ///< FFT单精度输出向量
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i];
                }

                return result;
            }

            /**
             * @brief 对实数值单精度 Eigen 列向量执行 FFT
             *
             * @details 将 VectorXf 转换为单精度复数向量（虚部补零），然后执行 FFT。
             *
             * @param[in] fd 时域单精度实数列向量
             * @return      频域单精度复数列向量
             *
             * @throws std::invalid_argument 当输入向量为空时抛出
             *
             * @code
             * Eigen::VectorXf x = Eigen::VectorXf::LinSpaced(8, 0.f, 7.f);
             * Eigen::VectorXcf F = FrequenceHelper::Fft(x); // 单精度实数信号 FFT
             * @endcode
             */
            Eigen::VectorXcf FrequenceHelper::Fft(const Eigen::VectorXf& fd) {

				auto f = fd.cast<std::complex<float>>(); ///< 将单精度实数向量转换为单精度复数向量（虚部补零）

                const int n = static_cast<int>(f.size()); ///< 信号点数

                // 转换为std::vector
                std::vector<std::complex<float>> data(n); ///< 单精度转换中间缓冲区（Eigen → std::vector）
                for (int i = 0; i < n; ++i) {
                    data[i] = f(i);
                }

                // 执行FFT
                ExecuteFFT(data, false);

                // 转换回Eigen::VectorXcf
                Eigen::VectorXcf result(n); ///< FFT单精度输出向量
                for (int i = 0; i < n; ++i) {
                    result(i) = data[i];
                }

                return result;
            }

            /**
             * @brief 对双精度 Eigen 复数矩阵逐列执行 FFT
             *
             * @details 对矩阵的每一列独立调用 Fft(VectorXcd)，
             * 适用于多通道时域信号的批量频谱计算。
             *
             * @param[in] f 时域双精度复数矩阵，每列为一路信号
             * @return     频域双精度复数矩阵
             *
             * @code
             * // 3通道256点时域信号批量FFT
             * Eigen::MatrixXcd X(256, 3);
             * auto F = FrequenceHelper::Fft(X);
             * @endcode
             */
            Eigen::MatrixXcd FrequenceHelper::Fft(const Eigen::MatrixXcd& f) {
                const int rows = static_cast<int>(f.rows()); ///< 矩阵行数（信号点数）
                const int cols = static_cast<int>(f.cols()); ///< 矩阵列数（信号通道数）

                Eigen::MatrixXcd result(rows, cols); ///< FFT输出复数矩阵

                // 对每一列执行FFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcd column = f.col(col); ///< 当前列的时域复数向量
                    result.col(col) = Fft(column);
                }

                return result;
            }

            /**
             * @brief 对实数值双精度 Eigen 矩阵逐列执行 FFT
             *
             * @details 将 MatrixXd 强制转换为复数矩阵（虚部补零），
             * 再对每列独立执行 FFT，适用于多通道实数时域信号的批量频谱计算。
             *
             * @param[in] fd 时域双精度实数矩阵，每列为一路信号
             * @return      频域双精度复数矩阵
             *
             * @code
             * Eigen::MatrixXd X = Eigen::MatrixXd::Random(256, 2);
             * Eigen::MatrixXcd F = FrequenceHelper::Fft(X);
             * @endcode
             */
            Eigen::MatrixXcd FrequenceHelper::Fft(const Eigen::MatrixXd& fd) {
				auto f = fd.cast<std::complex<double>>(); ///< 将实数矩阵转换为复数矩阵（虚部补零）
                const int rows = static_cast<int>(f.rows()); ///< 矩阵行数（信号点数）
                const int cols = static_cast<int>(f.cols()); ///< 矩阵列数（信号通道数）

                Eigen::MatrixXcd result(rows, cols); ///< FFT输出复数矩阵

                // 对每一列执行FFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcd column = f.col(col); ///< 当前列的时域复数向量
                    result.col(col) = Fft(column);
                }

                return result;
            }


            /**
             * @brief 对单精度 Eigen 复数矩阵逐列执行 FFT
             *
             * @details 对每列独立调用 Fft(VectorXcf)，
             * 适用于单精度多通道复数信号的批量变换。
             *
             * @param[in] f 时域单精度复数矩阵，每列为一路信号
             * @return     频域单精度复数矩阵
             *
             * @code
             * Eigen::MatrixXcf Xf(128, 2);
             * auto Ff = FrequenceHelper::Fft(Xf); // 单精度复数矩阵 FFT
             * @endcode
             */
            Eigen::MatrixXcf FrequenceHelper::Fft(const Eigen::MatrixXcf& f) {
                const int rows = static_cast<int>(f.rows()); ///< 矩阵行数（信号点数）
                const int cols = static_cast<int>(f.cols()); ///< 矩阵列数（信号通道数）

                Eigen::MatrixXcf result(rows, cols); ///< FFT单精度输出矩阵

                // 对每一列执行FFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcf column = f.col(col); ///< 当前列的单精度时域复数向量
                    result.col(col) = Fft(column);
                }

                return result;
            }


            /**
             * @brief 对实数值单精度 Eigen 矩阵逐列执行 FFT
             *
             * @details 将 MatrixXf 转换为单精度复数矩阵后，对每列独立执行 FFT，
             * 适用于多通道单精度实数时域信号的批量频谱计算。
             *
             * @param[in] fd 时域单精度实数矩阵，每列为一路信号
             * @return      频域单精度复数矩阵
             *
             * @code
             * Eigen::MatrixXf X = Eigen::MatrixXf::Random(128, 4);
             * Eigen::MatrixXcf F = FrequenceHelper::Fft(X); // 单精度实数矩阵 FFT
             * @endcode
             */
            Eigen::MatrixXcf FrequenceHelper::Fft(const Eigen::MatrixXf& fd) {
				auto f = fd.cast<std::complex<float>>(); ///< 将单精度实数矩阵转换为单精度复数矩阵（虚部补零）
                const int rows = static_cast<int>(f.rows()); ///< 矩阵行数（信号点数）
                const int cols = static_cast<int>(f.cols()); ///< 矩阵列数（信号通道数）

                Eigen::MatrixXcf result(rows, cols); ///< FFT单精度输出矩阵

                // 对每一列执行FFT
                for (int col = 0; col < cols; ++col) {
                    Eigen::VectorXcf column = f.col(col); ///< 当前列的实数转换后单精度复数向量
                    result.col(col) = Fft(column);
                }

                return result;
            }

        } // namespace Math
    } // namespace IO
} // namespace Qahse

#endif // FREQUENCEHELPER_H
