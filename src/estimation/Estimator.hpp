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

#define ROLL_OFFSET_HORIZON 0.061236f
#define PITCH_OFFSET_HORIZON 0.032144f

using namespace Eigen;

class Estimator
{
public:
	void collect_sensor_data(void);

	float roll_from_quat(const Quaternionf& q);
	float pitch_from_quat(const Quaternionf& q);
	float yaw_from_quat(const Quaternionf& q);

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