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

#ifndef INTERPOLATEHELPER_H
#define INTERPOLATEHELPER_H

#include <libInterpolate/Interpolate.hpp>
#include <Eigen/Dense>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "../Log/LogHelper.h"

namespace Qahse::IO::Math
{
	/**
	 * @brief 插値辅助类，提供一维和二维插値功能
	 *
	 * @details 封装 libInterpolate 库，支持以下插値类型：
	 * - 一维：线性插値、三次样条插値
	 * - 二维规则网格：双线性、双三次样条、最近邻近
	 * - 二维不规则网格：薄板样条、Delaunay 三角剂分
	 *
	 * 所有方法均为静态模板函数，支持 double/float 等浮点类型。
	 *
	 * @code
	 * // 一维线性插値示例
	 * std::vector<double> x = {0,1,2,3};
	 * std::vector<double> y = {0,1,4,9};
	 * double v = InterpolateHelper::Interp1D(x, y, 1.5); // ≈ 2.25
	 * @endcode
	 */
	class InterpolateHelper
	{
	public:
#pragma region  一维插值 

		/**
		 * @brief 一维插値方法类型枚举
		 *
		 * @details 用于选择 Interp1D() 函数的插値算法。
		 */
		enum class Interp1DType {
			/**
			 * @brief 线性插値
			 * @see https://en.wikipedia.org/wiki/Linear_interpolation
			 */
			Linear,
			/**
			 * @brief 三次样条插値
			 * @see https://en.wikipedia.org/wiki/Spline_interpolation
			 */
			CubicSpline
		};


		/**
		 * @brief 创建一维线性插値器（std::vector 版）
		 *
		 * @details 使用 libInterpolate 创建 _1D::LinearInterpolator，
		 * 通过返回 unique_ptr 支持延迟防隔地进行多次单点查询。
		 *
		 * @tparam T 数据类型（double 或 float）
		 * @param[in] x 自变量样本点列表
		 * @param[in] y 因变量对应列表，必须与 x 等长
		 * @return     指向 LinearInterpolator<T> 的 unique_ptr
		 *
		 * @note 若 x.size() != y.size() 或为空，调用 LogHelper::ErrorLog 中止程序
		 *
		 * @code
		 * std::vector<double> x = {0,1,2,3};
		 * std::vector<double> y = {0,1,4,9};
		 * auto interp = InterpolateHelper::Interp1DL(x, y);
		 * double v = (*interp)(1.5); // ≈ 2.5
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _1D::LinearInterpolator<T>> Interp1DL(
			const std::vector<T>& x,
			const std::vector<T>& y)
		{
			if (x.size() != y.size()) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y must have the same size");
			}
			if (x.empty()) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_1D::LinearInterpolator<T>>();

			interp->setData(x, y);

			return interp;

		}


		/**
		 * @brief 创建一维线性插値器（Eigen 列向量版）
		 *
		 * @details 接收 Eigen 列向量输入，其余与 std::vector 版相同。
		 *
		 * @tparam T 数据类型（double 或 float）
		 * @param[in] x Eigen 列向量自变量
		 * @param[in] y Eigen 列向量因变量，必须与 x 等长
		 * @return     指向 LinearInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * Eigen::VectorXd x = Eigen::VectorXd::LinSpaced(5, 0, 4);
		 * Eigen::VectorXd y = x.array().square();
		 * auto interp = InterpolateHelper::Interp1DL(x, y);
		 * double v = (*interp)(2.5);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _1D::LinearInterpolator<T>> Interp1DL(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y)
		{
			if (x.rows() != y.rows()) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y must have the same size");
			}
			if (x.cols() != 1) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_1D::LinearInterpolator<T>>();

			interp->setData(x, y);

			return interp;

		}



		/**
		 * @brief 创建一维三次样条插値器（std::vector 版）
		 *
		 * @details 使用 libInterpolate 创建 _1D::CubicSplineInterpolator，
		 * 对连续平滑函数有更高的拟合精度。
		 *
		 * @tparam T 数据类型（double 或 float）
		 * @param[in] x 自变量样本点列表
		 * @param[in] y 因变量对应列表，必须与 x 等长
		 * @return     指向 CubicSplineInterpolator<T> 的 unique_ptr
		 *
		 * @note 若 x.size() != y.size() 或为空，调用 LogHelper::ErrorLog 中止程序
		 *
		 * @code
		 * std::vector<double> x = {0,1,2,3};
		 * std::vector<double> y = {0,1,4,9};
		 * auto spline = InterpolateHelper::Interp1DS(x, y);
		 * double v = (*spline)(1.5);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _1D::CubicSplineInterpolator<T>> Interp1DS(
			const std::vector<T>& x,
			const std::vector<T>& y)
		{
			if (x.size() != y.size()) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y must have the same size");
			}
			if (x.empty()) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_1D::CubicSplineInterpolator<T>>();

			interp->setData(x, y);

			return interp;

		}






		/**
		 * @brief 创建一维三次样条插値器（Eigen 列向量版）
		 *
		 * @tparam T 数据类型（double 或 float）
		 * @param[in] x Eigen 列向量自变量
		 * @param[in] y Eigen 列向量因变量
		 * @return     指向 CubicSplineInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * Eigen::VectorXd x = Eigen::VectorXd::LinSpaced(5, 0, 4);
		 * Eigen::VectorXd y = x.array().square();
		 * auto spline = InterpolateHelper::Interp1DS(x, y);
		 * double v = (*spline)(2.5);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _1D::CubicSplineInterpolator<T>> Interp1DS(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y)
		{
			if (x.rows() != y.rows()) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y must have the same size");
			}
			if (x.cols() != 1) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_1D::CubicSplineInterpolator<T>>();

			interp->setData(x, y);

			return interp;

		}




		/**
		 * @brief 一维插値，返回单个插値点小数结果（std::vector 版）
		 *
		 * @details 根据给定的 x/y 数据和插値类型，对单个点 value 进行插値返回小数结果。
		 *
		 * @tparam T     数据类型（double 或 float）
		 * @param[in] x     自变量样本点列表
		 * @param[in] y     因变量对应列表
		 * @param[in] value 需要插値的点
		 * @param[in] type  插値类型，默认 Interp1DType::Linear
		 * @return          插値结果
		 *
		 * @note 若数据无效将调用 LogHelper::ErrorLog 中止程序
		 *
		 * @code
		 * std::vector<double> x = {0,1,2,3};
		 * std::vector<double> y = {0,1,4,9};
		 * double v = InterpolateHelper::Interp1D(x, y, 1.5); // 线性插値 ≈ 2.5
		 * double vs = InterpolateHelper::Interp1D(x, y, 1.5, Interp1DType::CubicSpline);
		 * @endcode
		 */		
		template<typename T>
		static T Interp1D(
			const std::vector<T>& x,
			const std::vector<T>& y,
			const T& value,
			Interp1DType type = Interp1DType::Linear)
		{
			if (x.size() != y.size() || x.empty())
			{
				Qahse::IO::Log::LogHelper::ErrorLog("输入数据无效，x和y必须具有相同的非零长度。");
			}

			switch (type)
			{
			case Interp1DType::Linear:
			{
				auto interp = _1D::LinearInterpolator<T>();
				interp.setData(x, y);
				return interp(value);
			}
			case Interp1DType::CubicSpline:
			{
				auto interp = _1D::CubicSplineInterpolator<T>();
				interp.setData(x, y);
				return interp(value);
			}
			default:
				Qahse::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return T();
			}
		}

		/**
		 * @brief 一维插値，返回单个插値点小数结果（Eigen 列向量版）
		 *
		 * @tparam T     数据类型（double 或 float）
		 * @param[in] x     Eigen 列向量自变量
		 * @param[in] y     Eigen 列向量因变量
		 * @param[in] value 需要插値的点
		 * @param[in] type  插値类型，默认 Interp1DType::Linear
		 * @return          插値结果
		 *
		 * @code
		 * Eigen::VectorXd x = Eigen::VectorXd::LinSpaced(5, 0, 4);
		 * Eigen::VectorXd y = x.array().square();
		 * double v = InterpolateHelper::Interp1D(x, y, 2.5);
		 * @endcode
		 */
		template<typename T>
		static T Interp1D(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y,
			const T& value,
			Interp1DType type = Interp1DType::Linear)
		{
			if (x.rows() != y.rows())
			{
				Qahse::IO::Log::LogHelper::ErrorLog("输入数据无效，x和y必须具有相同的非零长度。");
			}

			switch (type)
			{
			case Interp1DType::Linear:
			{
				auto interp = _1D::LinearInterpolator<T>();
				interp.setData(x, y);
				return interp(value);
			}
			case Interp1DType::CubicSpline:
			{
				auto interp = _1D::CubicSplineInterpolator<T>();
				interp.setData(x, y);
				return interp(value);
			}
			default:
				Qahse::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return T();
			}
		}



		/**
		 * @brief 一维批量插値，返回多个插値点结果（std::vector 版）
		 *
		 * @details 对数组 value 中每个点独立进行插値，返回等长结果数组。
		 *
		 * @tparam T      数据类型（double 或 float）
		 * @param[in] x      自变量样本点列表
		 * @param[in] y      因变量对应列表
		 * @param[in] value  需要插値的点列表
		 * @param[in] type   插値类型，默认 Interp1DType::Linear
		 * @return           插値结果数组
		 *
		 * @code
		 * std::vector<double> x = {0,1,2,3};
		 * std::vector<double> y = {0,1,4,9};
		 * std::vector<double> q = {0.5, 1.5, 2.5};
		 * auto res = InterpolateHelper::Interp1D(x, y, q);
		 * @endcode
		 */
		template<typename T>
		static std::vector<T> Interp1D(
			const std::vector<T>& x,
			const std::vector<T>& y,
			const std::vector<T>& value,
			Interp1DType type = Interp1DType::Linear)
		{
			if (x.size() != y.size() || x.empty())
			{
				Qahse::IO::Log::LogHelper::ErrorLog("输入数据无效，x和y必须具有相同的非零长度。");
			}
			std::vector<T> res(x.size());
			switch (type)
			{
			case Interp1DType::Linear:
			{
				auto interp = _1D::LinearInterpolator<T>();
				interp.setData(x, y);
				for (size_t i = 0; i < x.size(); i++)
				{
					res[i] = interp(value[i]);

				}
				return res;
			}
			case Interp1DType::CubicSpline:
			{
				auto interp = _1D::CubicSplineInterpolator<T>();
				interp.setData(x, y);
				for (size_t i = 0; i < x.size(); i++)
				{
					res[i] = interp(value[i]);

				}
				return res;
			}
			default:
				Qahse::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return std::vector<T>();
			}
		}

		/**
		 * @brief 一维批量插値，返回 Eigen 列向量结果（Eigen 版）
		 *
		 * @tparam T      数据类型（double 或 float）
		 * @param[in] x      Eigen 列向量自变量
		 * @param[in] y      Eigen 列向量因变量
		 * @param[in] value  需要插値的 Eigen 列向量
		 * @param[in] type   插値类型，默认 Interp1DType::Linear
		 * @return           Eigen 列向量插値结果
		 *
		 * @code
		 * Eigen::VectorXd x = Eigen::VectorXd::LinSpaced(5, 0, 4);
		 * Eigen::VectorXd y = x.array().square();
		 * Eigen::VectorXd q = Eigen::VectorXd::LinSpaced(3, 0.5, 3.5);
		 * auto res = InterpolateHelper::Interp1D(x, y, q);
		 * @endcode
		 */
		template<typename T>
		static Eigen::Matrix<T, Eigen::Dynamic, 1> Interp1D(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& value,
			Interp1DType type = Interp1DType::Linear)
		{
			if (x.rows() != y.rows())
			{
				Qahse::IO::Log::LogHelper::ErrorLog("输入数据无效，x和y必须具有相同的非零长度。");
			}
			Eigen::Matrix<T, Eigen::Dynamic, 1> res(x.rows());
			switch (type)
			{
			case Interp1DType::Linear:
			{
				auto interp = _1D::LinearInterpolator<T>();
				interp.setData(x, y);
				for (size_t i = 0; i < x.size(); i++)
				{
					res[i] = interp(value[i]);

				}
				return res;
			}
			case Interp1DType::CubicSpline:
			{
				auto interp = _1D::CubicSplineInterpolator<T>();
				interp.setData(x, y);
				for (size_t i = 0; i < x.size(); i++)
				{
					res[i] = interp(value[i]);

				}
				return res;
			}
			default:
				Qahse::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return std::vector<T>();
			}
		}

#pragma endregion 一维插值 



#pragma region 二维插值规则网格插值
		/**
		 * @brief 规则网格二维插値方法类型枚举
		 *
		 * @details 适用于均匀网格数据上的二维插値。
		 */
		enum class Interp2DRegularType {
			/**
			 * @brief 双线性插値
			 * @see https://en.wikipedia.org/wiki/Bilinear_interpolation
			 */
			Bilinear,
			/**
			 * @brief 双三次样条插値
			 * @see https://en.wikipedia.org/wiki/Bicubic_interpolation
			 */
			BiCubicSpline,
			/**
			 * @brief 最近邻近插値
			 * @see https://en.wikipedia.org/wiki/Nearest-neighbor_interpolation
			 */
			NearestNeighbor

		};


		/**
		 * @brief 创建规则网格二维双线性插値器（std::vector 版）
		 *
		 * @details 构建 _2D::BilinearInterpolator 并设置 (x,y,z) 数据。
		 * 注意: x, y, z 必须是封装成平坦数组的规则网格数据。
		 *
		 * @tparam T 数据类型（double 或 float）
		 * @param[in] x x 坐标列表
		 * @param[in] y y 坐标列表
		 * @param[in] z z 坐标列表（函数値），必须与 x, y 等长
		 * @return     指向 BilinearInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * std::vector<double> x = {0,1,2};
		 * std::vector<double> y = {0,1,2};
		 * std::vector<double> z = {0,1,4};
		 * auto interp = InterpolateHelper::Interp2DRL(x, y, z);
		 * double v = (*interp)(0.5, 0.5);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _2D::BilinearInterpolator<T>> Interp2DRL(
			const std::vector<T>& x,
			const std::vector<T>& y,
			const std::vector<T>& z)
		{
			if (x.size() != y.size() ||
				x.size() != z.size() ||
				y.size() != z.size()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.empty()) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::BilinearInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


		/**
		 * @brief 创建规则网格二维双线性插値器（Eigen 版）
		 *
		 * @tparam T 数据类型
		 * @param[in] x Eigen 列向量 x 坐标
		 * @param[in] y Eigen 列向量 y 坐标
		 * @param[in] z Eigen 列向量 z 坐标
		 * @return     指向 BilinearInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * Eigen::VectorXd x(3), y(3), z(3);
		 * x << 0,1,2; y << 0,1,2; z << 0,1,4;
		 * auto interp = InterpolateHelper::Interp2DRL(x, y, z);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _2D::BilinearInterpolator<T>> Interp2DRL(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& z)
		{
			if (x.rows() != y.rows() ||
				x.rows() != z.rows() ||
				y.rows() != z.rows()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.cols() != 1) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::BilinearInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}



		/**
		 * @brief 创建规则网格二维双三次样条插値器（std::vector 版）
		 *
		 * @details 构建 _2D::BicubicInterpolator，对平滑曲面有更高的拟合精度。
		 *
		 * @tparam T 数据类型
		 * @param[in] x x 坐标列表
		 * @param[in] y y 坐标列表
		 * @param[in] z z 坐标列表（函数値）
		 * @return     指向 BicubicInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * auto interp = InterpolateHelper::Interp2DRS(x, y, z);
		 * double v = (*interp)(0.5, 0.5);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _2D::BicubicInterpolator<T>> Interp2DRS(
			const std::vector<T>& x,
			const std::vector<T>& y,
			const std::vector<T>& z)
		{
			if (x.size() != y.size() ||
				x.size() != z.size() ||
				y.size() != z.size()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.empty()) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::BicubicInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


		/**
		 * @brief 创建规则网格二维双三次样条插値器（Eigen 版）
		 *
		 * @tparam T 数据类型
		 * @param[in] x Eigen 列向量 x 坐标
		 * @param[in] y Eigen 列向量 y 坐标
		 * @param[in] z Eigen 列向量 z 坐标
		 * @return     指向 BicubicInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * auto interp = InterpolateHelper::Interp2DRS(xe, ye, ze);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _2D::BicubicInterpolator<T>> Interp2DRS(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& z)
		{
			if (x.rows() != y.rows() ||
				x.rows() != z.rows() ||
				y.rows() != z.rows()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.cols() != 1) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::BicubicInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


		/**
		 * @brief 创建规则网格二维最近邻近插値器（std::vector 版）
		 *
		 * @details 构建 _2D::NearestNeighborInterpolator，返回最近网格点的函数値。
		 *
		 * @tparam T 数据类型
		 * @param[in] x x 坐标列表
		 * @param[in] y y 坐标列表
		 * @param[in] z z 坐标列表（函数値）
		 * @return     指向 NearestNeighborInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * auto interp = InterpolateHelper::Interp2DRN(x, y, z);
		 * double v = (*interp)(0.4, 0.7); // 返回最近点在 z 中的値
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _2D::NearestNeighborInterpolator<T>> Interp2DRN(
			const std::vector<T>& x,
			const std::vector<T>& y,
			const std::vector<T>& z)
		{
			if (x.size() != y.size() ||
				x.size() != z.size() ||
				y.size() != z.size()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.empty()) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::NearestNeighborInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


		/**
		 * @brief 创建规则网格二维最近邻近插値器（Eigen 版）
		 *
		 * @tparam T 数据类型
		 * @param[in] x Eigen 列向量 x 坐标
		 * @param[in] y Eigen 列向量 y 坐标
		 * @param[in] z Eigen 列向量 z 坐标
		 * @return     指向 NearestNeighborInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * auto interp = InterpolateHelper::Interp2DRN(xe, ye, ze);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _2D::NearestNeighborInterpolator<T>> Interp2DRN(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& z)
		{
			if (x.rows() != y.rows() ||
				x.rows() != z.rows() ||
				y.rows() != z.rows()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.cols() != 1) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::NearestNeighborInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}




		/**
		 * @brief 规则网格二维插値，返回单个点插値结果（std::vector 版）
		 *
		 * @details 根据指定类型对 (x1,y1) 进行二维插値并返回示数値。
		 *
		 * @tparam T     数据类型（double 或 float）
		 * @param[in] x     x 坐标列表
		 * @param[in] y     y 坐标列表
		 * @param[in] z     z 函数値列表
		 * @param[in] x1    查询点 x 坐标
		 * @param[in] y1    查询点 y 坐标
		 * @param[in] type  插値类型，默认 Interp2DRegularType::Bilinear
		 * @return          插値结果
		 *
		 * @code
		 * std::vector<double> x={0,1,2}, y={0,1,2}, z={0,1,4,1,2,5,4,5,8};
		 * double v = InterpolateHelper::Interp2DR(x, y, z, 0.5, 0.5);
		 * double vs = InterpolateHelper::Interp2DR(x, y, z, 0.5, 0.5,
		 *              InterpolateHelper::Interp2DRegularType::BiCubicSpline);
		 * @endcode
		 */
		template<typename T>
		static T Interp2DR(
			const std::vector<T>& x,
			const std::vector<T>& y,
			const std::vector<T>& z,
			T x1,
			T y1,
			Interp2DRegularType type = Interp2DRegularType::Bilinear)
		{
			if (x.size() != y.size() ||
				x.size() != z.size() ||
				y.size() != z.size()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}

			switch (type)
			{
			case Interp2DRegularType::Bilinear:
			{
				auto interp = _2D::BilinearInterpolator<T>();
				interp.setData(x, y, z);
				return interp(x1, y1);
			}
			case Interp2DRegularType::BiCubicSpline:
			{
				auto interp = _2D::BicubicInterpolator<T>();
				interp.setData(x, y, z);
				return interp(x1, y1);
			}
			case Interp2DRegularType::NearestNeighbor:
			{
				auto interp = _2D::NearestNeighborInterpolator<T>();
				interp.setData(x, y, z);
				return interp(x1, y1);
			}
			default:
				Qahse::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return T();
			}
		}

		/**
		 * @brief 规则网格二维插値，返回单个点插値结果（Eigen 版）
		 *
		 * @tparam T     数据类型
		 * @param[in] x     Eigen 列向量 x 坐标
		 * @param[in] y     Eigen 列向量 y 坐标
		 * @param[in] z     Eigen 列向量 z 函数値
		 * @param[in] x1    查询点 x 坐标
		 * @param[in] y1    查询点 y 坐标
		 * @param[in] type  插値类型，默认 Interp2DRegularType::Bilinear
		 * @return          插値结果
		 *
		 * @code
		 * Eigen::VectorXd x(3), y(3), z(9);
		 * double v = InterpolateHelper::Interp2DR(x, y, z, 0.5, 0.5);
		 * @endcode
		 */
		template<typename T>
		static T Interp2DR(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& z,
			T x1,
			T y1,
			Interp2DRegularType type = Interp2DRegularType::Bilinear)
		{
			if (x.rows() != y.rows() ||
				x.rows() != z.rows() ||
				y.rows() != z.rows()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}

			switch (type)
			{
			case Interp2DRegularType::Bilinear:
			{
				auto interp = _2D::BilinearInterpolator<T>();
				interp.setData(x, y, z);
				return interp(x1, y1);
			}
			case Interp2DRegularType::BiCubicSpline:
			{
				auto interp = _2D::BicubicInterpolator<T>();
				interp.setData(x, y, z);
				return interp(x1, y1);
			}
			case Interp2DRegularType::NearestNeighbor:
			{
				auto interp = _2D::NearestNeighborInterpolator<T>();
				interp.setData(x, y, z);
				return interp(x1, y1);
			}
			default:
				Qahse::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return T();
			}
		}


#pragma endregion 二维插值规则网格插值



#pragma region 二维插值不规则网格插值
		/// <summary>
/// 不规则网格二维插值
/// </summary>
		enum class Interp2DIrregularType
		{
			/// <summary>
			/// 薄板样条函数插值 https://en.wikipedia.org/wiki/Thin_plate_spline
			/// </summary>
			ThinPlateSpline,
			/// <summary>
			/// Delaunay 三角剖分插值 https://en.wikipedia.org/wiki/Delaunay_triangulation
			/// </summary>
			LinearDelaunayTriangles,


		};

		/**
		 * @brief 创建不规则网格 Delaunay 三角剂分插値器（std::vector 版）
		 *
		 * @details 构建 _2D::LinearDelaunayTriangleInterpolator，
		 * 适用于非均匀散点数据。
		 *
		 * @tparam T 数据类型
		 * @param[in] x x 坐标列表
		 * @param[in] y y 坐标列表
		 * @param[in] z z 函数値列表
		 * @return     指向 LinearDelaunayTriangleInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * auto interp = InterpolateHelper::Interp2DIL(x, y, z);
		 * double v = (*interp)(1.5, 1.5);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _2D::LinearDelaunayTriangleInterpolator<T>> Interp2DIL(
			const std::vector<T>& x,
			const std::vector<T>& y,
			const std::vector<T>& z)
		{
			if (x.size() != y.size() ||
				x.size() != z.size() ||
				y.size() != z.size()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.empty()) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::LinearDelaunayTriangleInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


		/**
		 * @brief 创建不规则网格 Delaunay 三角剂分插値器（Eigen 版）
		 *
		 * @tparam T 数据类型
		 * @param[in] x Eigen 列向量 x 坐标
		 * @param[in] y Eigen 列向量 y 坐标
		 * @param[in] z Eigen 列向量 z 函数値
		 * @return     指向 LinearDelaunayTriangleInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * auto interp = InterpolateHelper::Interp2DIL(xe, ye, ze);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _2D::LinearDelaunayTriangleInterpolator<T>> Interp2DIL(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& z)
		{
			if (x.rows() != y.rows() ||
				x.rows() != z.rows() ||
				y.rows() != z.rows()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.cols() != 1) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::LinearDelaunayTriangleInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}



		/**
		 * @brief 创建不规则网格薄板样条插値器（std::vector 版）
		 *
		 * @details 构建 _2D::ThinPlateSplineInterpolator，
		 * 适用于非均匀散点数据且需要平滑插値的场景。
		 *
		 * @tparam T 数据类型
		 * @param[in] x x 坐标列表
		 * @param[in] y y 坐标列表
		 * @param[in] z z 函数値列表
		 * @return     指向 ThinPlateSplineInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * auto interp = InterpolateHelper::Interp2DIT(x, y, z);
		 * double v = (*interp)(1.5, 1.5);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _2D::ThinPlateSplineInterpolator<T>> Interp2DIT(
			const std::vector<T>& x,
			const std::vector<T>& y,
			const std::vector<T>& z)
		{
			if (x.size() != y.size() ||
				x.size() != z.size() ||
				y.size() != z.size()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.empty()) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::ThinPlateSplineInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


		/**
		 * @brief 创建不规则网格薄板样条插値器（Eigen 版）
		 *
		 * @tparam T 数据类型
		 * @param[in] x Eigen 列向量 x 坐标
		 * @param[in] y Eigen 列向量 y 坐标
		 * @param[in] z Eigen 列向量 z 函数値
		 * @return     指向 ThinPlateSplineInterpolator<T> 的 unique_ptr
		 *
		 * @code
		 * auto interp = InterpolateHelper::Interp2DIT(xe, ye, ze);
		 * @endcode
		 */
		template<typename T>
		static std::unique_ptr< _2D::ThinPlateSplineInterpolator<T>> Interp2DIT(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& z)
		{
			if (x.rows() != y.rows() ||
				x.rows() != z.rows() ||
				y.rows() != z.rows()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.cols() != 1) {
				Qahse::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::ThinPlateSplineInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}



		/**
		 * @brief 不规则网格二维插值，返回单个点插值结果（std::vector 版）
		 *
		 * @details 根据指定类型对 (x1,y1) 进行不规则网格二维插値并返回结果。
		 *
		 * @tparam T     数据类型（double 或 float）
		 * @param[in] x     x 坐标列表（非均匀散点）
		 * @param[in] y     y 坐标列表
		 * @param[in] z     z 函数値列表
		 * @param[in] x1    查询点 x 坐标
		 * @param[in] y1    查询点 y 坐标
		 * @param[in] type  插値类型，默认 Interp2DIrregularType::ThinPlateSpline
		 * @return          插値结果
		 *
		 * @code
		 * // 非均匀散点插値示例
		 * std::vector<double> x = {0,1,0,1,0.5};
		 * std::vector<double> y = {0,0,1,1,0.5};
		 * std::vector<double> z = {0,1,1,2,1};
		 * double v = InterpolateHelper::Interp2DI(x, y, z, 0.3, 0.7);
		 * @endcode
		 */
		template<typename T>
		static T Interp2DI(
			const std::vector<T>& x,
			const std::vector<T>& y,
			const std::vector<T>& z,
			T x1,
			T y1,
			Interp2DIrregularType type = Interp2DIrregularType::ThinPlateSpline)
		{
			if (x.size() != y.size() ||
				x.size() != z.size() ||
				y.size() != z.size()
				) {
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}

			switch (type)
			{
			case Interp2DIrregularType::ThinPlateSpline:
			{
				auto interp = _2D::ThinPlateSplineInterpolator<T>();
				interp.setData(x, y, z);
				return interp(x1, y1);
			}
			case Interp2DIrregularType::LinearDelaunayTriangles:
			{
				auto interp = _2D::LinearDelaunayTriangleInterpolator<T>();
				interp.setData(x, y, z);
				return interp(x1, y1);
			}
			default:
				Qahse::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return T();
			}
		}

		// 2维插值 - 直接返回插值结果
		/**
		 * @brief 不规则网格二维插值，返回单个点插值结果（Eigen 版）
		 *
		 * @tparam T     数据类型
		 * @param[in] x     Eigen 列向量 x 坐标
		 * @param[in] y     Eigen 列向量 y 坐标
		 * @param[in] z     Eigen 列向量 z 函数値
		 * @param[in] x1    查询点 x 坐标
		 * @param[in] y1    查询点 y 坐标
		 * @param[in] type  插値类型，默认 Interp2DIrregularType::ThinPlateSpline
		 * @return          插値结果
		 *
		 * @code
		 * Eigen::VectorXd x(5), y(5), z(5);
		 * double v = InterpolateHelper::Interp2DI(x, y, z, 0.3, 0.7,
		 *              InterpolateHelper::Interp2DIrregularType::LinearDelaunayTriangles);
		 * @endcode
		 */
  		template<typename T>
		static T Interp2DI(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& z,
			T x1,
			T y1,
			Interp2DIrregularType type = Interp2DIrregularType::ThinPlateSpline)
		{
			if (x.rows() != y.rows() ||
				x.rows() != z.rows() ||
				y.rows() != z.rows()
				)
			{
				Qahse::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}

			switch (type)
			{
			case Interp2DIrregularType::ThinPlateSpline:
			{
				auto interp = _2D::ThinPlateSplineInterpolator<T>();
				interp.setData(x, y, z);
				return interp(x1, y1);
			}
			case Interp2DIrregularType::LinearDelaunayTriangles:
			{
				auto interp = _2D::LinearDelaunayTriangleInterpolator<T>();
				interp.setData(x, y, z);
				return interp(x1, y1);
			}
			default:

				Qahse::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return T();
			}
		}


#pragma endregion 二维插值不规则网格插值



	};

}

#endif // INTERPOLATEHELPER_H
