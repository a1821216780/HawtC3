// test_frequence_helper.cpp - FrequenceHelper 类的单元测试
#include <gtest/gtest.h>
#include <complex>
#include <vector>
#include <cmath>
#include <Eigen/Dense>
#include "../../../src/IO/Math/FrequenceHelper.h"

using namespace HawtC3::IO::Math;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// vector<complex<double>> FFT / IFFT
// ============================================================================

TEST(FrequenceHelperTest, Fft_Ifft_Roundtrip_ComplexDouble) {
	// 时域信号：4 个样本
	std::vector<std::complex<double>> input = {
		{1.0, 0.0}, {2.0, 0.0}, {3.0, 0.0}, {4.0, 0.0}
	};
	auto spectrum = FrequenceHelper::Fft(input);
	auto recovered = FrequenceHelper::Ifft(spectrum);

	ASSERT_EQ(recovered.size(), input.size());
	for (size_t i = 0; i < input.size(); ++i) {
		EXPECT_NEAR(recovered[i].real(), input[i].real(), 1e-10);
		EXPECT_NEAR(recovered[i].imag(), input[i].imag(), 1e-10);
	}
}

TEST(FrequenceHelperTest, Fft_DC_Signal_ComplexDouble) {
	// 纯直流信号: [1,1,1,1]
	// FFT 应该在 bin 0 得到 N，其余为 0
	std::vector<std::complex<double>> dc(4, {1.0, 0.0});
	auto spectrum = FrequenceHelper::Fft(dc);

	EXPECT_NEAR(spectrum[0].real(), 4.0, 1e-10);
	EXPECT_NEAR(spectrum[0].imag(), 0.0, 1e-10);
	for (size_t i = 1; i < spectrum.size(); ++i) {
		EXPECT_NEAR(std::abs(spectrum[i]), 0.0, 1e-10);
	}
}

// ============================================================================
// vector<complex<float>> FFT / IFFT
// ============================================================================

TEST(FrequenceHelperTest, Fft_Ifft_Roundtrip_ComplexFloat) {
	std::vector<std::complex<float>> input = {
		{1.0f, 0.5f}, {2.0f, -0.5f}, {3.0f, 1.0f}, {4.0f, -1.0f}
	};
	auto spectrum = FrequenceHelper::Fft(input);
	auto recovered = FrequenceHelper::Ifft(spectrum);

	ASSERT_EQ(recovered.size(), input.size());
	for (size_t i = 0; i < input.size(); ++i) {
		EXPECT_NEAR(recovered[i].real(), input[i].real(), 1e-4f);
		EXPECT_NEAR(recovered[i].imag(), input[i].imag(), 1e-4f);
	}
}

// ============================================================================
// pair<vector<double>> Fft(real, imag)
// ============================================================================

TEST(FrequenceHelperTest, Fft_RealImag_Pair) {
	std::vector<double> real = {1.0, 2.0, 3.0, 4.0};
	std::vector<double> imag = {0.0, 0.0, 0.0, 0.0};
	auto [outReal, outImag] = FrequenceHelper::Fft(real, imag);

	ASSERT_EQ(outReal.size(), real.size());
	ASSERT_EQ(outImag.size(), imag.size());

	// bin 0 实部应为和 = 10.0
	EXPECT_NEAR(outReal[0], 10.0, 1e-10);
	EXPECT_NEAR(outImag[0], 0.0, 1e-10);
}

// ============================================================================
// Eigen::VectorXcd FFT / IFFT
// ============================================================================

TEST(FrequenceHelperTest, Fft_Ifft_Roundtrip_VectorXcd) {
	Eigen::VectorXcd input(4);
	input << std::complex<double>(1, 0),
			 std::complex<double>(0, 1),
			 std::complex<double>(-1, 0),
			 std::complex<double>(0, -1);

	auto spectrum = FrequenceHelper::Fft(input);
	auto recovered = FrequenceHelper::Ifft(spectrum);

	ASSERT_EQ(recovered.size(), input.size());
	for (int i = 0; i < input.size(); ++i) {
		EXPECT_NEAR(recovered(i).real(), input(i).real(), 1e-10);
		EXPECT_NEAR(recovered(i).imag(), input(i).imag(), 1e-10);
	}
}

// ============================================================================
// Eigen::VectorXd FFT (实数向量 -> 复数频谱)
// ============================================================================

TEST(FrequenceHelperTest, Fft_VectorXd) {
	Eigen::VectorXd input(4);
	input << 1.0, 2.0, 3.0, 4.0;

	auto spectrum = FrequenceHelper::Fft(input);
	ASSERT_EQ(spectrum.size(), 4);

	// 直流分量
	EXPECT_NEAR(spectrum(0).real(), 10.0, 1e-10);
	EXPECT_NEAR(spectrum(0).imag(), 0.0, 1e-10);
}

// ============================================================================
// Eigen::VectorXcf FFT / IFFT
// ============================================================================

TEST(FrequenceHelperTest, Fft_Ifft_Roundtrip_VectorXcf) {
	Eigen::VectorXcf input(4);
	input << std::complex<float>(1, 0),
			 std::complex<float>(2, 0),
			 std::complex<float>(3, 0),
			 std::complex<float>(4, 0);

	auto spectrum = FrequenceHelper::Fft(input);
	auto recovered = FrequenceHelper::Ifft(spectrum);

	ASSERT_EQ(recovered.size(), input.size());
	for (int i = 0; i < input.size(); ++i) {
		EXPECT_NEAR(recovered(i).real(), input(i).real(), 1e-4f);
		EXPECT_NEAR(recovered(i).imag(), input(i).imag(), 1e-4f);
	}
}

// ============================================================================
// Eigen::VectorXf FFT (实数向量 -> 复数频谱)
// ============================================================================

TEST(FrequenceHelperTest, Fft_VectorXf) {
	Eigen::VectorXf input(4);
	input << 1.0f, 2.0f, 3.0f, 4.0f;

	auto spectrum = FrequenceHelper::Fft(input);
	ASSERT_EQ(spectrum.size(), 4);

	EXPECT_NEAR(spectrum(0).real(), 10.0f, 1e-4f);
	EXPECT_NEAR(spectrum(0).imag(), 0.0f, 1e-4f);
}

// ============================================================================
// Eigen::MatrixXcd FFT / IFFT（逐列变换）
// ============================================================================

TEST(FrequenceHelperTest, Fft_Ifft_Roundtrip_MatrixXcd) {
	Eigen::MatrixXcd input(4, 2);
	input.col(0) << std::complex<double>(1, 0), std::complex<double>(2, 0),
					 std::complex<double>(3, 0), std::complex<double>(4, 0);
	input.col(1) << std::complex<double>(4, 0), std::complex<double>(3, 0),
					 std::complex<double>(2, 0), std::complex<double>(1, 0);

	auto spectrum = FrequenceHelper::Fft(input);
	auto recovered = FrequenceHelper::Ifft(spectrum);

	ASSERT_EQ(recovered.rows(), input.rows());
	ASSERT_EQ(recovered.cols(), input.cols());
	for (int j = 0; j < input.cols(); ++j) {
		for (int i = 0; i < input.rows(); ++i) {
			EXPECT_NEAR(recovered(i, j).real(), input(i, j).real(), 1e-10);
			EXPECT_NEAR(recovered(i, j).imag(), input(i, j).imag(), 1e-10);
		}
	}
}

// ============================================================================
// Eigen::MatrixXd FFT (实数矩阵 -> 复数频谱矩阵)
// ============================================================================

TEST(FrequenceHelperTest, Fft_MatrixXd) {
	Eigen::MatrixXd input(4, 2);
	input.col(0) << 1.0, 2.0, 3.0, 4.0;
	input.col(1) << 4.0, 3.0, 2.0, 1.0;

	auto spectrum = FrequenceHelper::Fft(input);
	ASSERT_EQ(spectrum.rows(), 4);
	ASSERT_EQ(spectrum.cols(), 2);

	// 两列直流分量均为 10
	EXPECT_NEAR(spectrum(0, 0).real(), 10.0, 1e-10);
	EXPECT_NEAR(spectrum(0, 1).real(), 10.0, 1e-10);
}

// ============================================================================
// Eigen::MatrixXcf FFT / IFFT（逐列变换）
// ============================================================================

TEST(FrequenceHelperTest, Fft_Ifft_Roundtrip_MatrixXcf) {
	Eigen::MatrixXcf input(4, 1);
	input.col(0) << std::complex<float>(1, 0), std::complex<float>(2, 0),
					 std::complex<float>(3, 0), std::complex<float>(4, 0);

	auto spectrum = FrequenceHelper::Fft(input);
	auto recovered = FrequenceHelper::Ifft(spectrum);

	ASSERT_EQ(recovered.rows(), input.rows());
	for (int i = 0; i < input.rows(); ++i) {
		EXPECT_NEAR(recovered(i, 0).real(), input(i, 0).real(), 1e-4f);
		EXPECT_NEAR(recovered(i, 0).imag(), input(i, 0).imag(), 1e-4f);
	}
}

// ============================================================================
// Eigen::MatrixXf FFT (实数矩阵 -> 复数频谱矩阵)
// ============================================================================

TEST(FrequenceHelperTest, Fft_MatrixXf) {
	Eigen::MatrixXf input(4, 1);
	input << 1.0f, 2.0f, 3.0f, 4.0f;

	auto spectrum = FrequenceHelper::Fft(input);
	ASSERT_EQ(spectrum.rows(), 4);
	ASSERT_EQ(spectrum.cols(), 1);

	EXPECT_NEAR(spectrum(0, 0).real(), 10.0f, 1e-4f);
}

// ============================================================================
// 已知正弦信号 FFT 验证
// ============================================================================

TEST(FrequenceHelperTest, Fft_SingleFrequency_Sine) {
	// 生成 N=8 的正弦信号, 频率 = 1 (即 bin 1)
	const int N = 8;
	std::vector<std::complex<double>> signal(N);
	for (int i = 0; i < N; ++i) {
		signal[i] = std::complex<double>(std::sin(2.0 * M_PI * i / N), 0.0);
	}

	auto spectrum = FrequenceHelper::Fft(signal);

	// 对于纯正弦，bin 0 (DC) 应为 0
	EXPECT_NEAR(std::abs(spectrum[0]), 0.0, 1e-10);
	// bin 1 应有显著能量 (幅值 = N/2 = 4)
	EXPECT_NEAR(std::abs(spectrum[1]), 4.0, 1e-10);
}
