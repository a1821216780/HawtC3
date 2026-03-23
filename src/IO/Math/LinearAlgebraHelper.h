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

#ifndef LINEARALGEBRAHELPER_H
#define LINEARALGEBRAHELPER_H

#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <tuple>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>
#include <chrono>
#include <string>
#include <fstream>
#include <iostream>
#include <initializer_list>
#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace HawtC3::IO::Math {
	/// <summary>
	/// 线性代数帮助类，提供矩阵和向量运算的静态方法集合
	/// </summary>
	/// <remarks>
	/// 这个静态类包含用于执行线性代数运算的方法。包括矩阵操作、向量操作、
	/// 数值计算和其他在线性代数中常用的实用工具。设计用于与Eigen
	/// 库配合工作，并为矩阵分解、向量范数和矩阵重构等任务提供额外功能。
	/// 
	/// 基本使用示例:
	/// auto matrix = LinearAlgebraHelper::Eye(3);           // 创建3x3单位矩阵
	/// auto vector = LinearAlgebraHelper::ones(3);          // 创建全1向量
	/// auto result = matrix * vector;                       // 矩阵向量乘法
	/// 
	/// 统计计算示例:
	/// auto data = LinearAlgebraHelper::zeros({1, 2, 3, 4, 5});
	/// auto mean = LinearAlgebraHelper::mean(data.transpose(), 2); // 计算行均值
	/// </remarks>
	class LinearAlgebraHelper {
	public:
		// Type definitions for compatibility
		using VectorXd = Eigen::VectorXd;
		using VectorXf = Eigen::VectorXf;
		using VectorXi = Eigen::VectorXi;
		using MatrixXd = Eigen::MatrixXd;
		using MatrixXf = Eigen::MatrixXf;
		using SparseMatrix = Eigen::SparseMatrix<double>;
		using Vector3d = Eigen::Vector3d;
		using Vector3f = Eigen::Vector3f;
		using Vector4d = Eigen::Vector4d;

#pragma region Array Expansion and Processing

		/// <summary>
		/// 将数组扩展到指定的目标长度，同时保持原始数组元素在计算位置的精确值，并对其他位置进行插值
		/// </summary>
		/// <param name="originalArray">要扩展的原始数组。不能为null或空。</param>
		/// <param name="targetLength">扩展数组的期望长度。必须大于原始数组的长度。</param>
		/// <param name="low">下边界值，默认为NaN表示使用数组最小值</param>
		/// <param name="up">上边界值，默认为NaN表示使用数组最大值</param>
		/// <returns>指定长度的新数组，包含原始数组在计算位置的值，其他位置为插值</returns>
		static VectorXd ExpandArrayPreserveExact(const VectorXd& originalArray, int targetLength,
			double low = std::numeric_limits<double>::quiet_NaN(),
			double up = std::numeric_limits<double>::quiet_NaN());

		/// <summary>
		/// 将值插入到已排序向量的适当位置，保持排序顺序
		/// </summary>
		/// <param name="v1">要添加值的输入向量。不能为null或空。</param>
		/// <param name="value">要插入向量的值</param>
		/// <param name="sort">指示输入向量是否应在插入值之前排序的布尔值</param>
		/// <returns>包含输入向量所有元素的新向量，指定值插入正确位置以保持排序顺序</returns>
		static VectorXd AddSortedValue(const VectorXd& v1, double value, bool sort = true);

#pragma endregion

#pragma region Cumulative Operations

		/// <summary>
		/// 计算指定向量元素的累积和
		/// </summary>
		/// <param name="v">要计算累积和的输入向量。不能为null。</param>
		/// <returns>与输入向量大小相同的新向量，其中索引i处的每个元素表示输入向量中从索引0到i的所有元素的和</returns>
		static VectorXd Cumsum(const VectorXd& v);

		/// <summary>
		/// 计算指定数组元素的累积和
		/// </summary>
		/// <param name="v">要计算累积和的输入数组。不能为null。</param>
		/// <returns>与输入数组大小相同的新数组，其中索引i处的每个元素表示输入数组中从索引0到i的所有元素的和</returns>
		static std::vector<double> Cumsum(const std::vector<double>& v);

		/// <summary>
		/// 计算指定向量元素的反向累积和（差分）
		/// </summary>
		/// <param name="v">输入向量</param>
		/// <returns>差分后的向量</returns>
		static VectorXd ReCumsum(const VectorXd& v);

		/// <summary>
		/// 计算指定数组元素的反向累积和（差分）
		/// </summary>
		/// <param name="v">输入数组</param>
		/// <returns>差分后的数组</returns>
		static std::vector<double> ReCumsum(const std::vector<double>& v);

#pragma endregion

#pragma region Search and Find Operations

		/// <summary>
		/// 查找向量中最接近指定目标值的元素的索引和值
		/// </summary>
		/// <param name="v">要搜索的值集合</param>
		/// <param name="target">要查找最接近匹配的目标值</param>
		/// <returns>包含最接近元素的索引和值的元组</returns>
		template<typename T>
		static std::tuple<int, T> FindClosestIndexAndValue(const std::vector<T>& v, T target);


		/// <summary>
		/// 对指定序列的元素按升序排序
		/// </summary>
		template<typename T>
		static std::vector<T> Sort(const std::vector<T>& values);

		/// <summary>
		/// 在已排序向量中查找第一个大于或等于指定目标值的元素的索引
		/// </summary>
		/// <param name="v">要搜索的已排序双精度浮点数向量</param>
		/// <param name="target">要在向量中搜索的目标值</param>
		/// <returns>向量中第一个大于或等于target的元素的从零开始的索引</returns>
		static int FindSortedFirst(const VectorXd& v, double target);

#pragma endregion

#pragma region Comparison and Equality

		/// <summary>
		/// 判断两个实数是否在指定容差内近似相等
		/// </summary>
		/// <param name="a">要比较的第一个实数</param>
		/// <param name="b">要比较的第二个实数</param>
		/// <param name="epsilon">两个数字被认为相等的最大允许差值，默认值为1e-5</param>
		/// <returns>如果a和b之间的绝对差值小于epsilon则返回true；否则返回false</returns>
		static bool EqualRealNos(double a, double b, double epsilon = 1e-5);

#pragma endregion

#pragma region Matrix Information

		/// <summary>
		/// 获取指定矩阵的行数和列数
		/// </summary>
		/// <param name="matrix">要确定维度的矩阵。不能为null。</param>
		/// <returns>包含矩阵行数(m)和列数(n)的元组</returns>
		static std::tuple<int, int> Size(const MatrixXd& matrix);

		/// <summary>
		/// 获取二维数组的行数和列数
		/// </summary>
		static std::tuple<int, int> Size(const std::vector<std::vector<double>>& matrix);

		/// <summary>
		/// 获取三维数组的行数、列数和深度
		/// </summary>
		static std::tuple<int, int, int> Size(const std::vector<std::vector<std::vector<double>>>& matrix);

		/// <summary>
		/// 获取矩阵指定维度的大小（与MATLAB完全一致）
		/// </summary>
		/// <param name="matrix">输入矩阵</param>
		/// <param name="a">维度参数：1表示行数，2表示列数</param>
		/// <returns>指定维度的大小</returns>
		static int Size(const MatrixXd& matrix, int a);

#pragma endregion

#pragma region Matrix Computations

		/// <summary>
		/// 基于提供的向量生成表示坐标网格的两个2D矩阵
		/// </summary>
		/// <param name="x">表示x坐标的向量</param>
		/// <param name="y">表示y坐标的向量</param>
		/// <param name="f">指示是否反转结果网格中x坐标顺序的布尔标志</param>
		/// <returns>包含两个矩阵的元组：xx(每行包含x坐标)和yy(每列包含y坐标)</returns>
		static std::tuple<MatrixXd, MatrixXd> meshgrid(const VectorXd& x, const VectorXd& y, bool f = false);

		/// <summary>
		/// 执行矩阵乘法A*B，将结果乘以alpha，并加上乘以beta的矩阵C
		/// </summary>
		/// <param name="C">要乘以beta并加到结果的矩阵，也会用最终结果更新</param>
		/// <param name="A">乘法运算中的左侧矩阵</param>
		/// <param name="B">乘法运算中的右侧矩阵</param>
		/// <param name="alpha">缩放A和B乘积的标量值</param>
		/// <param name="beta">在加到结果之前缩放矩阵C的标量值</param>
		/// <returns>执行运算后的结果矩阵：C = alpha * (A * B) + beta * C</returns>
		static MatrixXd Mul(MatrixXd& C, const MatrixXd& A, const MatrixXd& B, double alpha, double beta);

		/// <summary>
		/// 将两个矩阵相乘并将结果存储在指定的输出矩阵中
		/// </summary>
		static MatrixXd Mul(MatrixXd& C, const MatrixXd& A, const MatrixXd& B);

		/// <summary>
		/// 执行对称正定矩阵的LDL^T分解
		/// </summary>
		/// <param name="A">要分解的对称正定矩阵</param>
		/// <returns>包含两个矩阵的元组：L(对角线为1的下三角矩阵)和D(对角矩阵)</returns>
		static std::tuple<MatrixXd, MatrixXd> LDL(const MatrixXd& A);

		/// <summary>
		/// 执行对称矩阵的LDL分解（单精度版本）
		/// </summary>
		static std::tuple<MatrixXf, MatrixXf> LDL(const MatrixXf& A);

		/// <summary>
		/// 执行给定方形矩阵的PLDL分解（并行版本）
		/// </summary>
		static std::tuple<MatrixXf, MatrixXf> PLDL(const MatrixXf& A);

#pragma endregion

#pragma region File I/O Operations

		/// <summary>
		/// 从文件中读取矩阵数据
		/// </summary>
		/// <param name="filePath">包含矩阵数据的文件路径</param>
		/// <returns>从文件读取的矩阵</returns>
		static MatrixXd ReadMatrixFromFile(const std::string& filePath);

		/// <summary>
		/// 从文件读取MATLAB风格的复数矩阵并返回矩阵对象
		/// </summary>
		/// <param name="filePath">包含MATLAB风格矩阵的文件路径</param>
		/// <returns>表示从文件读取的复数矩阵</returns>
		static Eigen::MatrixXcd ReadMatlabMatrixFromFile(const std::string& filePath);

#pragma endregion

#pragma region Eigenvalue and Eigenvector Operations

		/// <summary>
		/// 计算矩阵的特征值和特征向量，可选择按特征值大小升序排序
		/// </summary>
		/// <param name="M">要计算特征值和特征向量的矩阵，必须是方形矩阵</param>
		/// <param name="sort">指示是否按升序排序特征值的布尔值</param>
		/// <returns>包含特征值和特征向量的元组</returns>
		static std::tuple<VectorXd, MatrixXd> Eig(const MatrixXd& M, bool sort = false);

		/// <summary>
		/// 计算矩阵的特征值和特征向量（单精度版本）
		/// </summary>
		static std::tuple<VectorXf, MatrixXf> Eig(const MatrixXf& M, bool sort = false);

		/// <summary>
		/// 计算广义特征值问题的特征值和特征向量
		/// </summary>
		/// <param name="K">刚度矩阵</param>
		/// <param name="M">质量矩阵</param>
		/// <param name="sort">是否排序</param>
		/// <returns>特征值向量和特征向量矩阵的元组</returns>
		static std::tuple<VectorXd, MatrixXd> Eig(const MatrixXd& K, const MatrixXd& M, bool sort = false);

#pragma endregion

#pragma region Vector Products

		/// <summary>
		/// 计算向量的外积（外积矩阵）
		/// </summary>
		/// <param name="a">向量a</param>
		/// <param name="b">向量b</param>
		/// <returns>外积矩阵</returns>
		static MatrixXd Outer(const VectorXd& a, const VectorXd& b);

		/// <summary>
		/// 计算向量的内积（点积）
		/// </summary>
		/// <param name="a">向量a</param>
		/// <param name="b">向量b</param>
		/// <returns>标量点积值</returns>
		static double Inter(const VectorXd& a, const VectorXd& b);

#pragma endregion

#pragma region Vector Concatenation

		/// <summary>
		/// 将两个可枚举序列水平连接为一个向量
		/// </summary>
		/// <param name="a">前面的序列</param>
		/// <param name="b">后面的序列</param>
		/// <returns>连接后的向量</returns>
		static VectorXd Hact(const std::vector<double>& a, const std::vector<double>& b);

		/// <summary>
		/// 将两个整数向量水平连接
		/// </summary>
		static VectorXi Hact(const VectorXi& a, const VectorXi& b);

		/// <summary>
		/// 将一个标量和一个向量连接为一个向量
		/// </summary>
		static VectorXd Hact(double a, const VectorXd& b);

		/// <summary>
		/// 将一个向量和一个标量连接为一个向量
		/// </summary>
		static VectorXd Hact(const VectorXd& a, double b);

		/// <summary>
		/// 将多个矩阵水平连接成单个矩阵
		/// </summary>
		static MatrixXd Hact(const std::vector<MatrixXd>& input);

		/// <summary>
		/// 将多个向量连接为一个向量
		/// </summary>
		static VectorXd Hact(const std::vector<VectorXd>& input);

		/// <summary>
		/// 将Vector3数组转换为Vector，通过展平其分量
		/// </summary>
		static VectorXd Hact(const std::vector<Vector3d>& input);

		/// <summary>
		/// 将多个一维数组连接为一个向量
		/// </summary>
		static VectorXd Hact(const std::vector<std::vector<double>>& input);

		/// <summary>
		/// 将多个向量垂直连接为一个矩阵
		/// </summary>
		static MatrixXd Vact(const std::vector<VectorXd>& a);

		/// <summary>
		/// 将多个矩阵垂直连接为新的矩阵
		/// </summary>
		static MatrixXd Vact(const std::vector<MatrixXd>& a);

		/// <summary>
		/// 将多个一维数组垂直连接为一个矩阵
		/// </summary>
		static MatrixXd Vact(const std::vector<std::vector<double>>& a);

		/// <summary>
		/// 将两个向量连接为一个矩阵
		/// </summary>
		/// <param name="a">向量a</param>
		/// <param name="b">向量b</param>
		/// <param name="row">布尔值，默认按列设置为false。如果按行设置为true</param>
		/// <returns>矩阵</returns>
		static MatrixXd Vact(const VectorXd& a, const VectorXd& b, bool row = false);

#pragma endregion

#pragma region Special Matrices

		/// <summary>
		/// 计算给定三维向量的反对称矩阵
		/// </summary>
		/// <param name="a">输入的三维向量</param>
		/// <returns>计算得到的3x3反对称矩阵</returns>
		static MatrixXd SkewMatrix(const VectorXd& a);

		/// <summary>
		/// 计算给定Vector3的反对称矩阵
		/// </summary>
		static MatrixXd SkewMatrix(const Vector3d& a);

		/// <summary>
		/// 计算矩阵的迹（对角线元素之和）
		/// </summary>
		static double Trace(const MatrixXd& a);

		/// <summary>
		/// 计算矩阵的行列式
		/// </summary>
		static double Det(const MatrixXd& a);

		/// <summary>
		/// 计算矩阵的正交化矩阵
		/// </summary>
		static MatrixXd Orthogonal(const MatrixXd& a);

#pragma endregion

#pragma region Difference Operations

		/// <summary>
		/// 计算输入向量中连续元素之间的离散差分
		/// </summary>
		/// <param name="input">双精度浮点数向量，向量必须包含至少两个元素</param>
		/// <returns>包含输入向量连续元素差分的向量，结果向量长度为input.Count - 1</returns>
		static VectorXd Diff(const VectorXd& input);

#pragma endregion

#pragma region Norm Calculations

		/// <summary>
		/// 计算向量的范数，默认为L2范数
		/// </summary>
		/// <param name="vector">输入向量</param>
		/// <param name="a">范数类型：1表示L1范数，2表示L2范数，3表示无穷范数</param>
		/// <returns>计算得到的范数值</returns>
		static double Norm(const VectorXd& vector, int a = 2);

		/// <summary>
		/// 计算3D向量的范数
		/// </summary>
		static double Norm(const Vector3d& vector, int a = 2);

		/// <summary>
		/// 将MathNet.Numerics向量转换为Vector3
		/// </summary>
		static Vector3f ToVec3(const VectorXd& vector);

		/// <summary>
		/// 将Vector3实例转换为Vector
		/// </summary>
		static VectorXd Vec3ToVe(const Vector3d& vector);

		/// <summary>
		/// 计算向量的指定范数（字符串版本）
		/// </summary>
		static double Norm(const VectorXd& matrix, const std::string& a);

		/// <summary>
		/// 计算矩阵的范数
		/// </summary>
		static double Norm(const MatrixXd& matrix, int a = 2);

		/// <summary>
		/// 计算矩阵的指定范数（字符串版本）
		/// </summary>
		static double Norm(const MatrixXd& matrix, const std::string& a);

#pragma endregion

#pragma region Matrix Repetition (Repmat)

		/// <summary>
		/// 通过沿指定维度多次重复向量来创建矩阵
		/// </summary>
		/// <param name="a">要重复的向量，不能为null</param>
		/// <param name="num">重复次数，必须大于0</param>
		/// <param name="dim">重复的维度：1表示沿行重复，2表示沿列重复</param>
		/// <returns>包含重复向量的矩阵</returns>
		static MatrixXd Repmat(const VectorXd& a, int num, int dim);

		/// <summary>
		/// 重复指定向量的元素指定次数
		/// </summary>
		static VectorXd Repmat(const VectorXd& a, int num);

		/// <summary>
		/// 重复指定矩阵的行或列指定次数
		/// </summary>
		static MatrixXd Repmat(const MatrixXd& a, int num, bool row = true);

		/// <summary>
		/// 沿指定维度复制单精度向量以创建矩阵
		/// </summary>
		static MatrixXf Repmat(const VectorXf& a, int num, int dim);

#pragma endregion

#pragma region Reshape Operations

		/// <summary>
		/// 将一维向量重新整形为2维矩阵
		/// </summary>
		/// <param name="vector">要重新整形的向量</param>
		/// <param name="control">控制参数数组，包含行数和列数</param>
		/// <param name="row">是否按行填充，默认false按列填充</param>
		/// <returns>重新整形后的矩阵</returns>
		static MatrixXd Reshape(const VectorXd& vector, const std::vector<int>& control, bool row = false);

		/// <summary>
		/// 将一维单精度向量重新整形为2维矩阵
		/// </summary>
		static MatrixXf Reshape(const VectorXf& vector, const std::vector<int>& control, bool row = false);

		/// <summary>
		/// 将一维数组重新整形为2维矩阵
		/// </summary>
		static MatrixXd Reshape(const std::vector<double>& vector, const std::vector<int>& control, bool row = false);

		/// <summary>
		/// 将一维向量按指定行数或列数重新整形为2维矩阵
		/// </summary>
		static MatrixXd Reshape(const VectorXd& vector, int rowNumber = 0, int columnNumber = 0, bool row = false);

		/// <summary>
		/// 将一维数组按指定行数或列数重新整形为2维矩阵
		/// </summary>
		static MatrixXd Reshape(const std::vector<double>& vector, int rowNumber = 0, int columnNumber = 0, bool row = false);

		/// <summary>
		/// 将一个矩阵按照给定大小重新整形为新矩阵
		/// </summary>
		static MatrixXd Reshape(const MatrixXd& matrix, const std::vector<int>& control, bool row = false);

		/// <summary>
		/// 将一个矩阵按照给定行数或列数重新整形为新矩阵
		/// </summary>
		static MatrixXd Reshape(const MatrixXd& matrix, int rowNumber = 0, int columnNumber = 0, bool row = false);

		/// <summary>
		/// 将一个矩阵展开为一维向量
		/// </summary>
		static VectorXd Reshape(const MatrixXd& matrix);

		/// <summary>
		/// 将一个单精度矩阵展开为一维向量
		/// </summary>
		static VectorXf Reshape(const MatrixXf& matrix);

		/// <summary>
		/// 将一个向量转换为三维数组
		/// </summary>
		static std::vector<std::vector<std::vector<double>>> Reshape(const VectorXd& vector, const std::vector<int>& control);

		/// <summary>
		/// 将一个一维数组转换为三维数组
		/// </summary>
		static std::vector<std::vector<std::vector<double>>> Reshape(const std::vector<double>& vector, const std::vector<int>& control);

#pragma endregion

#pragma region Matrix and Vector Creation (zeros, ones, eye)

		/// <summary>
		/// 返回一个与给定矩阵大小相同的矩阵
		/// </summary>
		static MatrixXd zeros(const MatrixXd& matrix, bool co = true);

		/// <summary>
		/// 返回一个与给定矩阵大小相同的稀疏零矩阵
		/// </summary>
		static SparseMatrix zerosp(const MatrixXd& matrix);

		/// <summary>
		/// 返回一个与给定单精度矩阵大小相同的矩阵
		/// </summary>
		static MatrixXf zeros(const MatrixXf& matrix, bool co = true);

		/// <summary>
		/// 返回一个与给定矩阵列表大小相同的矩阵列表
		/// </summary>
		static std::vector<MatrixXd> zeros(const std::vector<MatrixXd>& matrix, bool co = true);

		/// <summary>
		/// 将矩阵转换为二维数组
		/// </summary>
		static std::vector<std::vector<double>> ConvertToDouble2(const MatrixXd& matrix);

		/// <summary>
		/// 将单精度矩阵转换为二维数组
		/// </summary>
		static std::vector<std::vector<float>> ConvertTofloat2(const MatrixXf& matrix);

		/// <summary>
		/// 将向量转换为一维数组
		/// </summary>
		static std::vector<double> zeros(const VectorXd& vector);

		/// <summary>
		/// 将指定向量的所有元素设为零
		/// </summary>
		static void zeros(VectorXd& vector);

		/// <summary>
		/// 将指定矩阵的所有元素设为零
		/// </summary>
		static void zeros(MatrixXd& mat);

		/// <summary>
		/// 将单精度向量转换为一维数组
		/// </summary>
		static std::vector<float> zeros(const VectorXf& vector);

		/// <summary>
		/// 返回一个与给定向量大小相同的向量
		/// </summary>
		static VectorXd zeros(const VectorXd& vector, bool co = true);

		/// <summary>
		/// 返回一个与给定单精度向量大小相同的向量
		/// </summary>
		static VectorXf fzeros(const VectorXf& vector, bool co = true);

		/// <summary>
		/// 将二维数组转换为矩阵
		/// </summary>
		static MatrixXd zeros(const std::vector<std::vector<double>>& matrix);

		/// <summary>
		/// 将二维单精度数组转换为矩阵
		/// </summary>
		static MatrixXf fzeros(const std::vector<std::vector<float>>& matrix);

		/// <summary>
		/// 将一维数组转换为向量
		/// </summary>
		static VectorXd zeros(const std::vector<double>& matrix);

		/// <summary>
		/// 将一维单精度数组转换为向量
		/// </summary>
		static VectorXf fzeros(const std::vector<float>& matrix);

		/// <summary>
		/// 将整数数组转换为双精度向量
		/// </summary>
		static VectorXd zeros(const std::vector<int>& matrix);

		/// <summary>
		/// 将整数数组转换为整数向量
		/// </summary>
		static VectorXi zeros(const std::vector<int>& matrix, bool unused = true);

		/// <summary>
		/// 生成指定维度的三维数组，并可选择初始值
		/// </summary>
		static std::vector<std::vector<std::vector<double>>> zeros(int m, int n, int x, double value);

		/// <summary>
		/// 计算第二类修正贝塞尔函数K_nu(z)
		/// </summary>
		static double Besselk(double nu, double z);

		/// <summary>
		/// 计算第二类修正贝塞尔函数K_nu(z)（单精度版本）
		/// </summary>
		static float Besselk(float nu, float z);

		/// <summary>
		/// 对向量的每个元素计算第二类修正贝塞尔函数K_nu(z)
		/// </summary>
		static VectorXd Besselk(double nu, const VectorXd& z);

		/// <summary>
		/// 对矩阵的每个元素计算第二类修正贝塞尔函数K_nu(z)
		/// </summary>
		static MatrixXd Besselk(double nu, const MatrixXd& z);

		/// <summary>
		/// 对单精度向量的每个元素计算第二类修正贝塞尔函数K_nu(z)
		/// </summary>
		static VectorXf Besselk(float nu, const VectorXf& z);

		/// <summary>
		/// 对单精度矩阵的每个元素计算第二类修正贝塞尔函数K_nu(z)
		/// </summary>
		static MatrixXf Besselk(float nu, const MatrixXf& z);

		/// <summary>
		/// 计算伽马函数Γ(x)
		/// </summary>
		static double Gamma(double x);

		/// <summary>
		/// 计算伽马函数Γ(x)（单精度版本）
		/// </summary>
		static float Gamma(float x);

		/// <summary>
		/// 对向量的每个元素计算伽马函数Γ(x)
		/// </summary>
		static VectorXd Gamma(double nu, const VectorXd& z);

		/// <summary>
		/// 对矩阵的每个元素计算伽马函数Γ(x)
		/// </summary>
		static MatrixXd Gamma(double nu, const MatrixXd& z);

		/// <summary>
		/// 对单精度向量的每个元素计算伽马函数Γ(x)
		/// </summary>
		static VectorXf Gamma(float nu, const VectorXf& z);

		/// <summary>
		/// 对单精度矩阵的每个元素计算伽马函数Γ(x)
		/// </summary>
		static MatrixXf Gamma(float nu, const MatrixXf& z);

		/// <summary>
		/// 生成指定维度的双精度零矩阵
		/// </summary>
		static MatrixXd zeros(int m, int n);

		/// <summary>
		/// 生成指定维度的稀疏零矩阵
		/// </summary>
		static SparseMatrix zerosp(int m, int n);

		/// <summary>
		/// 生成指定维度的单精度零矩阵
		/// </summary>
		static MatrixXf fzeros(int m, int n);

		/// <summary>
		/// 生成指定长度的双精度零向量
		/// </summary>
		static VectorXd zeros(int m);

		/// <summary>
		/// 生成指定长度的单精度零向量
		/// </summary>
		static VectorXf fzeros(int m);

		/// <summary>
		/// 生成指定数量的矩阵列表
		/// </summary>
		static std::vector<MatrixXd> zeros(int row, int column, int num);

		/// <summary>
		/// 生成指定数量的单精度矩阵列表
		/// </summary>
		static std::vector<MatrixXf> fzeros(int row, int column, int num);

		/// <summary>
		/// 生成嵌套的矩阵列表
		/// </summary>
		static std::vector<std::vector<MatrixXd>> zeros(int row, int column, int num, int num1);

		///// <summary>
		///// 创建向量的副本
		///// </summary>
		//static VectorXd Copy(const VectorXd& x);

		///// <summary>
		///// 创建嵌套矩阵列表的深拷贝
		///// </summary>
		//static std::vector<std::vector<MatrixXd>> Copy(const std::vector<std::vector<MatrixXd>>& matrix);

		///// <summary>
		///// 创建矩阵列表的深拷贝
		///// </summary>
		//static std::vector<MatrixXd> Copy(const std::vector<MatrixXd>& matrix);

		//template<typename T >
		///// <summary>
		///// 创建字符串数组的副本
		///// </summary>
		//static std::vector<T> Copy(const std::vector<T>& ori);

		/// <summary>
		/// 创建五维数组的深拷贝
		/// </summary>
		template<typename T>
		static std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>> Copy(
			const std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>>& ori);

		/// <summary>
		/// 创建四维数组的深拷贝
		/// </summary>
		template<typename T>
		static std::vector<std::vector<std::vector<std::vector<T>>>> Copy(
			const std::vector<std::vector<std::vector<std::vector<T>>>>& ori);

		/// <summary>
		/// 创建三维数组的深拷贝
		/// </summary>
		template<typename T>
		static std::vector<std::vector<std::vector<T>>> Copy(const std::vector<std::vector<std::vector<T>>>& ori);

		/// <summary>
		/// 创建二维数组的深拷贝（高性能版本）
		/// </summary>
		template<typename T>
		static std::vector<std::vector<T>> Copy(const std::vector<std::vector<T>>& ori);

		/// <summary>
		/// 创建一维数组的深拷贝（高性能版本）
		/// </summary>
		template<typename T>
		static std::vector<T> Copy(const std::vector<T>& ori);

		template<typename T>
		static T Copy(const T& ori);

		/// <summary>
		/// 创建n×n单位矩阵（高性能版本）
		/// </summary>
		static MatrixXd Eye(int n);

		/// <summary>
		/// 创建n×n矩阵，对角线元素为指定值（高性能版本）
		/// </summary>
		static MatrixXd Eye(int n, double value);

		/// <summary>
		/// 创建n×m单位矩阵（高性能版本）
		/// </summary>
		static MatrixXd Eye(int n, int m);

		/// <summary>
		/// 创建对角矩阵，对角线元素由向量指定（高性能版本）
		/// </summary>
		static MatrixXd Eye(const VectorXd& diag);

		/// <summary>
		/// 创建对角矩阵，对角线元素由数组指定（高性能版本）
		/// </summary>
		static MatrixXd Eye(const std::vector<double>& diag);

		/// <summary>
		/// 创建n×n单精度单位矩阵（高性能版本）
		/// </summary>
		static MatrixXf fEye(int n);

		/// <summary>
		/// 创建n×n单精度矩阵，对角线元素为指定值（高性能版本）
		/// </summary>
		static MatrixXf Eye(int n, float value);

		/// <summary>
		/// 创建n×m的单精度单位矩阵（高性能版本）
		/// </summary>
		static MatrixXf fEye(int n, int m);

		/// <summary>
		/// 创建对角矩阵，对角线元素由单精度向量指定（高性能版本）
		/// </summary>
		static MatrixXf Eye(const VectorXf& diag);

		/// <summary>
		/// 创建对角矩阵，对角线元素由单精度数组指定（高性能版本）
		/// </summary>
		static MatrixXf Eye(const std::vector<float>& diag);

		/// <summary>
		/// 生成双精度对角矩阵（高性能版本）
		/// </summary>
		static MatrixXd Diag(const VectorXd& a);

		/// <summary>
		/// 生成单精度对角矩阵（高性能版本）
		/// </summary>
		static MatrixXf Diag(const VectorXf& a);

		/// <summary>
		/// 生成双精度对角矩阵，支持可变参数（高性能版本）
		/// </summary>
		static MatrixXd Diag(const std::vector<double>& a);

		/// <summary>
		/// 生成元素全为1的双精度矩阵（高性能版本）
		/// </summary>
		static MatrixXd ones(int n, int m);

		/// <summary>
		/// 生成元素全为1的双精度向量
		/// </summary>
		static VectorXd ones(int n);

#pragma endregion

#pragma region Vector Cross and Dot Products

		/// <summary>
		/// 计算双精度向量的点积并将结果赋值给引用参数（高性能版本）
		/// </summary>
		static void dot(const VectorXd& a, const VectorXd& b, double& outn);

		/// <summary>
		/// 计算单精度向量的点积并将结果赋值给引用参数（高性能版本）
		/// </summary>
		static void dot(const VectorXf& a, const VectorXf& b, float& outn);

		/// <summary>
		/// 计算双精度向量的点积（高性能版本）
		/// </summary>
		static double dot(const VectorXd& a, const VectorXd& b);

		/// <summary>
		/// 计算单精度向量的点积（高性能版本）
		/// </summary>
		static float dot(const VectorXf& a, const VectorXf& b);

		/// <summary>
		/// 计算双精度矩阵的点积，可以根据矩阵的形态自动识别维度为3的行或列进行计算（高性能版本）
		/// </summary>
		static VectorXd dot(const MatrixXd& a, const MatrixXd& b, int dim);

		/// <summary>
		/// 计算单精度矩阵的点积，可以根据矩阵的形态自动识别维度为3的行或列进行计算（高性能版本）
		/// </summary>
		static VectorXf dot(const MatrixXf& a, const MatrixXf& b, int dim);

		/// <summary>
		/// 计算双精度矩阵的点积并将结果存储到引用向量中（高性能版本）
		/// </summary>
		static void dot(const MatrixXd& a, const MatrixXd& b, int dim, VectorXd& c);

		/// <summary>
		/// 计算双精度向量的叉积并将结果存储到引用向量中（高性能版本）
		/// </summary>
		static void cross(const VectorXd& a, const VectorXd& b, VectorXd& crosstemp);

		/// <summary>
		/// 计算双精度向量的叉积（高性能版本）
		/// </summary>
		static VectorXd cross(const VectorXd& a, const VectorXd& b);

		/// <summary>
		/// 计算双精度矩阵的叉积，可以根据矩阵的形态自动识别维度为3的行或列进行计算（高性能版本）
		/// </summary>
		static MatrixXd cross(const MatrixXd& a, const MatrixXd& b, int dim);

		/// <summary>
		/// 计算双精度矩阵的叉积并将结果存储到引用矩阵中（高性能版本）
		/// </summary>
		static void cross(const MatrixXd& a, const MatrixXd& b, int dim, MatrixXd& c);

		/// <summary>
		/// 计算单精度向量的叉积并将结果存储到引用向量中（高性能版本）
		/// </summary>
		static void cross(const VectorXf& a, const VectorXf& b, VectorXf& crosstemp);

		/// <summary>
		/// 计算单精度向量的叉积（高性能版本）
		/// </summary>
		static VectorXf cross(const VectorXf& a, const VectorXf& b);

		/// <summary>
		/// 计算单精度矩阵的叉积，可以根据矩阵的形态自动识别维度为3的行或列进行计算（高性能版本）
		/// </summary>
		static MatrixXf cross(const MatrixXf& a, const MatrixXf& b, int dim);

		/// <summary>
		/// 计算单精度矩阵的叉积并将结果存储到引用矩阵中（高性能版本）
		/// </summary>
		static void cross(const MatrixXf& a, const MatrixXf& b, int dim, MatrixXf& c);

#pragma endregion

#pragma region Convolution Operations

		/// <summary>
		/// 计算两个输入数组的离散线性卷积
		/// </summary>
		/// <param name="a">第一个输入数组，不能为null</param>
		/// <param name="b">第二个输入数组，不能为null</param>
		/// <returns>表示两个输入数组卷积的数组，长度为 a.Length + b.Length - 1</returns>
		static std::vector<double> Conv(const std::vector<double>& a, const std::vector<double>& b);

		/// <summary>
		/// 计算两个向量的离散线性卷积
		/// </summary>
		static VectorXd Conv(const VectorXd& a, const VectorXd& b);

#pragma endregion

#pragma region Linear System Solver

		/// <summary>
		/// 使用共轭梯度法求解线性方程组
		/// </summary>
		/// <param name="A">线性系统的系数矩阵，必须是方形、对称且正定的矩阵</param>
		/// <param name="b">线性系统的右侧向量</param>
		/// <param name="MaxStep">执行的最大迭代次数，默认为500</param>
		/// <param name="error">收敛容差，当残差误差小于此值时方法停止，默认为1E-15</param>
		/// <returns>表示线性系统解的向量，如果方法收敛，这是近似解</returns>
		static VectorXd ConjugateGradientSolver(const MatrixXd& A, const VectorXd& b, int MaxStep = 500, double error = 1E-15);

#pragma endregion

#pragma region Matrix Operations

		/// <summary>
		/// 提取指定矩阵的上三角部分，可选择通过给定的主元数进行偏移
		/// </summary>
		/// <param name="matrix">要提取上三角部分的输入矩阵</param>
		/// <param name="pc">用于偏移结果矩阵行和列的主元数，值为0时提取标准上三角矩阵，必须非负</param>
		/// <returns>包含输入矩阵上三角部分的新矩阵，从指定的主元偏移开始</returns>
		static MatrixXd Triu(const MatrixXd& matrix, int pc = 0);

		/// <summary>
		/// 通过填充上三角或下三角中的缺失值来确保指定方形矩阵的对称性
		/// </summary>
		static void FillTriangle(MatrixXd& matrix, int pc = 0, const std::string& errlog = "");

#pragma endregion

#pragma region System Function Extensions

		/// <summary>
		/// 创建指定数量元素的Vector3实例数组，每个元素初始化为默认值（高性能版本）
		/// </summary>
		static std::vector<Vector3d> zerovec3(int row);

		/// <summary>
		/// 创建Vector3实例的二维数组，初始化为默认值（高性能版本）
		/// </summary>
		static std::vector<std::vector<Vector3d>> zerovec3(int row, int col);

		/// <summary>
		/// 创建一个三维的 Vector3 数组，所有元素初始化为默认零向量
		/// </summary>
		static std::vector<std::vector<std::vector<Vector3d>>> zerovec3(int row, int col, int num1);

		/// <summary>
		/// 创建一个四维的 Vector3 数组，所有元素初始化为默认零向量
		/// </summary>
		static std::vector<std::vector<std::vector<std::vector<Vector3d>>>> zerovec3(int row, int col, int num1, int num2);

#pragma endregion

#pragma region Statistical Operations (Mean, NaN handling)

		/// <summary>
		/// 计算双精度矩阵在指定维度上的均值，将 NaN 值视为零处理
		/// </summary>
		/// <param name="matr">要计算均值的输入矩阵</param>
		/// <param name="dim">计算均值的维度。使用 1 计算沿列方向的均值（按行计算），使用 2 计算沿行方向的均值（按列计算）</param>
		/// <returns>包含沿指定维度计算的均值的向量</returns>
		static VectorXd nanmean(const MatrixXd& matr, int dim);

		/// <summary>
		/// 计算单精度矩阵在指定维度上的均值，忽略 NaN 值
		/// </summary>
		static VectorXf nanmean(const MatrixXf& matr, int dim);

		/// <summary>
		/// 计算单精度向量的均值，将 NaN 值视为零
		/// </summary>
		static double nanmean(const VectorXf& vec);

		/// <summary>
		/// 计算双精度矩阵在指定维度上的元素和，将 NaN 值视为零
		/// </summary>
		static VectorXd nansum(const MatrixXd& matr, int dim);

		/// <summary>
		/// 计算单精度矩阵在指定维度上的元素和，将 NaN 值视为零
		/// </summary>
		static VectorXf nansum(const MatrixXf& matr, int dim);

		/// <summary>
		/// 计算单精度向量所有元素的和，将 NaN 值视为零
		/// </summary>
		static float nansum(const VectorXf& vec);

		/// <summary>
		/// 计算双精度向量所有元素的和，将 NaN 值视为零
		/// </summary>
		static double nansum(const VectorXd& vec);

		/// <summary>
		/// 计算三维双精度数组在指定维度上的均值
		/// </summary>
		static std::vector<std::vector<double>> mean(const std::vector<std::vector<std::vector<double>>>& matr, int dim);

		/// <summary>
		/// 计算多个双精度矩阵中每行每列对应位置的均值
		/// </summary>
		static std::vector<std::vector<double>> mean(const std::vector<MatrixXd>& matr);

		/// <summary>
		/// 计算多个单精度矩阵中每行跨列的均值
		/// </summary>
		static std::vector<std::vector<float>> mean(const std::vector<MatrixXf>& matr);

		/// <summary>
		/// 计算双精度矩阵集合沿指定维度的均值
		/// </summary>
		static std::vector<std::vector<double>> mean(const std::vector<MatrixXd>& matr, int dim);

		/// <summary>
		/// 计算单精度矩阵集合沿指定维度的均值
		/// </summary>
		static std::vector<std::vector<float>> mean(const std::vector<MatrixXf>& matr, int dim);

		/// <summary>
		/// 计算三维双精度数组在指定维度上的均值并返回矩阵表示
		/// </summary>
		static MatrixXd mean(const std::vector<std::vector<std::vector<double>>>& matr, int dim, int b = 11);

		/// <summary>
		/// 计算二维双精度数组在指定维度上的均值
		/// </summary>
		static VectorXd mean(const std::vector<std::vector<double>>& matr, int dim);

		/// <summary>
		/// 计算二维单精度数组在指定维度上的均值
		/// </summary>
		static VectorXf mean(const std::vector<std::vector<float>>& matr, int dim);

		/// <summary>
		/// 计算双精度矩阵在指定维度上的均值
		/// </summary>
		static VectorXd mean(const MatrixXd& matr, int dim);

		/// <summary>
		/// 计算单精度矩阵在指定维度上的均值
		/// </summary>
		static VectorXf mean(const MatrixXf& matr, int dim);

		/// <summary>
		/// 计算二维矩阵沿指定维度的均值并返回 Vector3 结果
		/// </summary>
		static Vector3d mean(const std::vector<std::vector<double>>& matr, int dim, bool vec3 = false);

		/// <summary>
		/// 计算二维双精度数组中所有元素的均值（平均值）
		/// </summary>
		static double mean(const std::vector<std::vector<double>>& array);

#pragma endregion

#pragma region Histogram Functions

		/// <summary>
		/// 计算给定数据的二维直方图，沿 X 和 Y 轴将数据分割到指定数量的区间中
		/// </summary>
		/// <param name="xData">沿 X 轴的数据点数组，必须包含至少两个元素</param>
		/// <param name="yData">沿 Y 轴的数据点数组，必须包含至少两个元素</param>
		/// <param name="xBins">将 X 轴分割的区间数量，必须大于等于 1</param>
		/// <param name="yBins">将 Y 轴分割的区间数量，必须大于等于 1</param>
		/// <param name="weight">与数据点对应的可选权重数组。如果提供，权重将用于计算直方图值。如果为 null，所有数据点被等同对待</param>
		/// <returns>包含二维直方图计数矩阵、X轴区间边界和Y轴区间边界的元组</returns>
		static std::tuple<MatrixXd, std::vector<double>, std::vector<double>> Histogram2D(
			const std::vector<double>& xData, const std::vector<double>& yData,
			int xBins, int yBins, const std::vector<double>& weight = std::vector<double>());

		/// <summary>
		/// 计算给定数据的二维直方图，返回两个维度的区间计数和边界
		/// </summary>
		static std::tuple<MatrixXd, std::vector<double>, std::vector<double>> Histogram2D(
			const std::vector<double>& xData, const std::vector<double>& yData,
			int xBins, const std::vector<double>& yBins,
			const std::vector<double>& weight = std::vector<double>());

		/// <summary>
		/// 计算给定数据的二维直方图，返回两个维度的区间计数和边界
		/// </summary>
		static std::tuple<MatrixXd, std::vector<double>, std::vector<double>> Histogram2D(
			const std::vector<double>& xData, const std::vector<double>& yData,
			const std::vector<double>& xBins, int yBins,
			const std::vector<double>& weight = std::vector<double>());

		/// <summary>
		/// 为给定的数据点和区间边界计算二维直方图
		/// </summary>
		static std::tuple<MatrixXd, std::vector<double>, std::vector<double>> Histogram2D(
			const std::vector<double>& xData, const std::vector<double>& yData,
			const std::vector<double>& xBins, const std::vector<double>& yBins,
			const std::vector<double>& weight = std::vector<double>());

#pragma endregion

	private:
		/// <summary>
		/// 基于提供的数据点、区间边界和可选权重计算二维直方图矩阵的核心计算方法
		/// </summary>
		static MatrixXd Histogram2DCompute(const std::vector<double>& xData, const std::vector<double>& yData,
			const std::vector<double>& xBins, const std::vector<double>& yBins,
			const std::vector<double>& weight = std::vector<double>());
	};

}
#endif
