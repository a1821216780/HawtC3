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

#ifndef INTERPOLATEHELPER_H
#define INTERPOLATEHELPER_H

#include <libInterpolate/Interpolate.hpp>
#include <Eigen/Dense>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "../Log/LogHelper.h"

namespace HawtC3::IO::Math
{
	class InterpolateHelper
	{
	public:
#pragma region  一维插值 

		/// <summary>
/// 一维度线性插值
/// </summary>
		enum class Interp1DType {
			/// <summary>
			/// 线性插值 https://en.wikipedia.org/wiki/Linear_interpolation
			/// </summary>
			Linear,
			/// <summary>
			/// 三次样条插值 https://en.wikipedia.org/wiki/Spline_interpolation
			/// </summary>
			CubicSpline
		};


		template<typename T>
		static std::unique_ptr< _1D::LinearInterpolator<T>> Interp1DL(
			const std::vector<T>& x,
			const std::vector<T>& y)
		{
			if (x.size() != y.size()) {
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y must have the same size");
			}
			if (x.empty()) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_1D::LinearInterpolator<T>>();

			interp->setData(x, y);

			return interp;

		}


		template<typename T>
		static std::unique_ptr< _1D::LinearInterpolator<T>> Interp1DL(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y)
		{
			if (x.rows() != y.rows()) {
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y must have the same size");
			}
			if (x.cols() != 1) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_1D::LinearInterpolator<T>>();

			interp->setData(x, y);

			return interp;

		}



		template<typename T>
		static std::unique_ptr< _1D::CubicSplineInterpolator<T>> Interp1DS(
			const std::vector<T>& x,
			const std::vector<T>& y)
		{
			if (x.size() != y.size()) {
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y must have the same size");
			}
			if (x.empty()) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_1D::CubicSplineInterpolator<T>>();

			interp->setData(x, y);

			return interp;

		}






		template<typename T>
		static std::unique_ptr< _1D::CubicSplineInterpolator<T>> Interp1DS(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y)
		{
			if (x.rows() != y.rows()) {
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y must have the same size");
			}
			if (x.cols() != 1) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_1D::CubicSplineInterpolator<T>>();

			interp->setData(x, y);

			return interp;

		}




		// 一维插值 - 直接返回插值结果
		template<typename T>
		static T Interp1D(
			const std::vector<T>& x,
			const std::vector<T>& y,
			const T& value,
			Interp1DType type = Interp1DType::Linear)
		{
			if (x.size() != y.size() || x.empty())
			{
				HawtC3::IO::Log::LogHelper::ErrorLog("输入数据无效，x和y必须具有相同的非零长度。");
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
				HawtC3::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return T();
			}
		}

		template<typename T>
		static T Interp1D(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y,
			const T& value,
			Interp1DType type = Interp1DType::Linear)
		{
			if (x.rows() != y.rows())
			{
				HawtC3::IO::Log::LogHelper::ErrorLog("输入数据无效，x和y必须具有相同的非零长度。");
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
				HawtC3::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return T();
			}
		}



		template<typename T>
		static std::vector<T> Interp1D(
			const std::vector<T>& x,
			const std::vector<T>& y,
			const std::vector<T>& value,
			Interp1DType type = Interp1DType::Linear)
		{
			if (x.size() != y.size() || x.empty())
			{
				HawtC3::IO::Log::LogHelper::ErrorLog("输入数据无效，x和y必须具有相同的非零长度。");
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
				HawtC3::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return std::vector<T>();
			}
		}

		template<typename T>
		static Eigen::Matrix<T, Eigen::Dynamic, 1> Interp1D(
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& x,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& y,
			const Eigen::Matrix<T, Eigen::Dynamic, 1>& value,
			Interp1DType type = Interp1DType::Linear)
		{
			if (x.rows() != y.rows())
			{
				HawtC3::IO::Log::LogHelper::ErrorLog("输入数据无效，x和y必须具有相同的非零长度。");
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
				HawtC3::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return std::vector<T>();
			}
		}

#pragma endregion 一维插值 



#pragma region 二维插值规则网格插值
		/// <summary>
/// 规则网格二维插值
/// </summary>
		enum class Interp2DRegularType {
			/// <summary>
			/// 双线性插值 https://en.wikipedia.org/wiki/Bilinear_interpolation
			/// </summary>
			Bilinear,
			/// <summary>
			/// 双三次样条插值 https://en.wikipedia.org/wiki/Bicubic_interpolation
			/// </summary>
			BiCubicSpline,
			/// <summary>
			/// 最近近邻插值 https://en.wikipedia.org/wiki/Nearest-neighbor_interpolation
			/// </summary>
			NearestNeighbor

		};


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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.empty()) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::BilinearInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.cols() != 1) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::BilinearInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}



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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.empty()) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::BicubicInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.cols() != 1) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::BicubicInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.empty()) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::NearestNeighborInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.cols() != 1) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::NearestNeighborInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}




		// 2维插值 - 直接返回插值结果
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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
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
				HawtC3::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return T();
			}
		}

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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
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
				HawtC3::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.empty()) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::LinearDelaunayTriangleInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.cols() != 1) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::LinearDelaunayTriangleInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}



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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.empty()) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::ThinPlateSplineInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}


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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
			}
			if (x.cols() != 1) {
				HawtC3::IO::Log::LogHelper::ErrorLog("Input data cannot be empty");
			}
			auto interp = std::make_unique<::_2D::ThinPlateSplineInterpolator<T>>();

			interp->setData(x, y, z);

			return interp;

		}



		// 2维插值 - 直接返回插值结果
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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
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
				HawtC3::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return T();
			}
		}

		// 2维插值 - 直接返回插值结果
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
				HawtC3::IO::Log::LogHelper::ErrorLog("x and y and z must have the same size");
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

				HawtC3::IO::Log::LogHelper::ErrorLog("Unknown interpolation type");
				return T();
			}
		}


#pragma endregion 二维插值不规则网格插值



	};

}

#endif // INTERPOLATEHELPER_H
