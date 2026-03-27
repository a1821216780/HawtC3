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

#include <vector>
#include <Eigen/Dense>
#include <stdexcept>

namespace Qahse {
    namespace IO {
        namespace Math {

            /// <summary>
            /// 提供数值积分方法，包括梯形法和辛普森规则计算。
            /// </summary>
            /// <remarks>
            /// IntegrationHelper 类包含用于执行数值积分的静态方法，支持一维数组、向量和矩阵的积分计算。
            /// 这些方法同时支持梯形法和辛普森法积分技术。该类专为科学和工程应用而设计。
            /// </remarks>
            class IntegrationHelper {
            public:
                // ============================================================================
                // 梯形积分方法 (Trapz)
                // ============================================================================

                /// <summary>
                /// 使用梯形规则计算数据点集的积分 - 双精度数组版本
                /// </summary>
                /// <param name="x">x 坐标数组，代表自变量。这些值必须按升序排列。</param>
                /// <param name="y">y 坐标数组，代表因变量。其长度必须与x相同。</param>
                /// <returns>使用梯形规则计算的数据点的积分</returns>
                static double Trapz(const std::vector<double>& x, const std::vector<double>& y);

                /// <summary>
                /// 使用梯形规则计算数据点集的积分 - 双精度数组版本（引用输出）
                /// </summary>
                /// <param name="x">x 坐标数组</param>
                /// <param name="y">y 坐标数组</param>
                /// <param name="integral">存储计算结果的引用</param>
                static void Trapz(const std::vector<double>& x, const std::vector<double>& y, double& integral);

                /// <summary>
                /// 使用梯形规则计算数据点集的积分 - 单精度数组版本
                /// </summary>
                /// <param name="x">x 坐标数组</param>
                /// <param name="y">y 坐标数组</param>
                /// <returns>计算得到的积分值</returns>
                static float Trapz(const std::vector<float>& x, const std::vector<float>& y);

                /// <summary>
                /// 使用梯形规则计算数据点集的积分 - 单精度数组版本（引用输出）
                /// </summary>
                /// <param name="x">x 坐标数组</param>
                /// <param name="y">y 坐标数组</param>
                /// <param name="integral">存储计算结果的引用</param>
                static void Trapz(const std::vector<float>& x, const std::vector<float>& y, float& integral);

                /// <summary>
                /// 使用梯形规则计算向量数据的积分 - 双精度向量版本
                /// </summary>
                /// <param name="x">x 坐标向量</param>
                /// <param name="y">y 坐标向量</param>
                /// <returns>计算得到的积分值</returns>
                static double Trapz(const Eigen::VectorXd& x, const Eigen::VectorXd& y);

                /// <summary>
                /// 使用梯形规则计算向量数据的积分 - 单精度向量版本
                /// </summary>
                /// <param name="x">x 坐标向量</param>
                /// <param name="y">y 坐标向量</param>
                /// <returns>计算得到的积分值</returns>
                static float Trapz(const Eigen::VectorXf& x, const Eigen::VectorXf& y);

                /// <summary>
                /// 使用梯形规则沿指定维度计算矩阵数据的积分 - 双精度版本
                /// </summary>
                /// <param name="x">x 坐标向量</param>
                /// <param name="y">y 坐标矩阵</param>
                /// <param name="dim">积分维度：1=沿行积分（对每列），2=沿列积分（对每行）</param>
                /// <returns>包含积分结果的向量</returns>
                static Eigen::VectorXd Trapz(const Eigen::VectorXd& x, const Eigen::MatrixXd& y, int dim);

                /// <summary>
                /// 使用梯形规则沿指定维度计算矩阵数据的积分 - 双精度版本（引用输出）
                /// </summary>
                /// <param name="x">x 坐标向量</param>
                /// <param name="y">y 坐标矩阵</param>
                /// <param name="dim">积分维度</param>
                /// <param name="result">存储积分结果的向量引用</param>
                static void Trapz(const Eigen::VectorXd& x, const Eigen::MatrixXd& y, int dim, Eigen::VectorXd& result);

                /// <summary>
                /// 使用梯形规则沿指定维度计算矩阵数据的积分 - 单精度版本
                /// </summary>
                /// <param name="x">x 坐标向量</param>
                /// <param name="y">y 坐标矩阵</param>
                /// <param name="dim">积分维度</param>
                /// <returns>包含积分结果的向量</returns>
                static Eigen::VectorXf Trapz(const Eigen::VectorXf& x, const Eigen::MatrixXf& y, int dim);

                /// <summary>
                /// 使用梯形规则沿指定维度计算矩阵数据的积分 - 单精度版本（引用输出）
                /// </summary>
                /// <param name="x">x 坐标向量</param>
                /// <param name="y">y 坐标矩阵</param>
                /// <param name="dim">积分维度</param>
                /// <param name="result">存储积分结果的向量引用</param>
                static void Trapz(const Eigen::VectorXf& x, const Eigen::MatrixXf& y, int dim, Eigen::VectorXf& result);

                /// <summary>
                /// 使用梯形规则计算累积积分 - 双精度版本
                /// </summary>
                /// <param name="x">x 坐标向量</param>
                /// <param name="y">y 坐标向量</param>
                /// <returns>包含累积积分值的向量（从末尾向前计算）</returns>
                static Eigen::VectorXd TrapzA(const Eigen::VectorXd& x, const Eigen::VectorXd& y);

                /// <summary>
                /// 使用梯形规则计算累积积分 - 单精度版本
                /// </summary>
                /// <param name="x">x 坐标向量</param>
                /// <param name="y">y 坐标向量</param>
                /// <returns>包含累积积分值的向量（从末尾向前计算）</returns>
                static Eigen::VectorXf TrapzA(const Eigen::VectorXf& x, const Eigen::VectorXf& y);

                // ============================================================================
                // 辛普森积分方法 (Simps)
                // ============================================================================

                /// <summary>
                /// 使用辛普森规则计算数据点集的积分 - 双精度数组版本
                /// </summary>
                /// <param name="x">x 坐标数组，值必须按升序排列</param>
                /// <param name="y">y 坐标数组</param>
                /// <returns>使用辛普森规则计算的积分值</returns>
                static double Simps(const std::vector<double>& x, const std::vector<double>& y);

                /// <summary>
                /// 使用复合辛普森规则计算向量数据的积分 - 双精度版本
                /// </summary>
                /// <param name="x">x 坐标向量</param>
                /// <param name="y">y 坐标向量</param>
                /// <returns>计算得到的积分值</returns>
                static double Simps(const Eigen::VectorXd& x, const Eigen::VectorXd& y);

                /// <summary>
                /// 使用辛普森规则沿指定维度计算矩阵数据的积分 - 双精度版本
                /// </summary>
                /// <param name="x">x 坐标向量</param>
                /// <param name="y">y 坐标矩阵</param>
                /// <param name="dim">积分维度：1=沿行积分，2=沿列积分</param>
                /// <returns>包含积分结果的向量</returns>
                static Eigen::VectorXd Simps(const Eigen::VectorXd& x, const Eigen::MatrixXd& y, int dim);

            private:
                /// <summary>
                /// 验证输入数组大小是否匹配
                /// </summary>
                template<typename T>
                static void ValidateInputSizes(const std::vector<T>& x, const std::vector<T>& y, const char* methodName);

                /// <summary>
                /// 验证输入向量大小是否匹配
                /// </summary>
                template<typename Derived1, typename Derived2>
                static void ValidateInputSizes(const Eigen::MatrixBase<Derived1>& x,
                    const Eigen::MatrixBase<Derived2>& y,
                    const char* methodName);

                /// <summary>
                /// 验证辛普森积分的最小点数要求
                /// </summary>
                template<typename T>
                static void ValidateSimpsMinimumPoints(const T& x, const char* methodName);
            };

        } // namespace Math
    } // namespace IO
} // namespace Qahse
