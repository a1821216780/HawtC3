//**********************************************************************************************************************************
//LICENSING
// Copyright(C) 2021, 2025  TG Team,Key Laboratory of Jiangsu province High-Tech design of wind turbine,WTG,WL,������
//
//    This file is part of HawtC3.IO.Log.
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

#ifndef IOUTFILE_H
#define IOUTFILE_H

#include <string>
#include <vector>
#include <Eigen/Dense>

namespace HawtC3 {
	namespace IO {
		namespace Interface {

			/**
			 * @brief �ļ�����ӿڣ������˸����������͵��������
			 *
			 * �˽ӿ��ṩ��ͳһ���ļ�������ܣ�֧�ֻ����������͡�����������ȵĸ�ʽ�������
			 * ʹ��Eigen����Ϊ����������ĵײ�ʵ�֡�
			 */
			class IOutFile {
			public:
				virtual ~IOutFile() = default;

				/**
				 * @brief ��ȡ���ʵ���������ļ���
				 * @return �ļ����ַ���
				 */
				virtual std::string GetFilename() = 0;

				/**
				 * @brief ��ȡ�����ļ�·��
				 * @return �ļ�·���ַ���
				 */
				virtual std::string GetStreamFilePath() = 0;

				/**
				 * @brief ���������ļ�·��
				 * @param path �µ��ļ�·��
				 */
				virtual void SetStreamFilePath(const std::string& path) = 0;

				/**
				 * @brief ���������̲���ѡ��ɾ����ʱ��Դ
				 *
				 * ʹ�ô˷������������������ remove ����Ϊ true��
				 * ����ڹ����������������κ���ʱ��Դ��
				 *
				 * @param remove ָʾ�����ʱ�Ƿ�ɾ����ʱ��Դ��Ĭ��Ϊtrue
				 */
				virtual void Outfinish(bool remove = true) = 0;

				/**
				 * @brief ������з�
				 */
				virtual void WriteLine() = 0;

				/**
				 * @brief ��ָ������ֵ��Ϣд����������������ֹ��
				 * @param message Ҫд���������ֵ��Ϣ
				 */
				virtual void WriteLine(double message) = 0;

				/**
				 * @brief ����ַ��������һ���
				 * @param message �ַ������͵���Ϣ
				 */
				virtual void WriteLine(const std::string& message) = 0;

				/**
				 * @brief ����ַ������ݲ�����
				 * @param message Ҫ������ַ�����Ϣ
				 * @param fg �ָ�����Ĭ��Ϊ�Ʊ���
				 */
				virtual void Write(const std::string& message, const std::string& fg = "\t") = 0;

				/**
				 * @brief ��˫����ֵд���������ѡ��ָ��ǰ׺�ַ���
				 * @param message Ҫд���˫����ֵ
				 * @param fg ��ѡ��ǰ׺�ַ�����Ĭ��Ϊ�Ʊ���
				 */
				virtual void Write(double message, const std::string& fg = "\t") = 0;

				/**
				 * @brief ��ָ��������д����������п�ѡ��ǰ׺�ַ���
				 * @param message Ҫд�������˫��������
				 * @param fg ��ѡ���ַ���ǰ׺��Ĭ��Ϊ�Ʊ���
				 */
				virtual void Write(const Eigen::VectorXd& message, const std::string& fg = "\t") = 0;

				/**
				 * @brief ��ָ����������Ϣд����������п�ѡ��ǰ׺�ַ���
				 * @param message Ҫд��ĵ�����������Ϣ
				 * @param fg ��ѡ��ǰ׺�ַ�����Ĭ��Ϊ�Ʊ���
				 */
				virtual void Write(const Eigen::VectorXf& message, const std::string& fg = "\t") = 0;

				/**
				 * @brief ��һ��������Ϣд�������ÿ����Ϣ������з�
				 * @param messages ��ʾҪд����Ϣ���ַ���������ÿ����Ϣ��д��������
				 */
				virtual void WriteLine(const std::vector<std::string>& messages, bool row = false) = 0;

				/**
				 * @brief ������������һ���
				 * @param message Ҫ�����˫��������
				 * @param decimalPlaces С��λ����Ĭ��Ϊ10
				 */
				virtual void WriteLine(const Eigen::VectorXd& message, int decimalPlaces = 10) = 0;

				/**
				 * @brief ��ָ������д���������ÿ��Ԫ�ظ�ʽ��Ϊָ����С��λ��
				 *
				 * ������ÿ��Ԫ�ض���������ʽ������˳��д�롣
				 * �����ʽ��ָ����С��λ��ȷ����
				 *
				 * @param message Ҫд������ĵ���������
				 * @param decimalPlaces ��ʽ������ÿ��Ԫ�ص�С��λ����Ĭ��Ϊ10
				 */
				virtual void WriteLine(const Eigen::VectorXf& message, int decimalPlaces = 10) = 0;

				/**
				 * @brief ������������һ���
				 * @param message Ҫ�����˫���Ⱦ���
				 */
				virtual void WriteLine(const Eigen::MatrixXd& message) = 0;

				/**
				 * @brief ��ָ������д�������
				 *
				 * �÷������ʺϵײ�������ĸ�ʽ�������
				 * ȷ���ṩ�ľ���Ϊ���Ա���������Ϊ��
				 *
				 * @param message Ҫд��ĵ����Ⱦ���
				 */
				virtual void WriteLine(const Eigen::MatrixXf& message) = 0;

				/**
				 * @brief �����ά���������һ���
				 * @param message Ҫ����Ķ�ά˫��������
				 */
				virtual void WriteLine(const std::vector<std::vector<double>>& message) = 0;



				// ����Ĺ̶���С�����;���֧�֣������Ҫ�����Ż���

				/**
				 * @brief д��̶���С����
				 * @tparam Size ������С
				 * @param message �̶���С��˫��������
				 * @param fg �ָ�����Ĭ��Ϊ�Ʊ���
				 */
				template<int Size>
				void Write(const Eigen::Matrix<double, Size, 1>& message, const std::string& fg = "\t") {
					Write(message.template cast<Eigen::VectorXd>(), fg);
				}

				/**
				 * @brief д��̶���С����
				 * @tparam Rows ��������
				 * @tparam Cols ��������
				 * @param message �̶���С��˫���Ⱦ���
				 * @param fg �ָ�����Ĭ��Ϊ�Ʊ���
				 */
				template<int Rows, int Cols>
				void Write(const Eigen::Matrix<double, Rows, Cols>& message, const std::string& fg = "\t") {
					Write(message.template cast<Eigen::MatrixXd>(), fg);
				}

				/**
				 * @brief д��̶���С����������
				 * @tparam Size ������С
				 * @param message �̶���С��˫��������
				 * @param decimalPlaces С��λ����Ĭ��Ϊ10
				 */
				template<int Size>
				void WriteLine(const Eigen::Matrix<double, Size, 1>& message, int decimalPlaces = 10) {
					WriteLine(message.template cast<Eigen::VectorXd>(), decimalPlaces);
				}

				/**
				 * @brief д��̶���С���󲢻���
				 * @tparam Rows ��������
				 * @tparam Cols ��������
				 * @param message �̶���С��˫���Ⱦ���
				 */
				template<int Rows, int Cols>
				void WriteLine(const Eigen::Matrix<double, Rows, Cols>& message) {
					WriteLine(message.template cast<Eigen::MatrixXd>());
				}
			};

			// Ϊ�˱�����ԭʼC#�ӿڵļ����ԣ��ṩ���ͱ���
			using Vector = Eigen::VectorXd;
			using VectorF = Eigen::VectorXf;
			using Matrix = Eigen::MatrixXd;
			using MatrixF = Eigen::MatrixXf;

			// ���õĹ̶���С���ͱ���
			using Vector2d = Eigen::Vector2d;
			using Vector3d = Eigen::Vector3d;
			using Vector4d = Eigen::Vector4d;
			using Vector2f = Eigen::Vector2f;
			using Vector3f = Eigen::Vector3f;
			using Vector4f = Eigen::Vector4f;

			using Matrix2d = Eigen::Matrix2d;
			using Matrix3d = Eigen::Matrix3d;
			using Matrix4d = Eigen::Matrix4d;
			using Matrix2f = Eigen::Matrix2f;
			using Matrix3f = Eigen::Matrix3f;
			using Matrix4f = Eigen::Matrix4f;

		} // namespace Interface1
	} // namespace IO
} // namespace HawtC3

#endif // IOUTFILE_H
