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

#include <Time.hpp>
#include <Messenger.hpp>

#include <Eigen/Dense>

// Gyro
static constexpr float GYRO_OFFSET_X =  -0.011789f;
static constexpr float GYRO_OFFSET_Y =  0.032313f;
static constexpr float GYRO_OFFSET_Z =  0.027732f;
// Accel
static constexpr float ACCEL_OFFSET_X = 0.317478f;
static constexpr float ACCEL_OFFSET_Y = 0.224600f;
static constexpr float ACCEL_OFFSET_Z = -0.334862f;
static constexpr float ACCEL_SCALE_X =	1.003191f;
static constexpr float ACCEL_SCALE_Y =	0.998537f;
static constexpr float ACCEL_SCALE_Z =	0.992191f;
// Mag -- TODO: investigate why a single outlier fucks up the ellipsoid fit algorithm
static constexpr float MAG_OFFSET_X =   10.0653f;
static constexpr float MAG_OFFSET_Y =   34.8082f;
static constexpr float MAG_OFFSET_Z =   -159.862;
static constexpr float MAG_SCALE_X =  	289.209f;
static constexpr float MAG_SCALE_Y =  	282.384f;
static constexpr float MAG_SCALE_Z =  	262.062f;

using namespace Eigen;

class Estimator
{
public:
	void collect_sensor_data(void);

	float roll_from_quat(const Quaternionf& q);
	float pitch_from_quat(const Quaternionf& q);
	float yaw_from_quat(const Quaternionf& q);

	void apply_gyro_calibration(float& x, float& y, float& z);
	void apply_accel_calibration(float& x, float& y, float& z);
	void apply_mag_calibration(float& x, float& y, float& z);

	float get_roll() { return _roll_est; };
	float get_pitch() { return _pitch_est; };
	float get_yaw() { return _yaw_est; };

protected:
	abs_time_t _last_timestamp = {};

	// Sensor data collected from last polling
	Vector3f _gyro_xyz = {};
	Vector3f _accel_xyz = {};
	Vector3f _mag_xyz = {};

	// Data subscribers
	messenger::Subscriber<gyro_raw_data_s> _gyro_sub;
	messenger::Subscriber<accel_raw_data_s> _accel_sub;
	messenger::Subscriber<mag_raw_data_s> _mag_sub;

	float _roll_est {};
	float _pitch_est {};
	float _yaw_est {};
};