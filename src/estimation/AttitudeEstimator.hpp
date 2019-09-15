// MIT License

// Copyright (c) 2019 Jacob Dahl

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <Eigen/Dense>
#include <cmath>

#include <Time.hpp>
#include <board_config.hpp>
#include <Messenger.hpp>
#include <LowPassFilter.hpp>

using namespace Eigen;

// Gyro
static constexpr float GYRO_OFFSET_X =  -0.005717;
static constexpr float GYRO_OFFSET_Y =  0.030644;
static constexpr float GYRO_OFFSET_Z =  0.017080;
// Accel
static constexpr float ACCEL_OFFSET_X = 0.326940f;
static constexpr float ACCEL_OFFSET_Y = 0.193270f;
static constexpr float ACCEL_OFFSET_Z = -0.411380f;
static constexpr float ACCEL_SCALE_X =	0.997458f;
static constexpr float ACCEL_SCALE_Y =	0.998817f;
static constexpr float ACCEL_SCALE_Z =	0.984198f;
// Mag -- TODO: investigate why a single outlier fucks up the ellipsoid fit algorithm
static constexpr float MAG_OFFSET_X =   10.0653f;
static constexpr float MAG_OFFSET_Y =   34.8082f;
static constexpr float MAG_OFFSET_Z =   -159.862;
static constexpr float MAG_SCALE_X =  	289.209f;
static constexpr float MAG_SCALE_Y =  	282.384f;
static constexpr float MAG_SCALE_Z =  	262.062f;

class AttitudeEstimator
{
public:

	Quaternionf estimate_quat_1st_step(void);
	Quaternionf estimate_quat_2nd_step(const Quaternionf& q);

	float roll_from_quat(const Quaternionf& q);
	float pitch_from_quat(const Quaternionf& q);
	float yaw_from_quat(const Quaternionf& q);


	//----- Eq (5) -----//
	// Frame is NUE
	Matrix3f direction_cosine_matrix(const Quaternionf& q)
	{
		Matrix3f C_bn;

		float q0 = q.w();
		float q1 = q.x();
		float q2 = q.y();
		float q3 = q.z();

		C_bn(0,0) = q0*q0 + q1*q1 - q2*q2 - q3*q3;
		C_bn(0,1) = 2*q0*q3 + 2*q1*q2;
		C_bn(0,2) = -2*q0*q2 + 2*q1*q3;

		C_bn(1,0) = -2*q0*q3 + 2*q1*q2;
		C_bn(1,1) = q0*q0 - q1*q1 + q2*q2 - q3*q3;
		C_bn(1,2) = 2*q0*q1 + 2*q2*q3;

		C_bn(2,0) = 2*q0*q2 + 2*q1*q3;
		C_bn(2,1) = -2*q0*q1 + 2*q2*q3;
		C_bn(2,2) = q0*q0 - q1*q1 - q2*q2 + q3*q3;

		return C_bn;
	}

private:
	// Functions
	Quaternionf quat_error(Quaternionf q1, Quaternionf q2);

	Quaternionf vect2q(Vector4f v);

	Vector4f q2vect(Quaternionf q);

	Quaternionf qexp(Quaternionf q);

	Matrix3f crossp_mat(Vector3f v);

	template <typename T> int sgn(T val)
	{
    	return (T(0) < val) - (val < T(0));
	}

	// Variables
	abs_time_t _last_time = 0;

	Quaternionf _q_obs = {};
	Quaternionf _q_err = {};

	Vector3f _bias_obs = {};

	Vector3f _epsilon_err = {};
	float _eta_err = {};

	Quaternionf _unknown_variable = {};

	// Data for my actual impl
	Quaternionf _q_estimated = {1,0,0,0}; // level surface

	// Data subscribers
	messenger::Subscriber<gyro_raw_data_s> _gyro_sub;
	messenger::Subscriber<accel_raw_data_s> _accel_sub;
	messenger::Subscriber<mag_raw_data_s> _mag_sub;

};