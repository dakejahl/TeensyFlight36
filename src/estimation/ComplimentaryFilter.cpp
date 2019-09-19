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

#include <ComplimentaryFilter.hpp>

void ComplimentaryFilter::estimate_attitude(void)
{
	estimate_rpy_from_accel_and_gyro(_accel_xyz, _gyro_xyz);
}

void ComplimentaryFilter::estimate_rpy_from_accel_and_gyro(const Vector3f& accel, const Vector3f& gyro)
{
	// Calculate roll and pitch
	float roll = equations::roll_from_accel(accel.x(), accel.y(), accel.z());
	float pitch = equations::pitch_from_accel(accel.x(), accel.y(), accel.z());

	// Calculate roll and pitch rates
	float pitch_rate = equations::pitch_rate_from_gyro(_gyro_xyz.x(), _gyro_xyz.y(), _gyro_xyz.z(), _roll_est);

	float roll_rate = equations::roll_rate_from_gyro(_gyro_xyz.x(), _gyro_xyz.y(), _gyro_xyz.z(), _pitch_est, _roll_est);

	float yaw_rate = equations::yaw_rate_from_gyro(_gyro_xyz.x(), _gyro_xyz.y(), _gyro_xyz.z(), _pitch_est, _roll_est);

	// SYS_INFO("pitch_rate: %f", pitch_rate);
	// SYS_INFO("roll_rate: %f", roll_rate);
	// SYS_INFO("yaw_rate: %f", yaw_rate);

	auto now = time::HighPrecisionTimer::Instance()->get_absolute_time_us();
	float delta = now - _last_timestamp; // type is float to avoid truncation in the next line
	float dt = delta / MICROS_PER_SEC;
	_last_timestamp = now;

	// Run filter
	_roll_est = (1 - _alpha) * (_roll_est + roll_rate * dt) + _alpha * roll;
	_pitch_est = (1 - _alpha) * (_pitch_est + pitch_rate * dt) + _alpha * pitch;
	_yaw_est = _yaw_est + yaw_rate * dt; // just integrating the gyro

	// SYS_INFO("roll est: %f", _roll_est);
	// SYS_INFO("pitch est: %f", _pitch_est);
	// SYS_INFO("yaw est: %f", _yaw_est);
}