// test_physical_parameters.cpp - PhysicalParameters 类的单元测试
#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include "../../../src/IO/IO/PhysicalParameters.h"

using namespace Qahse::IO::IO;
using phy = PhysicalParameters;

// ============================================================================
// Constants
// ============================================================================

TEST(PhysicalParametersTest, PI) {
	EXPECT_NEAR(phy::PI, 3.141592653589793, 1e-15);
}

TEST(PhysicalParametersTest, R2D) {
	EXPECT_NEAR(phy::R2D, 180.0 / phy::PI, 1e-12);
}

TEST(PhysicalParametersTest, D2R) {
	EXPECT_NEAR(phy::D2R, phy::PI / 180.0, 1e-12);
}

TEST(PhysicalParametersTest, PiBy2) {
	EXPECT_NEAR(phy::PiBy2, phy::PI / 2.0, 1e-15);
}

TEST(PhysicalParametersTest, RoundTripRadianDegree) {
	double angle_rad = 1.5;
	double angle_deg = angle_rad * phy::R2D;
	double angle_back = angle_deg * phy::D2R;
	EXPECT_NEAR(angle_back, angle_rad, 1e-12);
}

TEST(PhysicalParametersTest, _2MutiPI) {
	EXPECT_NEAR(phy::_2MutiPI, 2.0 * phy::PI, 1e-15);
}

TEST(PhysicalParametersTest, Inv2Pi) {
	EXPECT_NEAR(phy::Inv2Pi, 1.0 / (2.0 * phy::PI), 1e-15);
}


// ============================================================================
// RPM/RPS conversions
// ============================================================================

TEST(PhysicalParametersTest, RPS2RPM) {
	// 1 rad/s = 60/(2*PI) rpm
	EXPECT_NEAR(phy::RPS2RPM, 60.0 / (2.0 * phy::PI), 1e-12);
}

TEST(PhysicalParametersTest, RPM2RPS) {
	// 1 rpm = 2*PI/60 rad/s
	EXPECT_NEAR(phy::RPM2RPS, (2.0 * phy::PI) / 60.0, 1e-12);
}

TEST(PhysicalParametersTest, RoundTripRPM_RPS) {
	double rpm = 100.0;
	double rps = rpm * phy::RPM2RPS;
	double rpm_back = rps * phy::RPS2RPM;
	EXPECT_NEAR(rpm_back, rpm, 1e-10);
}


// ============================================================================
// Float constants
// ============================================================================

TEST(PhysicalParametersTest, FloatConstants) {
	EXPECT_FLOAT_EQ(phy::R2Df, static_cast<float>(phy::R2D));
	EXPECT_FLOAT_EQ(phy::D2Rf, static_cast<float>(phy::D2R));
	EXPECT_FLOAT_EQ(phy::PiBy2f, static_cast<float>(phy::PiBy2));
}


// ============================================================================
// Numeric limits
// ============================================================================

TEST(PhysicalParametersTest, EpsilonD) {
	EXPECT_EQ(phy::EpsilonD, std::numeric_limits<double>::epsilon());
}

TEST(PhysicalParametersTest, EpsilonF) {
	EXPECT_EQ(phy::EpsilonF, std::numeric_limits<float>::epsilon());
}

TEST(PhysicalParametersTest, MaxDouble) {
	EXPECT_EQ(phy::MaxDouble, std::numeric_limits<double>::max());
}

TEST(PhysicalParametersTest, MinDouble) {
	EXPECT_EQ(phy::MinDouble, std::numeric_limits<double>::min());
}

TEST(PhysicalParametersTest, MaxFloat) {
	EXPECT_EQ(phy::MaxFloat, std::numeric_limits<float>::max());
}

TEST(PhysicalParametersTest, MinFloat) {
	EXPECT_EQ(phy::MinFloat, std::numeric_limits<float>::min());
}


// ============================================================================
// Unit conversions
// ============================================================================

TEST(PhysicalParametersTest, NtoKn) {
	EXPECT_DOUBLE_EQ(phy::NtoKn, 0.001);
	// 1000 N = 1 kN
	EXPECT_DOUBLE_EQ(1000.0 * phy::NtoKn, 1.0);
}

TEST(PhysicalParametersTest, NtoMn) {
	EXPECT_DOUBLE_EQ(phy::NtoMn, 0.000001);
	// 1000000 N = 1 MN
	EXPECT_DOUBLE_EQ(1000000.0 * phy::NtoMn, 1.0);
}


// ============================================================================
// Max / Min functions
// ============================================================================

TEST(PhysicalParametersTest, MaxDouble_Test) {
	EXPECT_DOUBLE_EQ(phy::Max(3.0, 5.0), 5.0);
	EXPECT_DOUBLE_EQ(phy::Max(-1.0, -3.0), -1.0);
	EXPECT_DOUBLE_EQ(phy::Max(2.0, 2.0), 2.0);
}

TEST(PhysicalParametersTest, MinDouble_Test) {
	EXPECT_DOUBLE_EQ(phy::Min(3.0, 5.0), 3.0);
	EXPECT_DOUBLE_EQ(phy::Min(-1.0, -3.0), -3.0);
	EXPECT_DOUBLE_EQ(phy::Min(2.0, 2.0), 2.0);
}

TEST(PhysicalParametersTest, MaxFloat_Test) {
	EXPECT_FLOAT_EQ(phy::Max(3.0f, 5.0f), 5.0f);
	EXPECT_FLOAT_EQ(phy::Max(-1.0f, -3.0f), -1.0f);
}

TEST(PhysicalParametersTest, MinFloat_Test) {
	EXPECT_FLOAT_EQ(phy::Min(3.0f, 5.0f), 3.0f);
	EXPECT_FLOAT_EQ(phy::Min(-1.0f, -3.0f), -3.0f);
}

TEST(PhysicalParametersTest, Onebythree) {
	EXPECT_NEAR(phy::Onebythree, 1.0 / 3.0, 1e-15);
}
